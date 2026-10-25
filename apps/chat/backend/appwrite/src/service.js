import { createHash, randomBytes, randomUUID } from 'node:crypto';
import { eq, key, order, reject } from './rest.js';
import { identifier } from './auth.js';
import { decryptPhoto, encryptPhoto, inspectJpeg, newPhotoKey } from './photos.js';
import { resolveRoute } from './routes.js';

const staff = member => member && ['owner', 'admin'].includes(member.role);
const posting = (room, member) => member && (room.kind !== 'channel' || staff(member));
const validId = value => typeof value === 'string' && /^[A-Za-z0-9][A-Za-z0-9._-]{0,35}$/.test(value);
const id = value => { if (!validId(value)) reject(400, 'validation', 'Choose a valid resource.'); return value; };
const memberId = (roomId, userId) => key('member:' + roomId + ':' + userId);
const publicProfile = profile => ({ id: profile.id, display_name: profile.display_name, username: profile.username });
async function mapConcurrent(items, action, width = 8) {
  const results = new Array(items.length); let next = 0, failure;
  await Promise.all(Array.from({ length: Math.min(width, items.length) }, async () => {
    while (!failure && next < items.length) {
      const index = next++;
      try { results[index] = await action(items[index], index); } catch (error) { failure = error; }
    }
  }));
  if (failure) throw failure;
  return results;
}

export class ChatService {
  constructor(repo, auth, { now = Date.now } = {}) { this.repo = repo; this.auth = auth; this.now = now; }
  timestamp() { return new Date(this.now()).toISOString(); }
  async profile(user) {
    return this.repo.lock('profile:' + user.$id, async () => {
      const previous = await this.repo.get('profiles', user.$id), email = user.emailVerification ? user.email.toLowerCase() : null,
        phone = user.phoneVerification ? user.phone : null;
      const row = { display_name: previous?.display_name ?? user.name ?? '', username: previous?.username || null,
        email, phone, discoverable_email: !!(email && previous?.email === email && previous.discoverable_email),
        discoverable_phone: !!(phone && previous?.phone === phone && previous.discoverable_phone) };
      if (!previous) return this.repo.create('profiles', row, user.$id);
      if (JSON.stringify({ ...previous, id: undefined }) !== JSON.stringify(row)) return this.repo.update('profiles', user.$id, row);
      return previous;
    });
  }
  async profileById(userId) {
    const previous = await this.repo.get('profiles', userId);
    if (previous) return previous;
    return this.profile(await this.auth.rest.request('GET', '/users/' + encodeURIComponent(id(userId))));
  }
  async updateProfile(user, values) {
    if (!values || typeof values !== 'object' || Array.isArray(values)) reject(400, 'validation', 'Provide profile changes.');
    const allowed = ['display_name', 'username', 'discoverable_email', 'discoverable_phone'];
    if (Object.keys(values).some(field => !allowed.includes(field))) reject(400, 'validation', 'Profiles cannot change login identifiers.');
    return this.repo.lock('profile:' + user.$id, async () => {
      const row = await this.repo.get('profiles', user.$id);
      if ('display_name' in values) {
        if (typeof values.display_name !== 'string' || !values.display_name.trim() || [...values.display_name].length > 80) reject(400, 'validation', 'Use a display name of 1–80 characters.');
        row.display_name = values.display_name.trim();
      }
      if ('username' in values) {
        if (typeof values.username !== 'string' || values.username && !/^[a-z][a-z0-9_]{2,31}$/.test(values.username)) reject(400, 'validation', 'Use a lowercase username of 3–32 letters, digits or underscores.');
        row.username = values.username || null;
      }
      for (const field of ['email', 'phone']) if ('discoverable_' + field in values) {
        const value = values['discoverable_' + field];
        if (typeof value !== 'boolean' || value && !row[field]) reject(400, 'validation', 'Discovery needs a verified identifier.');
        row['discoverable_' + field] = value;
      }
      return this.repo.update('profiles', user.$id, row);
    });
  }
  async findPeople(user, query) {
    if (typeof query !== 'string' || !query.trim() || query.length > 254) reject(400, 'validation', 'Enter an exact username, email or phone number.');
    await this.auth.limit('discovery:' + user.$id, 20, 60);
    let field = 'username', value = query.trim().toLowerCase().replace(/^@/, '');
    try { const parsed = identifier(query); field = parsed.field; value = parsed.value; } catch { /* An exact username is also allowed. */ }
    const matches = await this.repo.list('profiles', [eq(field, value)]), result = [];
    for (const profile of matches) {
      if (profile.id === user.$id || field !== 'username' && !profile['discoverable_' + field]) continue;
      if (field !== 'username') {
        const account = await this.auth.rest.request('GET', '/users/' + encodeURIComponent(profile.id));
        if (!account[field + 'Verification'] || account[field]?.toLowerCase() !== value) continue;
      }
      result.push(publicProfile(profile));
    }
    return result;
  }
  async membership(roomId, userId) { return this.repo.get('members', memberId(id(roomId), id(userId))); }
  async access(roomId, userId, needsPosting = false) {
    const room = await this.repo.get('rooms', id(roomId)), member = await this.membership(roomId, userId);
    if (!room || !member) reject(403, 'forbidden', 'You are not a member of this conversation.');
    if (needsPosting && !posting(room, member)) reject(403, 'forbidden', 'Only conversation staff can post here.');
    return { room, member };
  }
  roomView(room, member) {
    const { direct_key, ...result } = room; return { ...result, role: member.role, can_post: !!posting(room, member) };
  }
  async listRooms(user) {
    const memberships = await this.repo.list('members', [eq('user_id', user.$id)]), rooms = [];
    await mapConcurrent(memberships, async member => {
      const room = await this.repo.get('rooms', member.room_id);
      if (room) rooms.push(this.roomView(room, member));
    });
    return rooms.sort((a, b) => b.created_at.localeCompare(a.created_at));
  }
  async createRoom(user, body) {
    const { room_name: name, member_ids: members, room_kind: kind = 'group', room_visibility: visibility = 'private' } = body;
    if (typeof name !== 'string' || !name.trim() || [...name].length > 80 || !Array.isArray(members) || members.length > 50
      || members.some(value => !validId(value)) || !['direct', 'group', 'channel'].includes(kind)
      || !['public', 'private'].includes(visibility) || visibility === 'public' && kind !== 'channel') reject(400, 'validation', 'Choose a valid conversation name, type and members.');
    const userIds = [...new Set(members)].filter(value => value !== user.$id);
    if (kind === 'direct' && (userIds.length !== 1 || members.length !== 1 || visibility !== 'private')) reject(400, 'validation', 'Choose exactly one other person for a private direct chat.');
    const directKey = kind === 'direct' ? createHash('sha256').update([user.$id, userIds[0]].sort().join('\0')).digest('hex') : null;
    return this.repo.lock('create:' + (directKey || randomUUID()), async () => {
      if (directKey) {
        const existing = await this.repo.list('rooms', [eq('direct_key', directKey)], 1);
        if (existing[0]) return [this.roomView(existing[0], await this.membership(existing[0].id, user.$id))];
      }
      await mapConcurrent(userIds, userId => this.auth.rest.request('GET', '/users/' + encodeURIComponent(userId)));
      const roomId = randomUUID(), created = [], room = { name: name.trim(), kind, visibility, direct_key: directKey, created_by: user.$id, created_at: this.timestamp() };
      try {
        await mapConcurrent([user.$id, ...userIds], async userId => {
          const member = await this.repo.create('members', { room_id: roomId, user_id: userId,
            role: userId === user.$id ? 'owner' : kind === 'channel' ? 'subscriber' : 'member', created_at: this.timestamp() }, memberId(roomId, userId));
          created.push(member);
        });
        // Room creation is the visibility commit; unfinished membership rows cannot authorize anything.
        const committed = await this.repo.create('rooms', room, roomId);
        return [this.roomView(committed, created.find(member => member.user_id === user.$id))];
      } catch (error) {
        if (!await this.repo.get('rooms', roomId)) for (const member of created) await this.repo.remove('members', member.id);
        throw error;
      }
    });
  }
  async listMembers(user, roomId) {
    const { room, member } = await this.access(roomId, user.$id), members = await this.repo.list('members', [eq('room_id', roomId)]), result = [];
    await mapConcurrent(members.filter(other => room.kind !== 'channel' || staff(member) || other.user_id === user.$id || staff(other)), async other => {
      const profile = await this.profileById(other.user_id);
      result.push({ user_id: other.user_id, display_name: profile.display_name, username: profile.username, role: other.role });
    });
    return result;
  }
  async listContacts(user) {
    const rooms = await this.listRooms(user), contacts = new Map();
    for (const room of rooms) for (const member of await this.listMembers(user, room.id)) {
      if (member.user_id !== user.$id) contacts.set(member.user_id, { id: member.user_id, display_name: member.display_name, username: member.username });
    }
    return [...contacts.values()].sort((a, b) => a.display_name.localeCompare(b.display_name));
  }
  async setRole(user, body, remove = false) {
    const roomId = id(body.target_room), userId = id(body.target_user);
    return this.repo.lock('room:' + roomId, async () => {
      const { room, member } = await this.access(roomId, user.$id), target = await this.membership(roomId, userId);
      if (room.kind === 'direct' || !staff(member) || target?.role === 'owner') reject(403, 'forbidden', 'You cannot change this membership.');
      if (!target) reject(404, 'not_found', 'The member was not found.');
      if (remove) {
        if (member.role !== 'owner' && (staff(target) || target.user_id === user.$id)) reject(403, 'forbidden', 'Only the owner can remove staff.');
        await this.repo.remove('members', target.id); return true;
      }
      const allowed = room.kind === 'channel' ? ['admin', 'subscriber'] : ['admin', 'member'];
      if (member.role !== 'owner' || !allowed.includes(body.member_role)) reject(403, 'forbidden', 'Only the owner can assign valid member roles.');
      await this.repo.update('members', target.id, { ...target, role: body.member_role }); return true;
    });
  }
  async createInvite(user, roomId) {
    return this.repo.lock('room:' + id(roomId), async () => {
      const { room, member } = await this.access(roomId, user.$id);
      if (room.kind === 'direct' || !staff(member)) reject(403, 'forbidden', 'Only conversation staff can invite members.');
      const code = randomBytes(24).toString('hex');
      return this.repo.create('invites', { code, room_id: roomId, created_by: user.$id, created_at: this.timestamp(),
        expires_at: new Date(this.now() + 86400000).toISOString(), max_uses: 10, uses: 0, revoked: false }, key('invite:' + code));
    });
  }
  async joinInvite(user, code) {
    if (typeof code !== 'string' || !/^[a-f0-9]{48}$/.test(code)) reject(404, 'invalid_invite', 'The invitation is invalid or expired.');
    const inviteId = key('invite:' + code), initial = await this.repo.get('invites', inviteId);
    if (!initial) reject(404, 'invalid_invite', 'The invitation is invalid or expired.');
    return this.repo.lock('room:' + initial.room_id, async () => {
      const invite = await this.repo.get('invites', inviteId), room = await this.repo.get('rooms', initial.room_id);
      if (!room || !invite || invite.revoked || Date.parse(invite.expires_at) <= this.now()) reject(404, 'invalid_invite', 'The invitation is invalid or expired.');
      const old = await this.membership(room.id, user.$id);
      if (old) return this.roomView(room, old);
      if (invite.uses >= invite.max_uses) reject(400, 'invite_exhausted', 'The invitation has reached its usage limit.');
      // Reserve the allowance before membership creation. A failed creation may consume an
      // allowance, but concurrent workers can never grant more than max_uses memberships.
      await this.repo.update('invites', invite.id, { ...invite, uses: invite.uses + 1 });
      return this.roomView(room, await this.repo.create('members', { room_id: room.id, user_id: user.$id,
        role: room.kind === 'channel' ? 'subscriber' : 'member', created_at: this.timestamp() }, memberId(room.id, user.$id)));
    });
  }
  async revokeInvite(user, roomId, code) {
    return this.repo.lock('room:' + id(roomId), async () => {
      const { room, member } = await this.access(roomId, user.$id);
      if (room.kind === 'direct' || !staff(member)) reject(403, 'forbidden', 'Only conversation staff can revoke invitations.');
      const invite = typeof code === 'string' ? await this.repo.get('invites', key('invite:' + code)) : null;
      if (!invite || invite.room_id !== roomId) return false;
      await this.repo.update('invites', invite.id, { ...invite, revoked: true }); return true;
    });
  }
  async findChannels(query) {
    if (typeof query !== 'string' || query.length > 80) reject(400, 'validation', 'Use a channel name of at most 80 characters.');
    if (query.trim().length < 2) return [];
    const rooms = await this.repo.list('rooms', [eq('kind', 'channel'), eq('visibility', 'public')]);
    return rooms.filter(room => room.name.toLowerCase().includes(query.trim().toLowerCase())).map(room => ({ id: room.id, name: room.name, kind: room.kind, visibility: room.visibility, created_at: room.created_at }));
  }
  async joinPublic(user, roomId) {
    return this.repo.lock('room:' + id(roomId), async () => {
      const room = await this.repo.get('rooms', roomId);
      if (!room || room.kind !== 'channel' || room.visibility !== 'public') reject(404, 'not_found', 'Public channel not found.');
      let member = await this.membership(roomId, user.$id);
      if (!member) member = await this.repo.create('members', { room_id: roomId, user_id: user.$id, role: 'subscriber', created_at: this.timestamp() }, memberId(roomId, user.$id));
      return this.roomView(room, member);
    });
  }
  photoView(row, messageId = row.message_id) {
    const { secret, uploads, message_id, ...result } = row;
    return { ...result, status: messageId ? 'attached' : row.status, ...(messageId ? { message_id: messageId } : {}) };
  }
  async messageView(message, knownRoom) {
    const { attachment_ids, ...result } = message; result.attachments = [];
    const room = knownRoom || await this.repo.get('rooms', message.room_id);
    if (room?.kind === 'channel') result.sender_name = room.name;
    for (const attachmentId of attachment_ids || []) {
      const attachment = await this.repo.get('attachments', attachmentId);
      if (attachment?.message_id === message.id) result.attachments.push(this.photoView(attachment, message.id));
    }
    return result;
  }
  async listMessages(user, roomId, limit) {
    const { room } = await this.access(roomId, user.$id);
    limit = limit === undefined ? 100 : Number(limit);
    if (!Number.isInteger(limit) || limit < 1 || limit > 100) reject(400, 'validation', 'Message limit must be between 1 and 100.');
    const rows = await this.repo.list('messages', [eq('room_id', roomId), order('$createdAt', true), order('$id', true)], limit);
    return mapConcurrent(rows, row => this.messageView(row, room));
  }
  async postMessage(user, body) {
    const roomId = id(body.target_room), text = body.message_body ?? '', attachmentIds = body.attachment_ids ?? [];
    if (typeof text !== 'string' || Buffer.byteLength(text) > 4000 || !Array.isArray(attachmentIds) || attachmentIds.length > 4
      || new Set(attachmentIds).size !== attachmentIds.length || attachmentIds.some(value => !validId(value)) || !text.trim() && !attachmentIds.length) reject(400, 'validation', 'Write a message or attach up to four photos (maximum 4000 text bytes).');
    return this.repo.lock('room:' + roomId, async () => {
      const { room } = await this.access(roomId, user.$id, true);
      const attachments = [];
      for (const attachmentId of attachmentIds) {
        const photo = await this.repo.get('attachments', attachmentId);
        if (!photo || photo.room_id !== roomId || photo.uploader_id !== user.$id || photo.status !== 'ready'
          || Date.parse(photo.expires_at) <= this.now() || photo.message_id && await this.repo.get('messages', photo.message_id)) reject(403, 'invalid_attachment', 'Attach only your ready, unexpired photos in this conversation.');
        attachments.push(photo);
      }
      const messageId = randomUUID();
      for (const photo of attachments) await this.repo.update('attachments', photo.id, { ...photo, message_id: messageId });
      // The single immutable message document is the commit marker. Attachment rows
      // referencing an absent message remain private and may be retried or discarded.
      const message = await this.repo.create('messages', { room_id: roomId, sender_id: user.$id, body: text, created_at: this.timestamp(), attachment_ids: attachmentIds }, messageId);
      return [await this.messageView(message, room)];
    });
  }
  async reservePhoto(user, body) {
    const roomId = id(body.target_room), fields = ['photo_width', 'photo_height', 'photo_bytes', 'thumbnail_width', 'thumbnail_height', 'thumbnail_bytes'];
    if (fields.some(field => !Number.isInteger(body[field]) || body[field] < 1) || body.photo_bytes > 1000000 || body.thumbnail_bytes > 1000000
      || Math.max(body.photo_width, body.photo_height) > 1600 || Math.max(body.thumbnail_width, body.thumbnail_height) > 256) reject(400, 'validation', 'Photo size or dimensions exceed the upload limit.');
    return this.repo.lock('uploads:' + user.$id, async () => {
      const recent = await this.repo.list('attachments', [eq('uploader_id', user.$id)]);
      await this.cleanupExpiredPhotos(recent, 3);
      return this.repo.lock('room:' + roomId, async () => {
      await this.access(roomId, user.$id, true);
      let unfinished = 0;
      for (const row of recent) if (row.status !== 'discarded' && Date.parse(row.expires_at) > this.now()
        && (!row.message_id || !await this.repo.get('messages', row.message_id))) unfinished++;
      if (unfinished >= 20) reject(429, 'upload_limit', 'Finish or discard existing photo uploads first.');
      const attachmentId = randomUUID(), prefix = user.$id + '/' + roomId + '/' + attachmentId;
      return this.photoView(await this.repo.create('attachments', { room_id: roomId, uploader_id: user.$id, status: 'pending',
        width: body.photo_width, height: body.photo_height, byte_count: body.photo_bytes, thumbnail_width: body.thumbnail_width,
        thumbnail_height: body.thumbnail_height, thumbnail_byte_count: body.thumbnail_bytes, object_key: prefix + '/photo.jpg', thumbnail_key: prefix + '/thumbnail.jpg',
        created_at: this.timestamp(), expires_at: new Date(this.now() + 3600000).toISOString(), secret: newPhotoKey(), uploads: {} }, attachmentId));
      });
    });
  }
  async cleanupExpiredPhotos(rows, maximum = 100) {
    let removed = 0;
    for (const initial of rows) {
      if (removed >= maximum) break;
      if (Date.parse(initial.expires_at) > this.now()) continue;
      await this.repo.lock('room:' + initial.room_id, async () => {
        const row = await this.repo.get('attachments', initial.id);
        if (!row || Date.parse(row.expires_at) > this.now() || row.message_id && await this.repo.get('messages', row.message_id)) return;
        await this.repo.removeFile(row.object_key); await this.repo.removeFile(row.thumbnail_key);
        await this.repo.remove('attachments', row.id); removed++;
      });
    }
    return removed;
  }
  async photoAccess(user, attachmentId, unpublishedOnly = false) {
    const row = await this.repo.get('attachments', id(attachmentId));
    if (!row || row.status === 'discarded') reject(404, 'not_found', 'Photo is unavailable.');
    await this.access(row.room_id, user.$id);
    const committed = row.message_id && await this.repo.get('messages', row.message_id);
    if (committed && unpublishedOnly || !committed && (row.uploader_id !== user.$id || Date.parse(row.expires_at) <= this.now())) reject(403, 'forbidden', 'Photo is unavailable.');
    return { row, committed };
  }
  async uploadPhoto(user, objectKey, bytes) {
    const rows = await this.repo.list('attachments', [eq(objectKey.endsWith('/thumbnail.jpg') ? 'thumbnail_key' : 'object_key', objectKey)], 1);
    if (!rows[0]) reject(404, 'not_found', 'Photo reservation not found.');
    return this.repo.lock('room:' + rows[0].room_id, async () => {
      const { row } = await this.photoAccess(user, rows[0].id, true);
      await this.access(row.room_id, user.$id, true);
      if (row.status !== 'pending' || row.uploader_id !== user.$id) reject(403, 'forbidden', 'This photo cannot be uploaded.');
      const thumbnail = objectKey === row.thumbnail_key, name = thumbnail ? 'thumbnail' : 'photo', info = inspectJpeg(bytes);
      if (info.width !== (thumbnail ? row.thumbnail_width : row.width) || info.height !== (thumbnail ? row.thumbnail_height : row.height)
        || info.bytes !== (thumbnail ? row.thumbnail_byte_count : row.byte_count)) reject(400, 'invalid_photo', 'JPEG data does not match the reserved dimensions and size.');
      if (row.uploads[name]) reject(409, 'immutable_photo', 'Uploaded photos cannot be overwritten.');
      await this.repo.putFile(objectKey, encryptPhoto(bytes, row.secret, objectKey));
      await this.repo.update('attachments', row.id, { ...row, uploads: { ...row.uploads, [name]: true } });
      return { Key: objectKey };
    });
  }
  async finalizePhoto(user, attachmentId) {
    const initial = (await this.photoAccess(user, attachmentId, true)).row;
    return this.repo.lock('room:' + initial.room_id, async () => {
      const { row } = await this.photoAccess(user, attachmentId, true);
      await this.access(row.room_id, user.$id, true);
      if (row.uploader_id !== user.$id || row.status !== 'pending' || !row.uploads.photo || !row.uploads.thumbnail) reject(400, 'invalid_photo', 'Upload both JPEG objects before finalizing.');
      for (const [objectKey, width, height, byteCount] of [[row.object_key, row.width, row.height, row.byte_count], [row.thumbnail_key, row.thumbnail_width, row.thumbnail_height, row.thumbnail_byte_count]]) {
        const info = inspectJpeg(decryptPhoto(await this.repo.getFile(objectKey), row.secret, objectKey));
        if (info.width !== width || info.height !== height || info.bytes !== byteCount) reject(400, 'invalid_photo', 'The stored photo does not match its reservation.');
      }
      return this.photoView(await this.repo.update('attachments', row.id, { ...row, status: 'ready' }));
    });
  }
  async readPhoto(user, objectKey) {
    const rows = await this.repo.list('attachments', [eq(objectKey.endsWith('/thumbnail.jpg') ? 'thumbnail_key' : 'object_key', objectKey)], 1);
    if (!rows[0]) reject(404, 'not_found', 'Photo is unavailable.');
    const { row } = await this.photoAccess(user, rows[0].id);
    return decryptPhoto(await this.repo.getFile(objectKey), row.secret, objectKey);
  }
  async discardPhoto(user, attachmentId) {
    const initial = await this.repo.get('attachments', id(attachmentId));
    if (!initial || initial.uploader_id !== user.$id) reject(404, 'not_found', 'Photo is unavailable.');
    return this.repo.lock('room:' + initial.room_id, async () => {
      const row = await this.repo.get('attachments', attachmentId);
      if (row.message_id && await this.repo.get('messages', row.message_id)) reject(403, 'forbidden', 'Published photos cannot be discarded.');
      await this.repo.update('attachments', row.id, { ...row, status: 'discarded' });
      await this.repo.removeFile(row.object_key); await this.repo.removeFile(row.thumbnail_key);
      return this.photoView({ ...row, status: 'discarded', message_id: undefined });
    });
  }
  async removeObjects(user, keys) {
    if (!Array.isArray(keys) || keys.length > 8) reject(400, 'validation', 'Choose photo objects to remove.');
    for (const objectKey of keys) {
      if (typeof objectKey !== 'string') reject(400, 'validation', 'Choose valid photo objects.');
      const rows = await this.repo.list('attachments', [eq(objectKey.endsWith('/thumbnail.jpg') ? 'thumbnail_key' : 'object_key', objectKey)], 1);
      if (!rows[0] || rows[0].uploader_id !== user.$id || rows[0].status !== 'discarded') reject(403, 'forbidden', 'Only discarded unpublished objects can be removed.');
      await this.repo.removeFile(objectKey);
    }
    return true;
  }
  async dispatch(envelope, identity) {
    if (!envelope || typeof envelope !== 'object' || !['GET', 'POST', 'PUT', 'DELETE'].includes(envelope.method)
      || typeof envelope.path !== 'string' || !envelope.path.startsWith('/') || envelope.path.length > 2048) reject(400, 'validation', 'Provide a valid Chat request.');
    const url = new URL(envelope.path, 'https://chat.invalid'), route = resolveRoute(envelope.method, url);
    if (!identity?.jwt || !identity.userId) reject(401, 'unauthenticated', 'Sign in to continue.');
    const user = await this.auth.account(identity.jwt);
    if (user.$id !== identity.userId) reject(401, 'identity_mismatch', 'The function caller does not match the verified account.');
    const profile = await this.profile(user);
    return route.handler(this, { user, profile, url, body: envelope.body ?? {}, encoding: envelope.encoding, objectKey: route.objectKey });
  }
}
