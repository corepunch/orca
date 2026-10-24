// Inspect the uploaded structure and dimensions, independently of client metadata.
// Pixel entropy is decoded by the platform image loader, not by this inspector.
function locationMetadata(bytes, start, end) {
  const exif = [69, 120, 105, 102, 0, 0].every((value, i) => bytes[start + i] === value);
  if (!exif) return /gpslatitude|gpslongitude/.test(new TextDecoder().decode(bytes.subarray(start, end)).toLowerCase());
  const base = start + 6, size = end - base;
  if (size < 8) return true;
  const little = bytes[base] === 73 && bytes[base + 1] === 73;
  if (!little && !(bytes[base] === 77 && bytes[base + 1] === 77)) return true;
  const view = new DataView(bytes.buffer, bytes.byteOffset + base, size);
  const number = (offset, length) => offset < 0 || offset + length > size ? null
    : length === 2 ? view.getUint16(offset, little) : view.getUint32(offset, little);
  if (number(2, 2) !== 42) return true;
  const offset = number(4, 4), count = offset === null ? null : number(offset, 2);
  if (count === null || offset + 2 + count * 12 + 4 > size) return true;
  for (let i = 0; i < count; i++) if (number(offset + 2 + i * 12, 2) === 0x8825) return true;
  return false;
}

export function inspectJpeg(bytes, maxEdge) {
  const invalid = () => { throw new Error("Invalid JPEG photo"); };
  if (!(bytes instanceof Uint8Array) || bytes.length < 32 || bytes.length > 1000000
      || bytes[0] !== 0xff || bytes[1] !== 0xd8) invalid();
  let pos = 2, frame = null, scans = 0;
  while (pos < bytes.length) {
    if (bytes[pos++] !== 0xff) invalid();
    while (bytes[pos] === 0xff) pos++;
    const marker = bytes[pos++];
    if (marker === 0xd9) {
      if (pos !== bytes.length || !frame || !scans) invalid();
      return frame;
    }
    if (marker === undefined || marker === 0 || marker === 0xd8 || (marker >= 0xd0 && marker <= 0xd7)) invalid();
    if (pos + 2 > bytes.length) invalid();
    const length = (bytes[pos] << 8) | bytes[pos + 1], end = pos + length;
    if (length < 2 || end > bytes.length) invalid();
    // Encoders may emit their own color/density/resource segments. Bounds-check
    // them like other segments; privacy metadata is stripped during re-encoding.
    if (marker === 0xe1 && locationMetadata(bytes, pos + 2, end)) invalid();
    if ((marker >= 0xc0 && marker <= 0xcf) && ![0xc4, 0xc8, 0xcc].includes(marker)) {
      if (frame || ![0xc0, 0xc1, 0xc2].includes(marker) || length < 11) invalid();
      const height = (bytes[pos + 3] << 8) | bytes[pos + 4];
      const width = (bytes[pos + 5] << 8) | bytes[pos + 6];
      const components = bytes[pos + 7];
      if (bytes[pos + 2] !== 8 || ![1, 3].includes(components) || length !== 8 + components * 3
          || width < 1 || height < 1 || width > maxEdge || height > maxEdge) invalid();
      frame = { width, height };
    }
    pos = end;
    if (marker === 0xda) {
      if (!frame || length < 8) invalid();
      scans++;
      // Skip stuffed entropy bytes and restart markers; return to the marker
      // loop at the next table/scan/end marker (including progressive images).
      while (pos < bytes.length) {
        if (bytes[pos] !== 0xff) { pos++; continue; }
        let next = pos + 1;
        while (bytes[next] === 0xff) next++;
        if (bytes[next] === 0 || (bytes[next] >= 0xd0 && bytes[next] <= 0xd7)) { pos = next + 1; continue; }
        break;
      }
    }
  }
  invalid();
}
