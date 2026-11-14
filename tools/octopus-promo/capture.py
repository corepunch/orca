#!/usr/bin/env python3
"""Capture staged, offline Octopus screens in an isolated iPhone simulator app."""
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
OUTPUT = ROOT / 'marketing/octopus/app-store'
IDENTIFIER = 'org.orca.octopus.promoshots'
# name, stage, appearance. Captions live in render.py.
SHOTS = [
    ('01-home', 'home', 'dark'),
    ('02-connect', 'connect', 'dark'),
    ('03-discover', 'discover', 'dark'),
    ('04-post', 'post', 'dark'),
    ('05-profile', 'profile', 'dark'),
    ('06-write', 'write', 'dark'),
    ('07-album', 'album', 'dark'),
    ('08-light', 'light', 'light'),
]


def run(*args, **kwargs):
    return subprocess.run(args, check=True, **kwargs)


def prepare(bundle, binary, family='iphone'):
    # Package the current Lua/resources around an already-built simulator
    # executable, under a separate identifier so the real app stays untouched.
    if bundle.exists():
        raise SystemExit(f'Refusing to replace existing bundle: {bundle}')
    run('python3', str(ROOT / 'tools/ios/bundle.py'), '--binary', str(binary),
        '--bundle', str(bundle), '--project', str(ROOT / 'apps/octopus'),
        '--sdk', 'iphonesimulator', '--identifier', IDENTIFIER,
        '--minimum', '16.0', '--display-name', 'Octopus Promo', cwd=ROOT)
    project = bundle / 'project'
    promo = project / 'promo'
    (promo / 'assets').mkdir(parents=True)
    for name in ('Capture.lua', 'fixtures.lua'):
        shutil.copy2(HERE / name, promo / name)
    shutil.copy2(ROOT / 'tools/chat-promo/assets/lake.jpg', promo / 'assets/lake.jpg')
    portraits = sorted((ROOT / 'demo-assets/profiles/candid').glob('profile-*.jpg'))
    if len(portraits) != 64:
        raise SystemExit('Expected all 64 candid demo portraits; see demo-assets/profiles/candid/README.md')
    for portrait in portraits:
        shutil.copy2(portrait, project / 'assets/avatars' / portrait.name)
    package = project / 'package.lua'
    package.write_text(package.read_text().replace('"Octopus/App"', '"Octopus/promo/Capture"'))
    info_path = bundle / 'Info.plist'
    info = plistlib.loads(info_path.read_bytes())
    info.pop('ORCARemoteNotifications', None)
    info['UIDeviceFamily'] = [2] if family == 'ipad' else [1]
    info_path.write_bytes(plistlib.dumps(info))
    run('codesign', '--force', '--deep', '--sign', '-', str(bundle))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--device', required=True, help='Explicit booted simulator UDID')
    parser.add_argument('--family', choices=('iphone', 'ipad'), default='iphone')
    parser.add_argument('--output-dir', type=Path, default=OUTPUT)
    parser.add_argument('--bundle', type=Path, required=True, help='New temporary .app directory')
    parser.add_argument('--binary', type=Path, default=ROOT / 'build/ios/iphonesimulator-arm64/Orca.app/Orca')
    parser.add_argument('--delay', type=float, default=5)
    parser.add_argument('--only', nargs='+', help='Capture selected stage names for iteration')
    args = parser.parse_args()
    output = args.output_dir.resolve()
    records = []
    manifest = output / 'manifest.json'
    if args.only and manifest.exists():
        records = [s for s in json.loads(manifest.read_text())['shots'] if s['stage'] not in args.only]
    prepare(args.bundle, args.binary, args.family)
    raw = output / 'captures'
    raw.mkdir(parents=True, exist_ok=True)
    run('xcrun', 'simctl', 'install', args.device, str(args.bundle))
    run('xcrun', 'simctl', 'status_bar', args.device, 'override', '--time', '9:41',
        '--dataNetwork', 'wifi', '--wifiMode', 'active', '--wifiBars', '3',
        '--batteryState', 'discharging', '--batteryLevel', '100')
    for name, stage, appearance in SHOTS:
        if args.only and stage not in args.only:
            continue
        subprocess.run(['xcrun', 'simctl', 'terminate', args.device, IDENTIFIER], capture_output=True)
        run('xcrun', 'simctl', 'ui', args.device, 'appearance', appearance)
        env = dict(os.environ, SIMCTL_CHILD_OCTOPUS_PROMO_STAGE=stage,
                   SIMCTL_CHILD_OCTOPUS_APPEARANCE=appearance)
        log = raw / f'{name}.log'
        log.unlink(missing_ok=True)
        run('xcrun', 'simctl', 'launch', '--terminate-running-process',
            '--stdout=' + str(log), '--stderr=' + str(log), args.device, IDENTIFIER, env=env)
        deadline = time.monotonic() + 45
        while 'Application module initialized' not in log.read_text():
            if 'ORCA:' in log.read_text() or time.monotonic() > deadline:
                raise SystemExit(f'Application did not initialize; inspect {log}')
            time.sleep(0.3)
        time.sleep(args.delay)
        target = raw / f'{name}.png'
        run('xcrun', 'simctl', 'io', args.device, 'screenshot', str(target))
        text = log.read_text()
        if 'ORCA:' in text or 'octopus: ' in text:
            raise SystemExit(f'Capture failed; inspect {log}')
        records.append(dict(name=name, stage=stage, appearance=appearance,
                            capture=str(target.relative_to(output)),
                            sha256=hashlib.sha256(target.read_bytes()).hexdigest()))
    records.sort(key=lambda shot: shot['name'])
    metadata = dict(device=args.device, family=args.family, bundle_id=IDENTIFIER, offline=True,
                    binary_sha256=hashlib.sha256(args.binary.read_bytes()).hexdigest(), shots=records)
    manifest.write_text(json.dumps(metadata, indent=2) + '\n')
    print(f'Manifest contains {len(records)} views in {raw}')


if __name__ == '__main__':
    main()
