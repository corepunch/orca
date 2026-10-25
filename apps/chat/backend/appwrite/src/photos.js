import { createCipheriv, createDecipheriv, randomBytes } from 'node:crypto';
import { reject } from './rest.js';

function privateMetadata(bytes) {
  if (bytes.subarray(0, 6).toString('latin1') !== 'Exif\0\0') return /gpslatitude|gpslongitude/i.test(bytes.toString('latin1'));
  const data = bytes.subarray(6), little = data.toString('ascii', 0, 2) === 'II';
  if (!little && data.toString('ascii', 0, 2) !== 'MM') return true;
  const number = (offset, size) => {
    if (offset < 0 || offset + size > data.length) return null;
    return size === 2 ? little ? data.readUInt16LE(offset) : data.readUInt16BE(offset)
      : little ? data.readUInt32LE(offset) : data.readUInt32BE(offset);
  };
  if (number(2, 2) !== 42) return true;
  const offset = number(4, 4), count = offset === null ? null : number(offset, 2);
  if (count === null || offset + 2 + count * 12 + 4 > data.length) return true;
  for (let index = 0; index < count; index++) if (number(offset + 2 + index * 12, 2) === 0x8825) return true;
  return false;
}

// Same structural/privacy contract as api/jpeg.lua; this does not decode entropy.
export function inspectJpeg(bytes) {
  const invalid = () => reject(400, 'invalid_photo', 'Upload a complete JPEG without location metadata.');
  if (!Buffer.isBuffer(bytes) || bytes.length < 32 || bytes.length > 1000000 || bytes.readUInt16BE(0) !== 0xffd8) invalid();
  let at = 2, width, height, scans = 0;
  while (at < bytes.length) {
    if (bytes[at] !== 255) invalid();
    while (bytes[at] === 255) at++;
    const marker = bytes[at++];
    if (marker === 217) {
      if (at === bytes.length && width && scans) return { width, height, bytes: bytes.length };
      invalid();
    }
    if (!marker || marker === 216 || marker >= 208 && marker <= 215 || at + 2 > bytes.length) invalid();
    const length = bytes.readUInt16BE(at);
    if (length < 2 || at + length > bytes.length) invalid();
    if (marker === 225 && privateMetadata(bytes.subarray(at + 2, at + length))) invalid();
    if (marker >= 192 && marker <= 207 && ![196, 200, 204].includes(marker)) {
      if (width || ![192, 193, 194].includes(marker) || length < 11) invalid();
      height = bytes.readUInt16BE(at + 3); width = bytes.readUInt16BE(at + 5);
      const components = bytes[at + 7];
      if (bytes[at + 2] !== 8 || ![1, 3].includes(components) || length !== 8 + components * 3 || !width || !height) invalid();
    }
    at += length;
    if (marker === 218) {
      if (!width || length < 8) invalid();
      scans++;
      while (at < bytes.length) {
        if (bytes[at] !== 255) { at++; continue; }
        let next = at + 1; while (bytes[next] === 255) next++;
        if (bytes[next] === 0 || bytes[next] >= 208 && bytes[next] <= 215) at = next + 1;
        else break;
      }
    }
  }
  invalid();
}

export const newPhotoKey = () => randomBytes(32).toString('base64');
export function encryptPhoto(bytes, secret, objectKey) {
  const iv = randomBytes(12), cipher = createCipheriv('aes-256-gcm', Buffer.from(secret, 'base64'), iv);
  cipher.setAAD(Buffer.from(objectKey));
  const encrypted = Buffer.concat([cipher.update(bytes), cipher.final()]);
  return Buffer.concat([iv, cipher.getAuthTag(), encrypted]);
}
export function decryptPhoto(bytes, secret, objectKey) {
  if (bytes.length < 28) reject(500, 'photo_integrity', 'The stored photo is damaged.');
  try {
    const decipher = createDecipheriv('aes-256-gcm', Buffer.from(secret, 'base64'), bytes.subarray(0, 12));
    decipher.setAAD(Buffer.from(objectKey)); decipher.setAuthTag(bytes.subarray(12, 28));
    return Buffer.concat([decipher.update(bytes.subarray(28)), decipher.final()]);
  } catch { reject(500, 'photo_integrity', 'The stored photo is damaged.'); }
}

export function decodeUpload(body, encoding) {
  if (encoding !== 'base64' || typeof body !== 'string' || body.length > 1333336
      || body.length % 4 !== 0 || !/^[A-Za-z0-9+/]*={0,2}$/.test(body)) {
    reject(400, 'invalid_photo', 'Provide a base64 encoded JPEG.');
  }
  const bytes = Buffer.from(body, 'base64');
  if (bytes.toString('base64') !== body) reject(400, 'invalid_photo', 'Provide a canonical base64 encoded JPEG.');
  inspectJpeg(bytes); return bytes;
}
