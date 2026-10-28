#!/usr/bin/env python3
"""Sign an iPhone/iPad app using a matching installed development identity/profile."""
import argparse
from datetime import datetime, timezone
import hashlib
from pathlib import Path
import plistlib
import re
import shutil
import subprocess
import tempfile


def matches(pattern, identifier):
    return pattern == identifier or (pattern.endswith('.*') and identifier.startswith(pattern[:-1]))


def signing_entitlements(identifier, team, allowed_groups, push_environment=None):
    if not any(matches(group, identifier) for group in allowed_groups):
        raise ValueError('Provisioning profile does not allow the app Keychain group ' + identifier)
    entitlements = {
        'application-identifier': identifier,
        'com.apple.developer.team-identifier': team,
        'keychain-access-groups': [identifier],
        'get-task-allow': True,
    }
    if push_environment is not None:
        if push_environment != 'development':
            raise ValueError('Development signing requires an APNs development provisioning profile')
        entitlements['aps-environment'] = push_environment
    return entitlements


def simulated_entitlements(bundle_id):
    if not isinstance(bundle_id, str) or not bundle_id:
        raise ValueError('Simulator bundle has no application identifier')
    # The simulator Keychain uses the application identifier as each app's
    # private default group. This local prefix needs no team or profile.
    return {'application-identifier': 'ORCASIM000.' + bundle_id}


def simulator_entitlements(bundle):
    if bundle.get('CFBundleSupportedPlatforms') != ['iPhoneSimulator']:
        raise ValueError('Ad hoc simulator signing requires an iPhoneSimulator bundle')
    return simulated_entitlements(bundle.get('CFBundleIdentifier'))


def write_simulated_entitlements(path, bundle_id):
    """Write the linker section inputs, touching them only when they change."""
    data = plistlib.dumps(simulated_entitlements(bundle_id))
    if path.is_file() and path.read_bytes() == data and path.with_suffix(path.suffix + '.der').is_file():
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.tmp')
    temporary.write_bytes(data)
    subprocess.run(['derq', 'query', '-f', 'xml', '-i', str(temporary), '-o',
                    str(path.with_suffix(path.suffix + '.der')), '--raw'], check=True)
    temporary.replace(path)


def linked_entitlements(binary):
    with tempfile.TemporaryDirectory() as folder:
        section = Path(folder) / 'entitlements.plist'
        result = subprocess.run(['segedit', str(binary), '-extract', '__TEXT', '__entitlements', str(section)],
                                capture_output=True)
        return plistlib.loads(section.read_bytes()) if result.returncode == 0 and section.is_file() else None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', type=Path, nargs='?')
    parser.add_argument('--team')
    parser.add_argument('--device')
    parser.add_argument('--profile', type=Path)
    parser.add_argument('--simulator', action='store_true')
    parser.add_argument('--simulated-entitlements', type=Path, metavar='PATH')
    parser.add_argument('--identifier')
    args = parser.parse_args()
    if args.simulated_entitlements:
        try:
            write_simulated_entitlements(args.simulated_entitlements, args.identifier)
        except ValueError as error:
            raise SystemExit(str(error))
        return
    if not args.app:
        parser.error('the app bundle is required')
    app = args.app.resolve()
    bundle = plistlib.loads((app / 'Info.plist').read_bytes())
    bundle_id = bundle['CFBundleIdentifier']
    push_required = bool(bundle.get('ORCARemoteNotifications'))
    entitlement_path = app.parent / (app.stem + '.entitlements.plist')
    if args.simulator:
        try:
            entitlements = simulator_entitlements(bundle)
        except ValueError as error:
            raise SystemExit(str(error))
        # Simulators refuse to launch ad hoc signatures that carry entitlements.
        # As with Xcode, the identity is linked into the binary instead.
        if linked_entitlements(app / bundle['CFBundleExecutable']) != entitlements:
            raise SystemExit('Relink ' + bundle_id + ' with its simulated entitlements (make -f tools/ios/Makefile)')
        subprocess.run(['codesign', '--force', '--sign', '-', str(app)], check=True)
        subprocess.run(['codesign', '--verify', '--strict', str(app)], check=True)
        print('Signed simulator ' + bundle_id + ' with its private Keychain group')
        return
    identities = subprocess.check_output(['security', 'find-identity', '-v', '-p', 'codesigning'], text=True)
    valid = set()
    for line in identities.splitlines():
        match = re.search(r'\b([A-F0-9]{40})\s+"', line)
        if match and 'CSSMERR_' not in line:
            valid.add(match[1])
    if args.profile:
        paths = [args.profile]
    else:
        paths = []
        for folder in ('Library/Developer/Xcode/UserData/Provisioning Profiles',
                       'Library/MobileDevice/Provisioning Profiles'):
            paths.extend((Path.home() / folder).glob('*.mobileprovision'))
    candidates = []
    for path in paths:
        decoded = subprocess.run(['security', 'cms', '-D', '-i', str(path)], capture_output=True)
        if decoded.returncode:
            continue
        profile = plistlib.loads(decoded.stdout)
        entitlements = profile.get('Entitlements', {})
        team = entitlements.get('com.apple.developer.team-identifier')
        prefix = profile.get('ApplicationIdentifierPrefix', [''])[0]
        identifier = prefix + '.' + bundle_id
        if (args.team and args.team != team) or not team:
            continue
        if args.device and args.device not in profile.get('ProvisionedDevices', []):
            continue
        if 'iOS' not in profile.get('Platform', []):
            continue
        if profile['ExpirationDate'] <= datetime.now(timezone.utc).replace(tzinfo=None) or not entitlements.get('get-task-allow'):
            continue
        if not matches(entitlements.get('application-identifier', ''), identifier):
            continue
        push_environment = entitlements.get('aps-environment') if push_required else None
        if push_required and push_environment != 'development':
            continue
        groups = entitlements.get('keychain-access-groups', [])
        if not any(matches(group, identifier) for group in groups):
            continue
        certificates = {hashlib.sha1(cert).hexdigest().upper() for cert in profile.get('DeveloperCertificates', [])}
        available = sorted(valid & certificates)
        if available:
            candidates.append((profile['ExpirationDate'], str(path), available[0], identifier, team, groups,
                               push_environment))
    if not candidates:
        raise SystemExit('No matching development profile' + (' with APNs development enabled' if push_required else '') +
                         ' and signing identity for ' + bundle_id +
                         '. Install a valid profile/certificate or pass PROFILE=/path/to/profile.mobileprovision'
                         ' (and TEAM=... if needed). Simulator builds need neither.')
    _, profile, identity, identifier, team, groups, push_environment = max(candidates)
    shutil.copy2(profile, app / 'embedded.mobileprovision')
    entitlement_path.write_bytes(plistlib.dumps(signing_entitlements(identifier, team, groups, push_environment)))
    subprocess.run(['codesign', '--force', '--sign', identity, '--entitlements', str(entitlement_path),
                    '--generate-entitlement-der', str(app)], check=True)
    subprocess.run(['codesign', '--verify', '--strict', str(app)], check=True)
    print('Signed ' + bundle_id + ' with team ' + team)


if __name__ == '__main__':
    main()
