# Demo profile portraits

64 generated fictional profile portraits, spanning approximately ages 14–60,
including teenagers and young adults. These are synthetic demo identities.
The exact apparent age of a generated face is approximate.

`contact-sheet.png` is the original 8×8 sheet. `profile-01.jpg` through
`profile-64.jpg` are individual 256×256 RGB JPEGs in row-major order.
`manifest.json` records their stable Appwrite file IDs and grid positions.

Recreate the JPGs with Python and Pillow:

```sh
python3 tools/split-profile-grid.py demo-assets/profiles/contact-sheet.png demo-assets/profiles
```

The JPGs are uploaded to the Octopus project `69f1c06800389dc6a1a0`, in
`post-images`, as `demo-profile-01` through `demo-profile-64`. The bucket is
publicly readable. Both Chat and Octopus already resolve public profile
photos from `octopus-db/profiles.avatarId` to files in this bucket.
That optional string attribute must exist with size 36; the live project
was missing it and it was created during this repair.

Only seed demo identities receive generated photos. Existing personal
photos are preserved. `assignments.json` records the live seed associations.
The offline apps bundle a small selection under their own `assets/avatars`
directories, using the names expected by their existing fixture loaders.

Public file URL:

```text
https://fra.cloud.appwrite.io/v1/storage/buckets/post-images/files/demo-profile-01/view?project=69f1c06800389dc6a1a0
```
