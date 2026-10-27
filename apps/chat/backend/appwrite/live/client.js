// Live Chat client for manual end-to-end checks against a real Appwrite project.
// It mirrors the app's Lua client paths (api/chat.lua, api/features.lua) and the
// Appwrite adapter envelope (api/appwrite_transport.lua), so a passing run
// exercises the same wire contract as the shipped app.

export class ChatError extends Error {
  constructor(status, code, message, layer = 'chat') {
    super(`${layer} ${status} ${code}: ${message}`);
    this.status = status; this.code = code; this.layer = layer;
  }
}

const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
const escape = value => encodeURIComponent(String(value)).replace(/[!'()*]/g, c => '%' + c.charCodeAt(0).toString(16).toUpperCase());

// Same normalization as api/features.lua identifier().
export function identifier(value) {
  if (typeof value !== 'string') return null;
  value = value.trim();
  if (value.startsWith('+')) {
    value = value.replace(/[\s()-]/g, '');
    return /^\+[1-9]\d+$/.test(value) && value.length >= 9 && value.length <= 16 ? { field: 'phone', value } : null;
  }
  return /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(value) ? { field: 'email', value: value.toLowerCase() } : null;
}

export class Appwrite {
  constructor({ endpoint, projectId, timeout = 60000 }) {
    this.endpoint = endpoint.replace(/\/$/, ''); this.projectId = projectId; this.timeout = timeout;
  }
  // Returns { status, headers, body } and never throws for HTTP errors. Appwrite
  // abuse limits (429) are retried after their advertised reset.
  async call(method, path, { body, headers = {}, binary = false } = {}) {
    for (let attempt = 0; ; attempt++) {
      const response = await fetch(this.endpoint + path, {
        method, redirect: 'error', signal: AbortSignal.timeout(this.timeout),
        headers: { 'X-Appwrite-Project': this.projectId, ...(body !== undefined ? { 'Content-Type': 'application/json' } : {}), ...headers },
        body: body === undefined ? undefined : JSON.stringify(body),
      });
      if (response.status === 429 && attempt < 3) {
        const reset = Number(response.headers.get('x-ratelimit-reset')) * 1000;
        await sleep(Math.min(Math.max(reset ? reset - Date.now() : 0, 2000 * 2 ** attempt), 65000));
        continue;
      }
      const raw = Buffer.from(await response.arrayBuffer());
      let parsed = raw;
      if (!binary || !response.ok) {
        try { parsed = raw.length ? JSON.parse(raw.toString('utf8')) : null; } catch { parsed = raw.toString('utf8'); }
      }
      return { status: response.status, headers: response.headers, body: parsed };
    }
  }
}

export class ChatClient {
  constructor(config, label = 'user') {
    this.config = config; this.label = label; this.api = new Appwrite(config);
    this.cookie = null; this.jwt = null; this.jwtAt = 0; this.user = null; this.lookups = [];
  }
  get id() { return this.user?.$id; }

  // ---- Native Appwrite Account (api/appwrite_auth.lua) ----
  async account(method, path, body) {
    const response = await this.api.call(method, path, { body, headers: this.cookie ? { Cookie: this.cookie } : {} });
    if (response.status < 200 || response.status >= 300) {
      throw new ChatError(response.status, response.body?.type || 'appwrite_error', response.body?.message || 'Account request failed.', 'account');
    }
    return response.body;
  }
  sessionCookie(response) {
    const name = 'a_session_' + this.config.projectId;
    let cookie, legacy;
    for (const line of response.headers.getSetCookie?.() ?? []) {
      const match = line.match(/^\s*([^=; ]+)=([^;\r\n]+)/);
      if (match?.[1] === name) cookie = match[1] + '=' + match[2];
      else if (match?.[1] === name + '_legacy') legacy = match[1] + '=' + match[2];
    }
    return cookie || legacy || null;
  }
  async signUp(email, password, name) {
    this.cookie = null;
    return this.account('POST', '/account', { userId: 'unique()', email, password, name });
  }
  async signIn(email, password) {
    this.cookie = null;
    const response = await this.api.call('POST', '/account/sessions/email', { body: { email, password } });
    if (response.status !== 201) {
      throw new ChatError(response.status, response.body?.type || 'appwrite_error', response.body?.message || 'Sign-in failed.', 'account');
    }
    this.cookie = this.sessionCookie(response);
    if (!this.cookie) throw new ChatError(502, 'invalid_response', 'Appwrite did not return a private session cookie.', 'account');
    await this.refresh();
    return this.user;
  }
  // Reuses a saved session cookie; false when Appwrite no longer accepts it.
  async restore(cookie) {
    this.cookie = cookie;
    try { await this.refresh(); return true; }
    catch (error) { if (error.status === 401) { this.cookie = null; return false; } throw error; }
  }
  async refresh() {
    this.user = await this.account('GET', '/account');
    const result = await this.account('POST', '/account/jwts', {});
    if (typeof result?.jwt !== 'string' || !result.jwt) throw new ChatError(502, 'invalid_response', 'Appwrite did not return a user JWT.', 'account');
    this.jwt = result.jwt; this.jwtAt = Date.now();
    return this.user;
  }
  async signOut() {
    try { await this.account('DELETE', '/account/sessions/current'); }
    finally { this.cookie = null; this.jwt = null; }
  }

  // ---- Chat Function execution (api/appwrite_transport.lua) ----
  // Returns { outer, execution, result } without interpreting the inner status.
  async execute(envelope, { jwt = this.jwt } = {}) {
    const response = await this.api.call('POST', `/functions/${this.config.functionId}/executions`, {
      headers: jwt ? { 'X-Appwrite-JWT': jwt } : {}, body: { body: JSON.stringify(envelope), async: false },
    });
    if (response.status < 200 || response.status >= 300) return { outer: response };
    const execution = response.body;
    let result;
    try { result = JSON.parse(execution.responseBody); } catch { result = undefined; }
    return { outer: response, execution, result };
  }
  // Returns the inner Chat { status, body, encoding }; Appwrite-level failures throw.
  async send(method, path, body, { encoding } = {}) {
    // JWTs last 15 minutes; renew early like the app does near expiry.
    if (!this.jwt || Date.now() - this.jwtAt > 10 * 60000) await this.refresh();
    const envelope = { method, path, ...(body !== undefined ? { body } : {}), ...(encoding ? { encoding } : {}) };
    for (let attempt = 0; ; attempt++) {
      const { outer, execution, result } = await this.execute(envelope);
      if (outer.status < 200 || outer.status >= 300) {
        throw new ChatError(outer.status, outer.body?.type || 'appwrite_error', outer.body?.message || 'Function execution was rejected.', 'execution');
      }
      if (execution.status !== 'completed' || !result || typeof result.status !== 'number') {
        throw new ChatError(502, 'execution_failed', `Execution ${execution.$id} ended as ${execution.status} `
          + `(HTTP ${execution.responseStatusCode}, ${execution.duration}s): ${String(execution.errors || execution.responseBody).slice(0, 300)}`, 'execution');
      }
      // A room lock held by a concurrent operation is a retryable condition, not a test result.
      if (result.status === 409 && result.body?.code === 'busy' && attempt < 4) { await sleep(1500 * (attempt + 1)); continue; }
      return result;
    }
  }
  async call(method, path, body, options = {}) {
    const result = await this.send(method, path, body, options);
    if (result.status < 200 || result.status >= 300) {
      throw new ChatError(result.status, result.body?.code || 'http_error', result.body?.message || 'The chat service rejected the request.');
    }
    if (options.binary) {
      if (result.encoding !== 'base64') throw new ChatError(502, 'invalid_response', 'Expected base64 photo bytes.');
      return Buffer.from(result.body, 'base64');
    }
    return result.body;
  }
  rpc(name, body = {}) { return this.call('POST', '/rest/v1/rpc/' + name, body); }

  // ---- The app's API surface (same paths and argument shapes as Lua) ----
  getProfile() { return this.rpc('get_chat_profile', {}); }
  updateProfile(values) { return this.rpc('update_chat_profile', { values }); }
  // The server allows 20 lookups per account per clock minute; pace ordinary
  // searches (leaving room for raw probes) so the suite never trips it by accident.
  async findPeople(query) {
    const now = Date.now();
    this.lookups = this.lookups.filter(at => at > now - 61000);
    if (this.lookups.length >= 16) await sleep(this.lookups[0] + 61000 - now);
    this.lookups.push(Date.now());
    const parsed = identifier(query);
    return this.rpc('find_chat_people', { query: parsed ? parsed.value : query.toLowerCase() });
  }
  findChannels(query) { return this.rpc('find_chat_channels', { query }); }
  joinPublicChannel(roomId) { return this.rpc('join_public_chat_channel', { target_room: roomId }); }
  listRooms() { return this.call('GET', '/rest/v1/chat_rooms?select=*,role,can_post&order=created_at.desc'); }
  listContacts() {
    return this.call('GET', '/rest/v1/chat_profiles?select=id,display_name&order=display_name.asc&id=neq.' + escape(this.id));
  }
  async createRoom(name, memberIds, { kind = 'group', visibility = 'private' } = {}) {
    const rows = await this.rpc('create_chat_room', { room_name: name, member_ids: memberIds, room_kind: kind, room_visibility: visibility });
    if (!Array.isArray(rows) || !rows[0]) throw new ChatError(502, 'invalid_response', 'The chat service did not return the new conversation.');
    return rows[0];
  }
  listMembers(roomId) { return this.rpc('list_chat_members', { target_room: roomId }); }
  setMemberRole(roomId, userId, role) { return this.rpc('set_chat_member_role', { target_room: roomId, target_user: userId, member_role: role }); }
  removeMember(roomId, userId) { return this.rpc('remove_chat_member', { target_room: roomId, target_user: userId }); }
  leaveRoom(roomId) { return this.rpc('leave_chat_room', { target_room: roomId }); }
  renameRoom(roomId, name) { return this.rpc('update_chat_room', { target_room: roomId, room_name: name }); }
  createInvite(roomId) { return this.rpc('create_chat_invite', { target_room: roomId }); }
  joinInvite(code) { return this.rpc('join_chat_invite', { invite_code: code }); }
  revokeInvite(roomId, code) { return this.rpc('revoke_chat_invite', { target_room: roomId, invite_code: code }); }
  // Chronological display order, like Client:listMessages.
  async listMessages(roomId, { limit = 100 } = {}) {
    const rows = await this.call('GET', '/rest/v1/chat_messages?select=*,attachments&room_id=eq.' + escape(roomId)
      + '&order=created_at.desc,id.desc&limit=' + limit);
    return [...rows].reverse();
  }
  async sendMessage(roomId, body, attachmentIds = []) {
    const rows = await this.rpc('post_chat_message', { target_room: roomId, message_body: body, attachment_ids: attachmentIds });
    if (!Array.isArray(rows) || !rows[0]) throw new ChatError(502, 'invalid_response', 'The chat service did not return the sent message.');
    return rows[0];
  }
  async editMessage(messageId, body) {
    const rows = await this.rpc('edit_chat_message', { target_message: messageId, message_body: body });
    if (!Array.isArray(rows) || !rows[0]) throw new ChatError(502, 'invalid_response', 'The chat service did not return the edited message.');
    return rows[0];
  }
  deleteMessage(messageId) { return this.rpc('delete_chat_message', { target_message: messageId }); }
  reservePhoto(roomId, photo) {
    return this.rpc('reserve_chat_photo', { target_room: roomId, photo_width: photo.width, photo_height: photo.height,
      photo_bytes: photo.jpeg.length, thumbnail_width: photo.thumbnailWidth, thumbnail_height: photo.thumbnailHeight,
      thumbnail_bytes: photo.thumbnail.length });
  }
  putObject(objectKey, bytes) {
    return this.call('POST', '/storage/v1/object/chat-photos/' + objectKey, Buffer.from(bytes).toString('base64'), { encoding: 'base64' });
  }
  finalizePhoto(attachmentId) { return this.call('POST', '/functions/v1/finalize-chat-photo', { attachment_id: attachmentId }); }
  async uploadPhoto(roomId, photo) {
    const attachment = await this.reservePhoto(roomId, photo);
    await this.putObject(attachment.object_key, photo.jpeg);
    await this.putObject(attachment.thumbnail_key, photo.thumbnail);
    return this.finalizePhoto(attachment.id);
  }
  async photoMetadata(attachmentId) {
    const rows = await this.call('GET', '/rest/v1/chat_attachments?select=*&id=eq.' + escape(attachmentId));
    return rows[0];
  }
  async getPhoto(attachmentId, thumbnail = false) {
    const attachment = await this.photoMetadata(attachmentId);
    if (!attachment) throw new ChatError(404, 'not_found', 'Photo is unavailable.');
    return this.readObject(thumbnail ? attachment.thumbnail_key : attachment.object_key);
  }
  readObject(objectKey) { return this.call('GET', '/storage/v1/object/authenticated/chat-photos/' + objectKey, undefined, { binary: true }); }
  async discardPhoto(attachmentId) {
    const discarded = await this.rpc('discard_chat_photo', { attachment_id: attachmentId });
    await this.call('DELETE', '/storage/v1/object/chat-photos', { prefixes: [discarded.object_key, discarded.thumbnail_key] });
    return discarded;
  }
  requestIdentityLink(value) {
    const parsed = identifier(value);
    return this.call('PUT', '/auth/v1/user', { [parsed?.field || 'email']: parsed?.value || value });
  }

  // Public Octopus profile photos (no session), as in Client:getAvatars.
  async getAvatars(userIds) {
    const queries = [{ method: 'equal', attribute: 'userId', values: userIds },
      { method: 'select', values: ['userId', 'avatarId'] }, { method: 'limit', values: [100] }];
    const params = queries.map((query, index) => `queries[${index}]=` + escape(JSON.stringify(query))).join('&');
    const response = await this.api.call('GET', '/databases/octopus-db/collections/profiles/documents?' + params);
    if (response.status !== 200) throw new ChatError(response.status, response.body?.type || 'appwrite_error', response.body?.message || 'Profile photo lookup failed.', 'account');
    const avatars = Object.fromEntries(userIds.map(id => [id, false]));
    for (const doc of response.body.documents) if (doc.userId in avatars && doc.avatarId) avatars[doc.userId] = doc.avatarId;
    return avatars;
  }
}
