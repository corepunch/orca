#!/bin/sh
# Writes a 768x512 library thumbnail for every JPEG in Images/ into
# Images/Thumbs/. Run it after adding or replacing artwork (macOS: uses sips).
set -e
cd "$(dirname "$0")/../Images"
mkdir -p Thumbs
for image in *.jpg; do
    sips -Z 768 -s formatOptions 80 "$image" --out "Thumbs/$image" >/dev/null
done
