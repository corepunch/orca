import assert from 'node:assert/strict';
import { randomBytes } from 'node:crypto';

// Structural JPEG accepted by the Function's inspector (photos.js). The scan
// carries random bytes so every upload is distinct.
export function jpeg(width, height, scan = randomBytes(24).map(byte => byte === 255 ? 0 : byte)) {
  return Buffer.concat([Buffer.from([255, 216, 255, 192, 0, 11, 8, height >> 8, height & 255, width >> 8, width & 255, 1, 1, 17, 0,
    255, 218, 0, 8, 1, 1, 0, 0, 63, 0]), scan, Buffer.from([255, 217])]);
}

// A JPEG whose EXIF points at a GPS IFD (tag 0x8825); the server must refuse it.
export function locatedJpeg(width, height) {
  const tiff = Buffer.from([77, 77, 0, 42, 0, 0, 0, 8, 0, 1, 0x88, 0x25, 0, 4, 0, 0, 0, 1, 0, 0, 0, 26, 0, 0, 0, 0]);
  const app1 = Buffer.concat([Buffer.from('Exif\0\0', 'latin1'), tiff]);
  const plain = jpeg(width, height);
  return Buffer.concat([plain.subarray(0, 2), Buffer.from([255, 225, (app1.length + 2) >> 8, (app1.length + 2) & 255]), app1, plain.subarray(2)]);
}

export function photo(width = 40, height = 30, thumbnailWidth = 8, thumbnailHeight = 6) {
  return { jpeg: jpeg(width, height), thumbnail: jpeg(thumbnailWidth, thumbnailHeight), width, height, thumbnailWidth, thumbnailHeight };
}

// Asserts that an API call fails with the expected Chat status (and code, if given).
export async function rejects(promise, status, code) {
  let failure;
  try { await promise; } catch (error) { failure = error; }
  assert.ok(failure, `expected HTTP ${status}${code ? ' ' + code : ''}, but the request succeeded`);
  if (!('status' in failure)) throw failure;
  const statuses = [].concat(status);
  assert.ok(statuses.includes(failure.status), `expected HTTP ${statuses.join(' or ')}${code ? ' ' + code : ''}, got ${failure.message}`);
  if (code) assert.ok([].concat(code).includes(failure.code), `expected code ${code}, got ${failure.message}`);
  return failure;
}

export const ids = rows => rows.map(row => row.id ?? row.user_id).sort();
export const byId = (rows, id) => rows.find(row => (row.id ?? row.user_id) === id);
