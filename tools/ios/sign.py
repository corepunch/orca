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


def matches(pattern, identifier):
    return pattern == identifier or (pattern.endswith('.*') and identifier.startswith(pattern[:-1]))


def signing_entitlements(identifier, team, allowed_groups):
    if not any(matches(group, identifier) for group in allowed_groups):
        raise ValueError('Provisioning profile does not allow the app Keychain group ' + identifier)
    return {
        'application-identifier': identifier,
        'com.apple.developer.team-identifier': team,
        'keychain-access-groups': [identifier],
        'get-task-allow': True,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('app', type=Path)
    parser.add_argument('--team')
    parser.add_argument('--device')
    parser.add_argument('--profile', type=Path)
    args = parser.parse_args()
    app = args.app.resolve()
    bundle_id = plistlib.loads((app / 'Info.plist').read_bytes())['CFBundleIdentifier']
    entitlement_path = app.parent / (app.stem + '.entitlements.plist')
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
        groups = entitlements.get('keychain-access-groups', [])
        if not any(matches(group, identifier) for group in groups):
            continue
        certificates = {hashlib.sha1(cert).hexdigest().upper() for cert in profile.get('DeveloperCertificates', [])}
        available = sorted(valid & certificates)
        if available:
            candidates.append((profile['ExpirationDate'], str(path), available[0], identifier, team, groups))
    if not candidates:
        raise SystemExit('No matching development profile and signing identity for ' + bundle_id +
                         '. Install a valid profile/certificate or pass PROFILE=/path/to/profile.mobileprovision'
                         ' (and TEAM=... if needed). Simulator builds need neither.')
    _, profile, identity, identifier, team, groups = max(candidates)
    shutil.copy2(profile, app / 'embedded.mobileprovision')
    entitlement_path.write_bytes(plistlib.dumps(signing_entitlements(identifier, team, groups)))
    subprocess.run(['codesign', '--force', '--sign', identity, '--entitlements', str(entitlement_path),
                    '--generate-entitlement-der', str(app)], check=True)
    subprocess.run(['codesign', '--verify', '--strict', str(app)], check=True)
    print('Signed ' + bundle_id + ' with team ' + team)


if __name__ == '__main__':
    main()
