import { createPublicKey, verify } from 'node:crypto';
import { eq, key, reject } from './rest.js';

// Public device keys for end-to-end encryption. Secret keys never reach the
// server; it only stores self-signed public keys and enforces who may read them.
export const MAX_ACTIVE_DEVICES = 10;
export const MAX_DEVICE_RECORDS = 50;
const SIGNATURE_LABEL = 'orca-chat/device/v1';
const SPKI_ED25519 = Buffer.from('302a300506032b6570032100', 'hex');

const deviceRecordId = (userId, deviceId) => key('device:' + userId + ':' + deviceId);
const userId = value => {
  if (typeof value !== 'string' || !/^[A-Za-z0-9][A-Za-z0-9._-]{0,35}$/.test(value)) reject(400, 'validation', 'Choose a valid person.');
  return value;
};
// Unpadded base64url of an exact byte length; re-encoding rejects non-canonical text.
function bytes(value, size) {
  if (typeof value !== 'string' || !/^[A-Za-z0-9_-]+$/.test(value)) return null;
  const decoded = Buffer.from(value, 'base64url');
  return decoded.length === size && decoded.toString('base64url') === value ? decoded : null;
}

// Signed bytes: label NUL user_id NUL device_id(16) box_public(32).
export function deviceSignatureMessage(owner, deviceId, boxPublic) {
  return Buffer.concat([Buffer.from(SIGNATURE_LABEL + '\0' + owner + '\0', 'utf8'), deviceId, boxPublic]);
}

function verified(owner, body) {
  const deviceId = bytes(body.device_id, 16), boxPublic = bytes(body.box_public, 32),
    signPublic = bytes(body.sign_public, 32), signature = bytes(body.self_signature, 64);
  if (!deviceId || !boxPublic || !signPublic || !signature)
    reject(400, 'validation', 'Provide a device ID and its public encryption and signing keys.');
  let valid = false;
  try {
    const publicKey = createPublicKey({ key: Buffer.concat([SPKI_ED25519, signPublic]), format: 'der', type: 'spki' });
    valid = verify(null, deviceSignatureMessage(owner, deviceId, boxPublic), publicKey, signature);
  } catch { valid = false; }
  if (!valid) reject(400, 'invalid_signature', 'The device key signature does not match.');
  return { device_id: body.device_id, box_public: body.box_public, sign_public: body.sign_public, self_signature: body.self_signature };
}

const view = row => ({ user_id: row.user_id, device_id: row.device_id, box_public: row.box_public,
  sign_public: row.sign_public, self_signature: row.self_signature, created_at: row.created_at, revoked_at: row.revoked_at || null });

export class Devices {
  constructor(service, { now = Date.now } = {}) { this.service = service; this.repo = service.repo; this.now = now; }
  async publish(user, body) {
    const device = verified(user.$id, body || {}), recordId = deviceRecordId(user.$id, device.device_id);
    return this.repo.lock('devices:' + user.$id, async () => {
      const existing = await this.repo.get('devices', recordId);
      if (existing) {
        const same = ['box_public', 'sign_public'].every(field => existing[field] === device[field]);
        if (!same) reject(409, 'device_conflict', 'This device ID already has different keys. Create a new device.');
        if (existing.revoked_at) reject(409, 'device_revoked', 'This device was signed out. Create a new device.');
        return view(existing);
      }
      const rows = await this.repo.list('devices', [eq('user_id', user.$id)], MAX_DEVICE_RECORDS + 1);
      if (rows.filter(row => !row.revoked_at).length >= MAX_ACTIVE_DEVICES)
        reject(409, 'device_limit', 'Sign out of another device before adding this one.');
      if (rows.length >= MAX_DEVICE_RECORDS)
        reject(409, 'device_limit', 'This account has reached its device history limit.');
      const row = await this.repo.create('devices', { user_id: user.$id, ...device,
        created_at: new Date(this.now()).toISOString(), revoked_at: null }, recordId);
      return view(row);
    });
  }
  async revoke(user, body) {
    if (!bytes(body?.device_id, 16)) reject(400, 'validation', 'Choose a valid device.');
    const recordId = deviceRecordId(user.$id, body.device_id);
    return this.repo.lock('devices:' + user.$id, async () => {
      const row = await this.repo.get('devices', recordId);
      if (!row || row.user_id !== user.$id) return false;
      if (!row.revoked_at) await this.repo.update('devices', recordId, { ...row, revoked_at: new Date(this.now()).toISOString() });
      return true;
    });
  }
  // Keys are listed for yourself and for people you share a direct chat or group
  // with. Others are omitted rather than refused, so the route is no directory oracle.
  async list(user, body) {
    const ids = body?.user_ids;
    if (!Array.isArray(ids) || !ids.length || ids.length > 50 || new Set(ids).size !== ids.length)
      reject(400, 'validation', 'Choose between 1 and 50 different people.');
    ids.forEach(userId);
    const allowed = new Set([user.$id]), rooms = new Map();
    const others = ids.filter(id => id !== user.$id);
    if (others.length) {
      const own = new Set((await this.repo.list('members', [eq('user_id', user.$id)])).map(row => row.room_id));
      const encryptable = async roomId => {
        if (!rooms.has(roomId)) rooms.set(roomId, this.repo.get('rooms', roomId).then(room => !!room && room.kind !== 'channel'));
        return rooms.get(roomId);
      };
      for (const other of others) {
        for (const membership of await this.repo.list('members', [eq('user_id', other)])) {
          if (own.has(membership.room_id) && await encryptable(membership.room_id)) { allowed.add(other); break; }
        }
      }
    }
    const result = [];
    for (const id of ids) if (allowed.has(id)) {
      const rows = await this.repo.list('devices', [eq('user_id', id)], MAX_DEVICE_RECORDS);
      result.push(...rows.map(view).sort((a, b) => a.created_at.localeCompare(b.created_at) || a.device_id.localeCompare(b.device_id)));
    }
    return result;
  }
}
