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
const publicProfile = profile => ({ id: profile.id, display_name: profile.display_name, username: profile.username,
  avatar_id: profile.avatar_id || null });
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
  constructor(repo, auth, { now = Date.now, pushEnabled = false, pushError = () => {}, pushTimeoutMs = 3000 } = {}) {
    this.repo = repo; this.auth = auth; this.now = now;
    this.pushEnabled = pushEnabled; this.pushError = pushError; this.pushTimeoutMs = pushTimeoutMs;
  }
  timestamp() { return new Date(this.now()).toISOString(); }
  async profile(user) {
    const email = user.emailVerification ? user.email.toLowerCase() : null, phone = user.phoneVerification ? user.phone : null;
    const values = previous => ({ display_name: previous?.display_name || user.name || '', username: previous?.username || null,
      avatar_id: previous?.avatar_id || null,
      email, phone, discoverable_email: !!(email && previous?.email === email && previous.discoverable_email),
      discoverable_phone: !!(phone && previous?.phone === phone && previous.discoverable_phone) });
    const unchanged = (previous, row) => previous && Object.entries(row).every(([field, value]) => previous[field] === value);
    const previous = await this.repo.get('profiles', user.$id);
    if (unchanged(previous, values(previous))) return previous;
    return this.repo.lock('profile:' + user.$id, async () => {
      const current = await this.repo.get('profiles', user.$id), row = values(current);
      if (!current) return this.repo.create('profiles', row, user.$id);
      if (!unchanged(current, row)) return this.repo.update('profiles', user.$id, row);
      return current;
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
        if (typeof values.display_name !== 'string' || !values.display_name.trim() || [...values.display_name].length > 128) reject(400, 'validation', 'Use a display name of 1–128 characters.');
        row.display_name = values.display_name.trim();
      }
      if ('username' in values) {
        if (typeof values.username !== 'string' || values.username && !/^[a-z0-9][a-z0-9._-]{0,35}$/.test(values.username))
          reject(400, 'validation', 'Use a lowercase account name of 1–36 letters, digits, dots, underscores or hyphens.');
        if (row.username === user.$id && values.username !== row.username)
          reject(400, 'stable_account_name', 'Your account name is also your account ID and cannot be changed.');
        if (values.username && values.username !== row.username) {
          try {
            const account = await this.auth.rest.request('GET', '/users/' + encodeURIComponent(values.username));
            if (account.$id !== user.$id) reject(409, 'handle_taken', 'This account name belongs to another account.');
          } catch (error) { if (error.status !== 404) throw error; }
        }
        row.username = values.username || null;
      }
      for (const field of ['email', 'phone']) if ('discoverable_' + field in values) {
        const value = values['discoverable_' + field];
        if (typeof value !== 'boolean' || value && !row[field]) reject(400, 'validation', 'Discovery needs a verified identifier.');
        row['discoverable_' + field] = value;
      }
      const saved = await this.repo.update('profiles', user.$id, row);
      if (saved.display_name !== user.name) {
        try { await this.auth.rest.request('PATCH', '/users/' + encodeURIComponent(user.$id) + '/name', { body: { name: saved.display_name } }); }
        catch { reject(503, 'profile_sync', 'Your shared profile was saved, but the account display name could not be synced. Retry Save to finish syncing.'); }
      }
      return saved;
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
  // A direct chat names the other person, so clients can show their profile photo.
  async withPeer(view, userId) {
    if (view.kind !== 'direct') return view;
    const peer = (await this.repo.list('members', [eq('room_id', view.id)], 2)).find(member => member.user_id !== userId);
    return peer ? { ...view, peer_id: peer.user_id } : view;
  }
  async listRooms(user) {
    const memberships = await this.repo.list('members', [eq('user_id', user.$id)]), rooms = [];
    await mapConcurrent(memberships, async member => {
      const room = await this.repo.get('rooms', member.room_id);
      if (room) rooms.push(await this.withPeer(this.roomView(room, member), user.$id));
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
        if (existing[0]) return [await this.withPeer(this.roomView(existing[0], await this.membership(existing[0].id, user.$id)), user.$id)];
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
        const view = this.roomView(committed, created.find(member => member.user_id === user.$id));
        return [directKey ? { ...view, peer_id: userIds[0] } : view];
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
    const memberships = await this.repo.list('members', [eq('user_id', user.$id)]), peers = new Set();
    await mapConcurrent(memberships, async membership => {
      const room = await this.repo.get('rooms', membership.room_id);
      if (!room) return;
      const member = await this.membership(room.id, user.$id);
      if (!member) reject(403, 'forbidden', 'You are not a member of this conversation.');
      const members = await this.repo.list('members', [eq('room_id', room.id)]);
      for (const other of members) if (other.user_id !== user.$id && (room.kind !== 'channel' || staff(member) || staff(other))) peers.add(other.user_id);
    });
    const contacts = await mapConcurrent([...peers], async userId => publicProfile(await this.profileById(userId)));
    return contacts.sort((a, b) => a.display_name.localeCompare(b.display_name));
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
  async leaveRoom(user, roomId) {
    return this.repo.lock('room:' + id(roomId), async () => {
      const { room, member } = await this.access(roomId, user.$id);
      if (room.kind === 'direct' || member.role === 'owner') reject(403, 'forbidden', 'Only non-owner group and channel members can leave.');
      await this.repo.remove('members', member.id); return true;
    });
  }
  async updateRoom(user, body) {
    const roomId = id(body.target_room);
    if (Object.keys(body).some(field => !['target_room', 'room_name'].includes(field))) reject(400, 'validation', 'Only the conversation name can be changed.');
    const name = body.room_name;
    if (typeof name !== 'string' || !name.trim() || [...name].length > 80) reject(400, 'validation', 'Use a conversation name of 1–80 characters.');
    return this.repo.lock('room:' + roomId, async () => {
      const { room, member } = await this.access(roomId, user.$id);
      if (room.kind === 'direct' || member.role !== 'owner') reject(403, 'forbidden', 'Only the owner can rename a group or channel.');
      return this.roomView(await this.repo.update('rooms', roomId, { ...room, name: name.trim() }), member);
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
  async pageMessages(user, roomId, limit, before) {
    const { room } = await this.access(roomId, user.$id);
    if (limit === undefined) limit = 30;
    else if (typeof limit !== 'number' && (typeof limit !== 'string' || !/^-?\d+$/.test(limit)))
      reject(400, 'validation', 'Message limit must be a whole number.');
    limit = Number(limit);
    if (!Number.isSafeInteger(limit)) reject(400, 'validation', 'Message limit must be a whole number.');
    limit = Math.max(1, Math.min(100, limit));
    const queries = [eq('room_id', roomId), order('created_at', true), order('$id', true)];
    if (before !== undefined && before !== null && before !== '') {
      const anchor = await this.repo.get('messages', id(before));
      if (!anchor || anchor.room_id !== roomId)
        reject(409, 'stale_cursor', 'This history cursor is no longer available. Reload the latest messages.');
      queries.push({ method: 'cursorAfter', values: [before] });
    }
    let rows;
    try { rows = await this.repo.list('messages', queries, limit + 1); }
    catch (error) {
      // An anchor can be deleted after validation but before the list request.
      if (error.code === 'general_cursor_not_found' || before && error.status === 404) reject(409, 'stale_cursor', 'This history cursor is no longer available. Reload the latest messages.');
      throw error;
    }
    const hasMore = rows.length > limit, selected = rows.slice(0, limit);
    const cursor = hasMore ? selected.at(-1).id : null;
    return { messages: await mapConcurrent(selected.reverse(), row => this.messageView(row, room)),
      next_cursor: cursor, has_more: hasMore };
  }
  async listMessages(user, roomId, limit, before) {
    // Preserve the legacy descending rows response while the explicit page RPC
    // returns chronological messages and pagination metadata.
    return (await this.pageMessages(user, roomId, limit, before)).messages.reverse();
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
      await this.notifyMessage(message);
      return [await this.messageView(message, room)];
    });
  }
  // Called only after publication, under the room lock: recipients remain
  // members until their notifications have been queued. Appwrite owns delivery.
  async notifyMessage(message) {
    if (!this.pushEnabled) return;
    const report = error => {
      // Never log upstream bodies, message content, identifiers or device tokens.
      try { this.pushError('Chat push could not be queued' + (Number.isInteger(error?.status) ? ` (HTTP ${error.status})` : '') + '.'); }
      catch { /* Notification diagnostics must not fail a committed send. */ }
    };
    try {
      const signal = AbortSignal.timeout(this.pushTimeoutMs);
      const members = await this.repo.list('members', [eq('room_id', message.room_id)], undefined, { signal });
      const recipients = [...new Set(members.map(member => member.user_id))].filter(userId => userId !== message.sender_id);
      await mapConcurrent(recipients, async userId => {
        if (signal.aborted) return;
        try {
          await this.auth.rest.request('POST', '/messaging/messages/push', { signal, body: {
            messageId: key('push:' + message.id + ':' + userId), users: [userId],
            title: 'ORCA Chat', body: 'New message', sound: 'default', draft: false,
            data: { conversationId: message.room_id, messageId: message.id, userId },
          } });
        } catch (error) {
          // A lost response can safely be replayed with the same delivery ID.
          if (error.status !== 409) report(error);
        }
      });
    } catch (error) { report(error); }
  }
  // Edit and delete serialize on the room lock and re-read the message inside it,
  // so they never race a concurrent post, edit, delete or membership change.
  async messageAction(user, messageId, action) {
    const initial = await this.repo.get('messages', id(messageId));
    if (!initial) reject(404, 'not_found', 'The message was not found.');
    return this.repo.lock('room:' + initial.room_id, async () => {
      const message = await this.repo.get('messages', messageId);
      if (!message) reject(404, 'not_found', 'The message was not found.');
      return action(message, await this.access(message.room_id, user.$id));
    });
  }
  async editMessage(user, body) {
    const text = body.message_body;
    if (typeof text !== 'string' || Buffer.byteLength(text) > 4000) reject(400, 'validation', 'Write a message of at most 4000 text bytes.');
    return this.messageAction(user, body.target_message, async (message, { room, member }) => {
      // Only the author edits, and only while they may still post here.
      if (message.sender_id !== user.$id || !posting(room, member)) reject(403, 'forbidden', 'You can edit only your own messages.');
      if (!text.trim() && !message.attachment_ids?.length) reject(400, 'validation', 'A message without photos needs text.');
      if (text === message.body) return [await this.messageView(message, room)];
      const edited = await this.repo.update('messages', message.id, { ...message, body: text, edited_at: this.timestamp() });
      return [await this.messageView(edited, room)];
    });
  }
  async deleteMessage(user, body) {
    return this.messageAction(user, body.target_message, async (message, { room, member }) => {
      // Authors delete their own messages; group and channel staff moderate everyone's.
      if (message.sender_id !== user.$id && !(room.kind !== 'direct' && staff(member))) reject(403, 'forbidden', 'You cannot delete this message.');
      // Removing the commit marker unpublishes it at once; photos are then reclaimed.
      await this.repo.remove('messages', message.id);
      for (const attachmentId of message.attachment_ids || []) {
        const photo = await this.repo.get('attachments', attachmentId);
        if (photo?.message_id !== message.id) continue;
        await this.repo.removeFile(photo.object_key); await this.repo.removeFile(photo.thumbnail_key);
        await this.repo.remove('attachments', photo.id);
      }
      return true;
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
