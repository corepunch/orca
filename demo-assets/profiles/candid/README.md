# Candid fictional contacts

64 new synthetic adult contact photos generated with the built-in image generation
tool on 2026-10-28. The user requested unrelated personal photos rather than a
uniform studio session: different locations, angles, expressions, lighting and
camera quality. No real accounts or people's photographs are used.

`contact-sheet.png` is the original 8 × 8 generated image. `profile-01.jpg` through
`profile-64.jpg` are square 256-pixel RGB JPEGs, in row-major order. Their crops
are deterministic outputs of `tools/split-profile-grid.py`. The exact generation
prompt is saved in `prompt.txt`.

These are local promotional fixtures. They have not been uploaded to Appwrite
or assigned to live users. The splitter's manifest `fileId` values are inherited
from the existing tool and do not describe a deployment.

Chat's screenshot controller assigns stable fictional names to these images;
repeated people keep the same image across Contacts, inboxes and conversations.
Capture packaging copies them into an isolated simulator bundle. The existing
studio portrait library and production profile data remain available.
