#!/usr/bin/env python3
"""Find a physical iPhone or iPad, then sign/install/launch a bundled ORCA app."""
import argparse
import json
import os
from pathlib import Path
import plistlib
import subprocess
import sys
import tempfile


def device_list_command(requested, output):
    command = ['xcrun', 'devicectl', 'list', 'devices']
    if not requested:
        # Match devicectl's displayed State; tunnel and transport can remain
        # connected for paired devices that are unavailable for deployment.
        command += ['--filter', "State BEGINSWITH 'available' OR State = 'connected'"]
    return command + ['--json-output', output]


def select_device(devices, device_type, requested=None):
    candidates = []
    for device in devices:
        hardware = device.get('hardwareProperties', {})
        properties = device.get('deviceProperties', {})
        identity = device.get('identifier', '')
        name = properties.get('name', '')
        is_target_type = (hardware.get('deviceType') == device_type
                          or device_type.lower() in hardware.get('marketingName', '').lower())
        if not is_target_type or hardware.get('reality') != 'physical':
            continue
        if requested:
            if requested in (identity, name, hardware.get('udid')):
                candidates.append(identity)
        else:
            candidates.append(identity)
    if len(candidates) != 1:
        if device_type == 'iPhone':
            guidance = 'Connect exactly one available physical iPhone; run make ios-devices to inspect devices.'
        else:
            guidance = 'Run make ios-devices and set DEVICE=<identifier>.'
        raise ValueError(f'Expected exactly one available physical {device_type}; found {len(candidates)}. {guidance}')
    return candidates[0]


def run(*args):
    subprocess.run([str(arg) for arg in args], check=True)


def select_simulator(payload, device_type, requested=None):
    devices = [device for group in payload['devices'].values() for device in group
               if device.get('isAvailable') and
               (device_type in device.get('deviceTypeIdentifier', '') or device_type in device['name'])]
    if requested:
        devices = [device for device in devices if requested in (device['udid'], device['name'])]
        if len(devices) > 1:
            raise ValueError('Simulator name is ambiguous; set DEVICE to its UDID.')
    devices.sort(key=lambda device: (device['state'] != 'Booted', device['name'], device['udid']))
    if not devices:
        raise ValueError(f'No available {device_type} simulator matches. Run make ios-devices to inspect devices.')
    return devices[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--bundle', type=Path, required=True)
    parser.add_argument('--device')
    parser.add_argument('--device-type', choices=('iPhone', 'iPad'), default='iPhone')
    parser.add_argument('--team')
    parser.add_argument('--profile')
    parser.add_argument('--simulator', action='store_true')
    args = parser.parse_args()
    bundle = args.bundle.resolve()
    identifier = plistlib.loads((bundle / 'Info.plist').read_bytes())['CFBundleIdentifier']
    if args.simulator:
        payload = json.loads(subprocess.check_output(['xcrun', 'simctl', 'list', 'devices', 'available', '-j']))
        device = select_simulator(payload, args.device_type, args.device)
        if device['state'] != 'Booted':
            run('xcrun', 'simctl', 'boot', device['udid'])
        run('xcrun', 'simctl', 'bootstatus', device['udid'], '-b')
        developer = os.environ.get('DEVELOPER_DIR') or subprocess.check_output(['xcode-select', '-p'], text=True).strip()
        simulator = Path(developer) / 'Applications/Simulator.app'
        if simulator.is_dir():
            subprocess.run(['open', '-a', str(simulator)], check=False)
        else:
            print('Simulator GUI is unavailable; continuing with the simulator service.', flush=True)
        run('xcrun', 'simctl', 'install', device['udid'], bundle)
        run('xcrun', 'simctl', 'launch', '--terminate-running-process', device['udid'], identifier)
        print('Launched on ' + device['name'] + ' (' + device['udid'] + ')')
        return
    with tempfile.TemporaryDirectory() as temporary:
        output = Path(temporary) / 'devices.json'
        run(*device_list_command(args.device, output))
        payload = json.loads(output.read_text())
    device = select_device(payload['result']['devices'], args.device_type, args.device)
    print('Selected ' + args.device_type + ' ' + device, flush=True)
    sign = [sys.executable, str(Path(__file__).with_name('sign.py')), str(bundle), '--device', device]
    # The signer checks provisioning device UDIDs after resolving CoreDevice's identifier below.
    matching = next(d for d in payload['result']['devices'] if d['identifier'] == device)
    sign[-1] = matching.get('hardwareProperties', {}).get('udid', device)
    if args.team:
        sign += ['--team', args.team]
    if args.profile:
        sign += ['--profile', args.profile]
    run(*sign)
    run('xcrun', 'devicectl', 'device', 'install', 'app', '--device', device, bundle)
    run('xcrun', 'devicectl', 'device', 'process', 'launch', '--device', device, '--terminate-existing', identifier)


if __name__ == '__main__':
    try:
        main()
    except ValueError as error:
        raise SystemExit(str(error))
    except subprocess.CalledProcessError as error:
        raise SystemExit(f'Deployment step failed (exit {error.returncode}). See the tool error above; unlock the device if launch was denied.')
