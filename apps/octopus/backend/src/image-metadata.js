// Header-only dimensions for operator backfills. No image decoding or dependencies.
export const MAX_HEADER_BYTES = 256 * 1024;

export class ImageMetadataError extends Error {
  constructor(code, message) { super(message); this.code = code; }
}
const invalid = message => { throw new ImageMetadataError('invalid_image_header', message); };
const pair = (width, height) => {
  if (!Number.isInteger(width) || !Number.isInteger(height) || width < 1 || height < 1 || width > 65535 || height > 65535)
    invalid('Image dimensions must be integers between 1 and 65535.');
  return { imageWidth: width, imageHeight: height };
};
export const validDimensions = row => Number.isInteger(row?.imageWidth) && Number.isInteger(row?.imageHeight)
  && row.imageWidth >= 1 && row.imageWidth <= 65535 && row.imageHeight >= 1 && row.imageHeight <= 65535;

function exifOrientation(bytes) {
  if (bytes.length < 6 || bytes.toString('ascii', 0, 6) !== 'Exif\0\0') return null;
  const tiff = bytes.subarray(6);
  if (tiff.length < 8) invalid('Truncated JPEG EXIF header.');
  const order = tiff.toString('ascii', 0, 2);
  if (order !== 'II' && order !== 'MM') invalid('Invalid JPEG EXIF byte order.');
  const u16 = offset => order === 'II' ? tiff.readUInt16LE(offset) : tiff.readUInt16BE(offset);
  const u32 = offset => order === 'II' ? tiff.readUInt32LE(offset) : tiff.readUInt32BE(offset);
  if (u16(2) !== 42) invalid('Invalid JPEG EXIF TIFF header.');
  const offset = u32(4);
  if (offset < 8 || offset + 2 > tiff.length) invalid('Invalid JPEG EXIF directory offset.');
  const count = u16(offset);
  if (offset + 2 + count * 12 > tiff.length) invalid('Truncated JPEG EXIF directory.');
  for (let i = 0; i < count; i++) {
    const entry = offset + 2 + i * 12;
    if (u16(entry) !== 0x112) continue;
    if (u16(entry + 2) !== 3 || u32(entry + 4) !== 1) invalid('Invalid JPEG EXIF orientation field.');
    const orientation = u16(entry + 8);
    // Unknown optional orientation values do not invalidate the JPEG frame.
    // Image decoders such as libvips default these to upright; match the Lua
    // upload parser and preserve the encoded dimensions when no valid tag exists.
    if (orientation < 1 || orientation > 8) return null;
    return orientation;
  }
  return null;
}

// null means more header bytes are needed. JPEG waits until SOS so EXIF after
// SOF is also applied before exposing the display aspect ratio.
export function parseImageDimensions(input) {
  const bytes = Buffer.isBuffer(input) ? input : Buffer.from(input);
  if (bytes.length < 2) return null;
  if (bytes[0] === 0x47 && bytes[1] === 0x49) {
    if (bytes.length < 6) return null;
    if (!['GIF87a', 'GIF89a'].includes(bytes.toString('ascii', 0, 6))) invalid('Invalid GIF signature.');
    if (bytes.length < 13) return null;
    return pair(bytes.readUInt16LE(6), bytes.readUInt16LE(8));
  }
  if (bytes[0] === 0x52 && bytes[1] === 0x49) {
    if (bytes.length < 12) return null;
    if (bytes.toString('ascii', 0, 4) !== 'RIFF' || bytes.toString('ascii', 8, 12) !== 'WEBP') invalid('Invalid WebP signature.');
    if (bytes.length < 20) return null;
    const kind = bytes.toString('ascii', 12, 16), length = bytes.readUInt32LE(16);
    if (kind === 'VP8X') {
      if (length !== 10) invalid('Invalid extended WebP header.');
      if (bytes.length < 30) return null;
      // EXIF may follow the entire compressed image, outside our bounded header.
      // Preserve fallback sizing rather than publish a potentially rotated ratio.
      if (bytes[20] & 8) invalid('WebP EXIF orientation cannot be verified from its bounded header; metadata was left unchanged.');
      return pair(bytes.readUIntLE(24, 3) + 1, bytes.readUIntLE(27, 3) + 1);
    }
    if (kind === 'VP8L') {
      if (length < 5) invalid('Invalid lossless WebP header.');
      if (bytes.length < 25) return null;
      if (bytes[20] !== 0x2f) invalid('Invalid lossless WebP signature.');
      const bits = bytes.readUInt32LE(21);
      return pair((bits & 0x3fff) + 1, ((bits >>> 14) & 0x3fff) + 1);
    }
    if (kind === 'VP8 ') {
      if (length < 10) invalid('Invalid lossy WebP header.');
      if (bytes.length < 30) return null;
      if (bytes[20] & 1 || bytes[23] !== 0x9d || bytes[24] !== 1 || bytes[25] !== 0x2a) invalid('Invalid lossy WebP frame.');
      return pair(bytes.readUInt16LE(26) & 0x3fff, bytes.readUInt16LE(28) & 0x3fff);
    }
    invalid('Unsupported WebP frame header.');
  }
  if (bytes[0] === 0x89 && bytes[1] === 0x50) {
    if (bytes.length < 8) return null;
    if (!bytes.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]))) invalid('Invalid PNG signature.');
    if (bytes.length < 33) return null;
    if (bytes.readUInt32BE(8) !== 13 || bytes.toString('ascii', 12, 16) !== 'IHDR') invalid('Invalid PNG IHDR header.');
    return pair(bytes.readUInt32BE(16), bytes.readUInt32BE(20));
  }
  if (bytes[0] !== 0xff || bytes[1] !== 0xd8) invalid('Only JPEG, PNG, GIF and WebP image headers are supported.');
  let offset = 2, dimensions, orientation = 1;
  while (offset < bytes.length) {
    if (bytes[offset++] !== 0xff) invalid('Invalid JPEG marker.');
    while (offset < bytes.length && bytes[offset] === 0xff) offset++;
    if (offset >= bytes.length) return null;
    const marker = bytes[offset++];
    if (marker === 0xda || marker === 0xd9) {
      if (!dimensions) invalid('JPEG contains no dimensions before image data.');
      return orientation >= 5 ? pair(dimensions.imageHeight, dimensions.imageWidth) : dimensions;
    }
    if (marker === 0 || marker === 0xd8) invalid('Invalid JPEG header marker.');
    if (marker === 0x01 || (marker >= 0xd0 && marker <= 0xd7)) continue;
    if (offset + 2 > bytes.length) return null;
    const length = bytes.readUInt16BE(offset);
    if (length < 2) invalid('Invalid JPEG segment length.');
    if (offset + length > bytes.length) return null;
    if (marker === 0xe1) orientation = exifOrientation(bytes.subarray(offset + 2, offset + length)) ?? orientation;
    if (marker >= 0xc0 && marker <= 0xcf && ![0xc4, 0xc8, 0xcc].includes(marker)) {
      if (length < 8) invalid('Invalid JPEG frame header.');
      dimensions = pair(bytes.readUInt16BE(offset + 5), bytes.readUInt16BE(offset + 3));
    }
    offset += length;
  }
  return null;
}

export async function readImageDimensions({ endpoint, projectId, apiKey, bucketId, fileId,
  fetch: fetcher = fetch, maxBytes = MAX_HEADER_BYTES, timeoutMs = 20000 }) {
  if (!Number.isInteger(maxBytes) || maxBytes < 33 || maxBytes > MAX_HEADER_BYTES)
    throw new Error(`Image header byte limit must be between 33 and ${MAX_HEADER_BYTES}.`);
  if (!apiKey) throw new Error('A server API key is required.');
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), timeoutMs);
  let reader;
  try {
    const url = `${endpoint.replace(/\/$/, '')}/storage/buckets/${encodeURIComponent(bucketId)}/files/${encodeURIComponent(fileId)}/download`;
    const response = await fetcher(url, { method: 'GET', redirect: 'error', signal: controller.signal,
      headers: { 'X-Appwrite-Project': projectId, 'X-Appwrite-Key': apiKey, Range: `bytes=0-${maxBytes - 1}` } });
    if (!response.ok) throw new ImageMetadataError('image_download_failed', `Image header download failed (HTTP ${response.status}).`);
    if (!response.body?.getReader) throw new ImageMetadataError('image_download_failed', 'Image download returned no readable stream.');
    reader = response.body.getReader();
    let bytes = Buffer.alloc(0);
    for (;;) {
      const { done, value } = await reader.read();
      if (done) throw new ImageMetadataError('truncated_image_header', 'Image ended before its dimensions could be read.');
      const remaining = maxBytes - bytes.length;
      const chunk = Buffer.from(value.buffer, value.byteOffset, Math.min(value.byteLength, remaining));
      bytes = Buffer.concat([bytes, chunk]);
      const dimensions = parseImageDimensions(bytes);
      if (dimensions) return dimensions;
      if (bytes.length >= maxBytes)
        throw new ImageMetadataError('image_header_limit', `Image dimensions were not found within the ${maxBytes}-byte header limit.`);
    }
  } catch (error) {
    if (error instanceof ImageMetadataError) throw error;
    throw new ImageMetadataError('image_download_failed', controller.signal.aborted
      ? 'Image header download timed out.' : 'Image header download failed; check storage access and retry.');
  } finally {
    clearTimeout(timeout);
    controller.abort();
    if (reader) await reader.cancel().catch(() => {});
  }
}
