#!/usr/bin/env python3
"""Capture staged, offline Chat screens in an isolated iPhone simulator app."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import plistlib
import shutil
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
HERE = Path(__file__).resolve().parent
OUTPUT = ROOT / 'marketing/chat/app-store'
IDENTIFIER = 'org.orca.chat.promoshots'
SHOTS = [
    ('01-contacts', 'contacts', 'dark', 'your people.\na little closer.', 'A familiar face. A conversation away.'),
    ('02-contacts-wide', 'contacts-more', 'dark', 'more familiar\nfaces.', 'A wide view of the people in your world.'),
    ('03-inbox', 'inbox', 'dark', 'every chat.\none place.', 'Friends, plans, and the everyday in between.'),
    ('04-messages', 'direct', 'dark', 'small talk.\nbig connection.', 'Make time for the people who get you.'),
    ('05-photos', 'photos', 'dark', 'some moments\nsay everything.', 'Share the view. Keep the memory.'),
    ('06-groups', 'group', 'dark', 'good plans start\nin the group chat.', 'Keep the whole crew in the conversation.'),
    ('07-channels', 'channel', 'dark', 'a little more\nin the loop.', 'Follow the channels that bring you together.'),
    ('08-light', 'inbox', 'light', 'a brighter\nkind of day.', 'A familiar place, in light or dark.'),
    ('09-contacts-all', 'contacts-all', 'dark', '', ''),
]


def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


def prepare(bundle, binary):
    # Start from the already-built simulator executable; don't rebuild an engine
    # which may have unrelated work in progress. Package current Lua/resources.
    if bundle.exists():
        raise SystemExit(f'Refusing to replace existing bundle: {bundle}')
    run('python3', str(ROOT / 'tools/ios/bundle.py'), '--binary', str(binary),
        '--bundle', str(bundle), '--project', str(ROOT / 'apps/chat'),
        '--sdk', 'iphonesimulator', '--identifier', IDENTIFIER,
        '--minimum', '16.0', '--display-name', 'Chat Promo', cwd=ROOT)
    project = bundle / 'project'
    promo = project / 'promo'
    promo.mkdir()
    for name in ('Capture.lua', 'fixtures.lua'):
        shutil.copy2(HERE / name, promo / name)
    shutil.copytree(HERE / 'assets', promo / 'assets')
    portraits = sorted((ROOT / 'demo-assets/profiles/candid').glob('profile-*.jpg'))
    if len(portraits) != 64:
        raise SystemExit('Expected all 64 candid demo portraits; see demo-assets/profiles/candid/README.md')
    for portrait in portraits:
        shutil.copy2(portrait, promo / 'assets' / portrait.name)
    package = project / 'package.lua'
    package.write_text(package.read_text().replace('"Chat/App"', '"Chat/promo/Capture"'))
    # A separate identifier keeps the normal Chat application and Keychain alone.
    info_path = bundle / 'Info.plist'
    info = plistlib.loads(info_path.read_bytes())
    info.pop('ORCARemoteNotifications', None)
    info['UIDeviceFamily'] = [1]
    info_path.write_bytes(plistlib.dumps(info))
    run('codesign', '--force', '--deep', '--sign', '-', str(bundle))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--device', required=True, help='Explicit iPhone simulator UDID')
    parser.add_argument('--bundle', type=Path, required=True, help='New temporary .app directory')
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/ios/iphonesimulator-arm64/Orca.app/Orca')
    parser.add_argument('--delay', type=float, default=5)
    parser.add_argument('--only', nargs='+', help='Capture selected stage names for iteration')
    args = parser.parse_args()
    records = []
    if args.only:
        previous = json.loads((OUTPUT / 'manifest.json').read_text())
        if previous['binary_sha256'] != hashlib.sha256(args.binary.read_bytes()).hexdigest():
            raise SystemExit('Binary changed; capture the complete set first')
        unknown = set(args.only) - {shot[1] for shot in SHOTS}
        if unknown:
            raise SystemExit('Unknown stage: ' + ', '.join(sorted(unknown)))
        records = [shot for shot in previous['shots'] if shot['stage'] not in args.only]
    prepare(args.bundle, args.binary)
    raw = OUTPUT / 'captures'
    raw.mkdir(parents=True, exist_ok=True)
    run('xcrun', 'simctl', 'install', args.device, str(args.bundle))
    run('xcrun', 'simctl', 'status_bar', args.device, 'override', '--time', '9:41',
        '--dataNetwork', 'wifi', '--wifiMode', 'active', '--wifiBars', '3',
        '--batteryState', 'discharging', '--batteryLevel', '100')
    captured = 0
    for name, stage, appearance, title, subtitle in SHOTS:
        if args.only and stage not in args.only:
            continue
        subprocess.run(['xcrun', 'simctl', 'terminate', args.device, IDENTIFIER], capture_output=True)
        run('xcrun', 'simctl', 'ui', args.device, 'appearance', appearance)
        env = dict(os.environ, SIMCTL_CHILD_CHAT_PROMO_STAGE=stage,
                   SIMCTL_CHILD_CHAT_APPEARANCE=appearance)
        log = raw / f'{name}.log'
        log.unlink(missing_ok=True)
        run('xcrun', 'simctl', 'launch', '--terminate-running-process',
            '--stdout=' + str(log), '--stderr=' + str(log), args.device, IDENTIFIER, env=env)
        deadline = time.monotonic() + 45
        while 'Application module initialized' not in log.read_text():
            if 'ORCA:' in log.read_text() or time.monotonic() > deadline:
                raise SystemExit(f'Application did not initialize; inspect {log}')
            time.sleep(0.3)
        route = '/contacts' if stage.startswith('contacts') else '/' if stage == 'inbox' else '/thread'
        if 'Navigate: ' + route + '\n' not in log.read_text():
            raise SystemExit(f'Wrong initial route for {stage}; inspect {log}')
        time.sleep(args.delay)
        target = raw / f'{name}.png'
        run('xcrun', 'simctl', 'io', args.device, 'screenshot', str(target))
        if 'ORCA:' in log.read_text() or 'Chat error' in log.read_text():
            raise SystemExit(f'Capture failed; inspect {log}')
        records.append(dict(name=name, stage=stage, appearance=appearance, title=title,
                            subtitle=subtitle, capture=str(target.relative_to(OUTPUT)),
                            auxiliary=stage == 'contacts-all',
                            sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
        captured += 1
    records.sort(key=lambda shot: shot['name'])
    metadata = dict(device=args.device, bundle_id=IDENTIFIER, offline=True,
                    binary_sha256=hashlib.sha256(args.binary.read_bytes()).hexdigest(), shots=records)
    (OUTPUT / 'manifest.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print(f'Captured {captured} iPhone screens; manifest contains {len(records)} views in {raw}')


if __name__ == '__main__':
    main()
