#!/usr/bin/env python3
"""Stage an ORCA project's assets and AndroidManifest.xml for the Gradle build."""
import argparse
import hashlib
import importlib.util
from pathlib import Path
import shutil
from xml.sax.saxutils import quoteattr

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('ios_bundle', ROOT / 'tools/ios/bundle.py')
ios_bundle = importlib.util.module_from_spec(spec)
spec.loader.exec_module(ios_bundle)

MANIFEST = '''<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android">
  <uses-feature android:glEsVersion="0x00030000" android:required="true" />
  <uses-permission android:name="android.permission.INTERNET" />
  <queries>
    <intent><action android:name="android.support.customtabs.action.CustomTabsService" /></intent>
    <intent>
      <action android:name="android.intent.action.VIEW" />
      <category android:name="android.intent.category.BROWSABLE" />
      <data android:scheme="https" />
    </intent>
  </queries>
  <application android:label={label} android:hasCode="true" android:allowBackup="false"
      android:theme="@android:style/Theme.Material.NoActionBar">
    <activity android:name="org.orca.OrcaActivity" android:exported="true" android:launchMode="singleTask"
        android:configChanges="orientation|screenSize|screenLayout|smallestScreenSize|keyboard|keyboardHidden|uiMode|density|navigation"
        android:windowSoftInputMode="adjustNothing|stateHidden">
      <meta-data android:name="android.app.lib_name" android:value="main" />
      <intent-filter>
        <action android:name="android.intent.action.MAIN" />
        <category android:name="android.intent.category.LAUNCHER" />
      </intent-filter>{links}
    </activity>
  </application>
</manifest>
'''
LINK = '''
      <intent-filter>
        <action android:name="android.intent.action.VIEW" />
        <category android:name="android.intent.category.DEFAULT" />
        <category android:name="android.intent.category.BROWSABLE" />
        <data android:scheme={scheme} />
      </intent-filter>'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--project', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--application-id', required=True)
    parser.add_argument('--display-name', required=True)
    args = parser.parse_args()
    project = args.project.resolve()
    if not (project / 'package.lua').is_file():
        parser.error('Expected an ORCA project with package.lua')
    assets = args.output.resolve() / 'assets'
    shutil.rmtree(assets, ignore_errors=True)
    assets.mkdir(parents=True)
    # Same runtime layout and credential filter as the iOS bundle.
    for name, source in (('share', ROOT / 'share'), ('project', project)):
        shutil.copytree(source, assets / name, ignore=ios_bundle.resource_filter)
    files = sorted(path.relative_to(assets).as_posix() for path in assets.rglob('*') if path.is_file())
    digest = hashlib.sha256()
    for name in files:
        digest.update(name.encode() + b'\0' + (assets / name).read_bytes())
    (assets / 'orca-files.txt').write_text('\n'.join(files) + '\n')
    (assets / 'orca-version.txt').write_text(digest.hexdigest())
    links = ''.join(LINK.format(scheme=quoteattr(scheme)) for scheme in ios_bundle.url_schemes(project))
    (args.output.resolve() / 'AndroidManifest.xml').write_text(
        MANIFEST.format(label=quoteattr(args.display_name), links=links))
    print('Staged ' + str(len(files)) + ' files for ' + args.application_id)


if __name__ == '__main__':
    main()
