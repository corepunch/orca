import test from 'node:test';
import assert from 'node:assert/strict';
import { randomUUID } from 'node:crypto';
import { readFile } from 'node:fs/promises';
import { Auth } from '../src/auth.js';
import { AppwriteRest, Failure, Repository } from '../src/rest.js';
import { ChatService } from '../src/service.js';
import handler, { execute, runtimeIdentity } from '../src/main.js';
import { decryptPhoto, encryptPhoto, inspectJpeg, newPhotoKey } from '../src/photos.js';

const clone = value => JSON.parse(JSON.stringify(value));
class MemoryRepository {
  constructor() { this.rows = new Map(); this.files = new Map(); this.locks = new Set(); }
  table(name) { if (!this.rows.has(name)) this.rows.set(name, new Map()); return this.rows.get(name); }
  async get(name, id) { const row = this.table(name).get(id); return row ? clone(row) : null; }
  async create(name, row, id = randomUUID()) {
    if (this.table(name).has(id) || name === 'profiles' && row.username && [...this.table(name).values()].some(other => other.username === row.username)) throw new Failure(409, 'conflict', 'Document exists.');
    const stored = clone({ ...row, id }); this.table(name).set(id, stored); return clone(stored);
  }
  async update(name, id, row) {
    if (!this.table(name).has(id)) throw new Failure(404, 'not_found', 'Document absent.');
    this.table(name).set(id, clone({ ...row, id })); return this.get(name, id);
  }
  async remove(name, id) { return this.table(name).delete(id); }
  async list(name, queries = [], limit) {
    let rows = [...this.table(name).values()].map(clone);
    for (const query of queries) if (query.method === 'equal') rows = rows.filter(row => query.values.includes(row[query.attribute]));
    for (const query of [...queries].reverse()) if (query.method.startsWith('order')) rows.sort((a, b) => {
      const field = query.attribute === '$createdAt' ? 'created_at' : query.attribute === '$id' ? 'id' : query.attribute;
      return String(a[field]).localeCompare(String(b[field])) * (query.method === 'orderDesc' ? -1 : 1);
    });
    return limit ? rows.slice(0, limit) : rows;
  }
  async lock(name, action) {
    if (this.locks.has(name)) throw new Failure(409, 'busy', 'Retry.'); this.locks.add(name);
    try { return await action(); } finally { this.locks.delete(name); }
  }
  async putFile(objectKey, bytes) { if (this.files.has(objectKey)) throw new Failure(409, 'conflict', 'Immutable.'); this.files.set(objectKey, Buffer.from(bytes)); }
  async getFile(objectKey) { const file = this.files.get(objectKey); if (!file) throw new Failure(404, 'not_found', 'Absent.'); return Buffer.from(file); }
  async removeFile(objectKey) { this.files.delete(objectKey); }
}

function jpeg(width = 1, height = 1) {
  return Buffer.from([255, 216, 255, 192, 0, 11, 8, height >> 8, height & 255, width >> 8, width & 255, 1, 1, 17, 0,
    255, 218, 0, 8, 1, 1, 0, 0, 63, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 255, 217]);
}
function setup() {
  const repo = new MemoryRepository(), clock = { now: 1700000000000 }, calls = [], users = new Map();
  for (const name of ['owner', 'alice', 'bob', 'eve']) users.set(name, { $id: name, name, email: name + '@example.invalid', emailVerification: true, phone: '', phoneVerification: false });
  const rest = { async request(method, path, options = {}) {
    calls.push({ method, path, options: clone(options) });
    if (path === '/account') {
      if (method === 'GET' && users.has(options.jwt)) return clone(users.get(options.jwt));
      throw new Failure(401, 'unauthenticated', 'Invalid session.');
    }
    if (path.startsWith('/users/') && method === 'GET') {
      const user = users.get(path.slice('/users/'.length));
      if (!user) throw new Failure(404, 'not_found', 'Missing account.'); return clone(user);
    }
    if (path.startsWith('/users?')) return { users: [] };
    if (path.startsWith('/users/') && method === 'PATCH') {
      const [, , userId, kind, phone] = path.split('/'), row = users.get(userId);
      if (kind === 'email' || kind === 'phone') { row[kind] = options.body[kind]; row[kind + 'Verification'] = false; }
      else if (kind === 'verification') row[phone === 'phone' ? 'phoneVerification' : 'emailVerification'] = true;
      return clone(row);
    }
    if (path.includes('/targets') || path.startsWith('/messaging/messages/')) return {};
    throw new Error('Unexpected fake REST route: ' + method + ' ' + path);
  } };
  const auth = new Auth(rest, repo, { now: () => clock.now, otpHmacKey: 'test-secret-never-use-in-production-123', emailProviderId: 'email-provider', smsProviderId: 'sms-provider' });
  const service = new ChatService(repo, auth, { now: () => clock.now });
  const call = (jwt, path, body = {}, method = 'POST', extra = {}) => execute({ method, path, body, ...extra }, service, { jwt, userId: jwt });
  return { repo, auth, service, clock, calls, users, call };
}
async function room(fixture, kind = 'group', others = ['alice'], visibility = 'private') {
  const response = await fixture.call('owner', '/rest/v1/rpc/create_chat_room', { room_name: 'Conversation', room_kind: kind, room_visibility: visibility, member_ids: others });
  assert.equal(response.status, 200); return response.body[0];
}

test('every protected request validates the platform JWT, including deleted or forged accounts', async () => {
  const fixture = setup();
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).body.id, 'owner');
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).status, 200);
  assert.equal(fixture.calls.filter(call => call.path === '/account').length, 2);
  assert.equal((await fixture.call('forged', '/rest/v1/chat_rooms', {}, 'GET')).status, 401);
  fixture.users.delete('owner'); assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).status, 401);
});

test('direct chats reuse canonical pairs; membership and owners cannot be changed through direct routes', async () => {
  const fixture = setup(), first = await room(fixture, 'direct');
  const response = await fixture.call('alice', '/rest/v1/rpc/create_chat_room', { room_name: 'Other name', member_ids: ['owner'], room_kind: 'direct' });
  assert.equal(response.body[0].id, first.id);
  assert.equal(first.peer_id, 'alice'); assert.equal(response.body[0].peer_id, 'owner');
  const listed = await fixture.call('alice', '/rest/v1/chat_rooms', {}, 'GET');
  assert.equal(listed.body.find(value => value.id === first.id).peer_id, 'owner');
  assert.equal((await room(fixture)).peer_id, undefined);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/remove_chat_member', { target_room: first.id, target_user: 'alice' })).status, 403);
  assert.equal((await fixture.call('eve', '/rest/v1/chat_messages?room_id=eq.' + first.id, {}, 'GET')).status, 403);
});

test('channels expose staff and self to subscribers, enforce posting, role changes and removal', async () => {
  const fixture = setup(), channel = await room(fixture, 'channel', ['alice', 'bob'], 'public');
  let response = await fixture.call('alice', '/rest/v1/rpc/list_chat_members', { target_room: channel.id });
  assert.deepEqual(response.body.map(value => value.user_id).sort(), ['alice', 'owner']);
  response = await fixture.call('alice', '/rest/v1/chat_profiles', {}, 'GET');
  assert.deepEqual(response.body.map(value => value.id), ['owner']);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/post_chat_message', { target_room: channel.id, message_body: 'bad' })).status, 403);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/set_chat_member_role', { target_room: channel.id, target_user: 'bob', member_role: 'admin' })).status, 403);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: channel.id, target_user: 'alice', member_role: 'admin' })).status, 200);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/post_chat_message', { target_room: channel.id, message_body: 'allowed' })).status, 200);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/remove_chat_member', { target_room: channel.id, target_user: 'owner' })).status, 403);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/remove_chat_member', { target_room: channel.id, target_user: 'bob' })).status, 200);
  assert.equal((await fixture.call('bob', '/rest/v1/rpc/list_chat_members', { target_room: channel.id })).status, 403);
});

test('verified opt-in exact discovery and rate limits never reveal identifiers in public profiles', async () => {
  const fixture = setup();
  await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { username: 'alice_name' } });
  let result = await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: 'alice@example.invalid' }); assert.deepEqual(result.body, []);
  await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { discoverable_email: true } });
  result = await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: 'alice@example.invalid' });
  assert.equal(result.body[0].id, 'alice'); assert.equal(result.body[0].email, undefined);
  assert.deepEqual((await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: 'alice@' })).body, []);
  fixture.users.get('alice').emailVerification = false;
  assert.deepEqual((await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: 'alice@example.invalid' })).body, []);
  for (let index = 0; index < 16; index++) assert.equal((await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: '@alice_name' })).status, 200);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: '@alice_name' })).status, 429);
});

test('invites count only new memberships, expire, revoke, and never exceed allowance', async () => {
  const fixture = setup(), group = await room(fixture, 'group', []);
  const invite = (await fixture.call('owner', '/rest/v1/rpc/create_chat_invite', { target_room: group.id })).body;
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/join_chat_invite', { invite_code: invite.code })).status, 200);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/join_chat_invite', { invite_code: invite.code })).status, 200);
  assert.equal((await fixture.repo.get('invites', invite.id)).uses, 1);
  await fixture.repo.update('invites', invite.id, { ...invite, uses: 10 });
  assert.equal((await fixture.call('bob', '/rest/v1/rpc/join_chat_invite', { invite_code: invite.code })).status, 400);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/revoke_chat_invite', { target_room: group.id, invite_code: invite.code })).body, true);
  assert.equal((await fixture.call('eve', '/rest/v1/rpc/join_chat_invite', { invite_code: invite.code })).status, 404);
  fixture.clock.now += 60000;
  const next = (await fixture.call('owner', '/rest/v1/rpc/create_chat_invite', { target_room: group.id })).body;
  fixture.clock.now += 86400000;
  assert.equal((await fixture.call('bob', '/rest/v1/rpc/join_chat_invite', { invite_code: next.code })).status, 404);
});

async function upload(fixture, roomId) {
  const bytes = jpeg(12, 8), thumbnail = jpeg(3, 2);
  const reserved = await fixture.call('owner', '/rest/v1/rpc/reserve_chat_photo', { target_room: roomId,
    photo_width: 12, photo_height: 8, photo_bytes: bytes.length, thumbnail_width: 3, thumbnail_height: 2, thumbnail_bytes: thumbnail.length });
  assert.equal(reserved.status, 200); assert.equal(reserved.body.secret, undefined);
  for (const [objectKey, photo] of [[reserved.body.object_key, bytes], [reserved.body.thumbnail_key, thumbnail]]) {
    const result = await fixture.call('owner', '/storage/v1/object/chat-photos/' + objectKey, photo.toString('base64'), 'POST', { encoding: 'base64' });
    assert.equal(result.status, 200); assert.notDeepEqual(fixture.repo.files.get(objectKey), photo);
  }
  const ready = await fixture.call('owner', '/functions/v1/finalize-chat-photo', { attachment_id: reserved.body.id });
  assert.equal(ready.status, 200); return { photo: ready.body, bytes, thumbnail };
}
test('photos are encrypted, private before publication, membership protected after publication, immutable and tamper checked', async () => {
  const fixture = setup(), group = await room(fixture), { photo, bytes, thumbnail } = await upload(fixture, group.id);
  const photoPath = '/storage/v1/object/authenticated/chat-photos/' + photo.object_key;
  assert.equal((await fixture.call('alice', photoPath, {}, 'GET')).status, 403);
  assert.equal((await fixture.call('eve', photoPath, {}, 'GET')).status, 403);
  const sent = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, message_body: '', attachment_ids: [photo.id] });
  assert.equal(sent.status, 200); assert.equal(sent.body[0].attachments[0].status, 'attached'); assert.equal(sent.body[0].attachments[0].secret, undefined);
  const response = await fixture.call('alice', photoPath, {}, 'GET');
  assert.equal(response.encoding, 'base64'); assert.deepEqual(Buffer.from(response.body, 'base64'), bytes);
  const preview = await fixture.call('alice', '/storage/v1/object/authenticated/chat-photos/' + photo.thumbnail_key, {}, 'GET');
  assert.deepEqual(Buffer.from(preview.body, 'base64'), thumbnail);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/discard_chat_photo', { attachment_id: photo.id })).status, 403);
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: [photo.id] })).status, 403);
  fixture.repo.files.get(photo.object_key)[30] ^= 1;
  assert.equal((await fixture.call('alice', photoPath, {}, 'GET')).status, 500);
  await fixture.call('owner', '/rest/v1/rpc/remove_chat_member', { target_room: group.id, target_user: 'alice' });
  assert.equal((await fixture.call('alice', photoPath, {}, 'GET')).status, 403);
});

test('failed publication leaves photos private and retryable; expired reservations cannot publish', async () => {
  const fixture = setup(), group = await room(fixture), { photo } = await upload(fixture, group.id);
  const create = fixture.repo.create.bind(fixture.repo);
  fixture.repo.create = async (name, ...args) => { if (name === 'messages') throw new Error('Simulated crash before commit.'); return create(name, ...args); };
  let result = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: [photo.id] });
  assert.equal(result.status, 503);
  assert.equal((await fixture.call('alice', '/storage/v1/object/authenticated/chat-photos/' + photo.object_key, {}, 'GET')).status, 403);
  fixture.repo.create = create;
  result = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: [photo.id] }); assert.equal(result.status, 200);
  const next = await upload(fixture, group.id); fixture.clock.now += 3600000;
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: [next.photo.id] })).status, 403);
});

test('discard removes both encrypted objects and permits cleanup after removal from room', async () => {
  const fixture = setup(), group = await room(fixture), { photo } = await upload(fixture, group.id);
  const result = await fixture.call('owner', '/rest/v1/rpc/discard_chat_photo', { attachment_id: photo.id });
  assert.equal(result.status, 200); assert.equal(fixture.repo.files.size, 0);
  assert.equal((await fixture.call('owner', '/storage/v1/object/chat-photos', { prefixes: [photo.object_key, photo.thumbnail_key] }, 'DELETE')).status, 200);
});

test('identity linking stages proof without mutating Auth, enforces guesses and updates same account only', async () => {
  const fixture = setup(), before = clone(fixture.users.get('owner'));
  const result = await fixture.call('owner', '/auth/v1/user', { phone: '+49123456789' }, 'PUT');
  assert.equal(result.status, 200); assert.deepEqual(fixture.users.get('owner'), before);
  const delivery = fixture.calls.find(call => call.path === '/messaging/messages/sms'), code = delivery.options.body.content.match(/\b(\d{6})\b/)[1];
  const wrong = code === '000000' ? '000001' : '000000';
  assert.equal((await fixture.call('owner', '/auth/v1/verify', { phone: '+49123456789', type: 'phone_change', token: wrong })).status, 400);
  const verified = await fixture.call('owner', '/auth/v1/verify', { phone: '+49123456789', type: 'phone_change', token: code });
  assert.equal(verified.status, 200); assert.equal(verified.body.user.id, 'owner');
  assert.equal(fixture.users.get('owner').phoneVerification, true); assert.equal(fixture.users.get('owner').email, before.email);
  assert.equal((await fixture.call('owner', '/auth/v1/verify', { phone: '+49123456789', type: 'phone_change', token: code })).status, 400);
  assert.ok(fixture.calls.some(call => call.method === 'DELETE' && call.path.includes('/targets/')));
});

test('identity linking fails before Auth mutation without a provider, on expiry, or after five guesses', async () => {
  const fixture = setup(); fixture.auth.providers.phone = undefined;
  assert.equal((await fixture.call('owner', '/auth/v1/user', { phone: '+49123456789' }, 'PUT')).status, 503);
  assert.equal(fixture.calls.some(call => call.method === 'PATCH' && call.path.startsWith('/users/')), false);
  fixture.auth.providers.phone = 'sms-provider';
  await fixture.call('owner', '/auth/v1/user', { phone: '+49123456789' }, 'PUT');
  const delivered = fixture.calls.find(call => call.path === '/messaging/messages/sms').options.body.content.match(/\b(\d{6})\b/)[1];
  const wrong = delivered === '000000' ? '000001' : '000000';
  for (let attempt = 0; attempt < 5; attempt++) assert.equal((await fixture.call('owner', '/auth/v1/verify', { phone: '+49123456789', type: 'phone_change', token: wrong })).status, 400);
  assert.equal((await fixture.call('owner', '/auth/v1/verify', { phone: '+49123456789', type: 'phone_change', token: delivered })).status, 400);
  assert.equal(fixture.users.get('owner').phone, '');
});

test('JPEG inspection rejects truncation, malformed segments, and GPS metadata; AES binds each object key', () => {
  const bytes = jpeg(12, 8); assert.equal(inspectJpeg(bytes).width, 12);
  assert.throws(() => inspectJpeg(bytes.subarray(0, -1)), Failure);
  const gps = Buffer.from('gpslatitude=1'), segment = Buffer.alloc(4); segment[0] = 255; segment[1] = 225; segment.writeUInt16BE(gps.length + 2, 2);
  assert.throws(() => inspectJpeg(Buffer.concat([bytes.subarray(0, 2), segment, gps, bytes.subarray(2)])), Failure);
  const secret = newPhotoKey(), encrypted = encryptPhoto(bytes, secret, 'original');
  assert.deepEqual(decryptPhoto(encrypted, secret, 'original'), bytes);
  assert.throws(() => decryptPhoto(encrypted, secret, 'thumbnail'), Failure);
});

test('REST JWT requests omit server key and repository pagination loads every member beyond 100', async () => {
  let headers;
  const rest = new AppwriteRest({ endpoint: 'https://appwrite.invalid/v1', projectId: 'project', apiKey: 'server-secret', fetch: async (url, init) => {
    headers = init.headers; return new Response(JSON.stringify({ $id: 'user' }), { status: 200 });
  } });
  await rest.request('GET', '/account', { admin: false, jwt: 'user-jwt' });
  assert.equal(headers['X-Appwrite-Key'], undefined); assert.equal(headers['X-Appwrite-JWT'], 'user-jwt'); assert.equal(headers['X-Appwrite-Session'], undefined);
  const documents = Array.from({ length: 137 }, (_, index) => ({ $id: String(index), payload: JSON.stringify({ user_id: String(index) }) }));
  const repo = new Repository({ async request(method, path) {
    const queries = new URL(path, 'https://appwrite.invalid').searchParams.getAll('queries[]').map(JSON.parse), cursor = queries.find(query => query.method === 'cursorAfter');
    const start = cursor ? Number(cursor.values[0]) + 1 : 0;
    return { documents: documents.slice(start, start + 100) };
  } }, 'db', 'bucket');
  assert.equal((await repo.list('members')).length, 137);
});

test('schema collections deny direct access and manifest uses only required scopes', async () => {
  const schema = JSON.parse(await readFile(new URL('../schema.json', import.meta.url)));
  assert.equal(schema.collections.length, 7);
  for (const collection of schema.collections) { assert.deepEqual(collection.permissions, []); assert.equal(collection.documentSecurity, false); }
  const config = JSON.parse(await readFile(new URL('../appwrite.config.json', import.meta.url)));
  assert.deepEqual(config.functions[0].execute, ['users']);
  assert.equal(config.functions[0].scopes.includes('sessions.write'), false);
  assert.equal(config.functions[0].scopes.includes('keys.write'), false);
  assert.equal(config.functions[0].scopes.includes('functions.write'), false);
});

test('runtime identity comes only from platform headers and legacy envelope credentials cannot override it', async () => {
  assert.throws(() => runtimeIdentity({}), error => error.status === 401);
  assert.throws(() => runtimeIdentity({ 'x-appwrite-user-id': 'owner' }), error => error.status === 401);
  assert.deepEqual(runtimeIdentity({ 'x-appwrite-user-id': 'owner', 'x-appwrite-user-jwt': 'owner' }), { userId: 'owner', jwt: 'owner' });
  const fixture = setup(), envelope = { method: 'POST', path: '/rest/v1/rpc/get_chat_profile', session: 'eve', jwt: 'eve', body: {} };
  const missingRuntime = await handler({ req: { bodyJson: envelope, headers: { 'x-appwrite-key': 'execution-key' } }, res: { json: value => value } });
  assert.equal(missingRuntime.status, 401);
  assert.equal((await execute(envelope, fixture.service)).status, 401);
  const result = await execute(envelope, fixture.service, { jwt: 'owner', userId: 'owner' });
  assert.equal(result.status, 200); assert.equal(result.body.id, 'owner');
  assert.equal((await execute(envelope, fixture.service, { jwt: 'alice', userId: 'owner' })).status, 401);
  assert.equal(fixture.calls.filter(call => call.path === '/account').every(call => call.options.admin === false && !!call.options.jwt), true);
});

test('function exposes no password, signup, OTP login, refresh, or logout proxy and requires authenticated execution', async () => {
  const fixture = setup();
  for (const path of ['/auth/v1/signup', '/auth/v1/otp', '/auth/v1/token?grant_type=password', '/auth/v1/token?grant_type=refresh_token', '/auth/v1/logout']) {
    assert.equal((await fixture.call('owner', path, { email: 'owner@example.invalid', password: 'password123' })).status, 404);
  }
  assert.equal((await fixture.call('owner', '/auth/v1/verify', { email: 'owner@example.invalid', type: 'email', token: '123456' })).status, 404);
  assert.equal(fixture.calls.some(call => call.path.startsWith('/account/sessions') || call.path.startsWith('/account/tokens')), false);
});

test('a room with fifty other users commits all fifty-one memberships, with no partial room on failed member creation', async () => {
  const fixture = setup(), members = Array.from({ length: 50 }, (_, index) => 'user' + index);
  for (const userId of members) fixture.users.set(userId, { $id: userId, name: userId, email: '', emailVerification: false, phone: '', phoneVerification: false });
  const group = await room(fixture, 'group', members);
  assert.equal((await fixture.repo.list('members', [{ method: 'equal', attribute: 'room_id', values: [group.id] }])).length, 51);
  const create = fixture.repo.create.bind(fixture.repo);
  fixture.repo.create = async (name, row, ...args) => { if (name === 'members' && row.user_id === 'user7') throw new Error('Simulated member failure'); return create(name, row, ...args); };
  const failed = await fixture.call('owner', '/rest/v1/rpc/create_chat_room', { room_name: 'Failed group', member_ids: members });
  assert.equal(failed.status, 503);
  assert.equal((await fixture.repo.list('rooms')).length, 1); assert.equal((await fixture.repo.list('members')).length, 51);
});

test('expired unpublished photos are reclaimed lazily while attached photos remain intact', async () => {
  const fixture = setup(), group = await room(fixture), published = await upload(fixture, group.id), abandoned = await upload(fixture, group.id);
  await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: [published.photo.id] });
  fixture.clock.now += 3600001;
  const bytes = jpeg();
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/reserve_chat_photo', { target_room: group.id, photo_width: 1, photo_height: 1,
    photo_bytes: bytes.length, thumbnail_width: 1, thumbnail_height: 1, thumbnail_bytes: bytes.length })).status, 200);
  assert.equal(await fixture.repo.get('attachments', abandoned.photo.id), null);
  assert.ok(await fixture.repo.get('attachments', published.photo.id));
  assert.ok(fixture.repo.files.has(published.photo.object_key)); assert.equal(fixture.repo.files.has(abandoned.photo.object_key), false);
});

test('immutable rate claims enforce a shared quota under concurrency without persistent global locks', async () => {
  const fixture = setup();
  const outcomes = await Promise.allSettled(Array.from({ length: 40 }, () => fixture.auth.limit('shared', 20, 60)));
  assert.equal(outcomes.filter(result => result.status === 'fulfilled').length, 20);
  assert.ok(outcomes.filter(result => result.status === 'rejected').every(result => result.reason.status === 429));
  assert.equal(fixture.repo.locks.size, 0);
  fixture.clock.now += 60000;
  await fixture.auth.limit('shared', 20, 60);
});

test('authors edit their own text; edits keep photos, enforce limits and posting rights', async () => {
  const fixture = setup(), group = await room(fixture, 'group', ['alice']), { photo } = await upload(fixture, group.id);
  const post = (user, body, ids = []) => fixture.call(user, '/rest/v1/rpc/post_chat_message', { target_room: group.id, message_body: body, attachment_ids: ids });
  const edit = (user, target, body) => fixture.call(user, '/rest/v1/rpc/edit_chat_message', { target_message: target, message_body: body });
  const sent = (await post('owner', 'helo')).body[0];
  fixture.clock.now += 5000;
  const edited = await edit('owner', sent.id, 'hello');
  assert.equal(edited.status, 200);
  assert.equal(edited.body[0].body, 'hello'); assert.equal(edited.body[0].id, sent.id);
  assert.equal(edited.body[0].created_at, sent.created_at); assert.equal(edited.body[0].edited_at, new Date(fixture.clock.now).toISOString());
  const listed = (await fixture.call('alice', '/rest/v1/chat_messages?room_id=eq.' + group.id, {}, 'GET')).body;
  assert.deepEqual(listed.map(row => [row.body, !!row.edited_at]), [['hello', true]]);
  assert.equal((await edit('owner', sent.id, 'hello')).body[0].edited_at, edited.body[0].edited_at, 'unchanged text is not a new edit');
  assert.equal((await edit('alice', sent.id, 'hijack')).status, 403);
  assert.equal((await edit('eve', sent.id, 'hijack')).status, 403);
  assert.equal((await edit('owner', sent.id, '   ')).status, 400);
  assert.equal((await edit('owner', sent.id, 'я'.repeat(2001))).status, 400);
  assert.equal((await edit('owner', sent.id, 7)).status, 400);
  assert.equal((await edit('owner', 'missing', 'x')).status, 404);
  assert.equal((await edit('owner', '../x', 'x')).status, 400);
  const withPhoto = (await post('owner', 'caption', [photo.id])).body[0];
  const captionless = await edit('owner', withPhoto.id, '');
  assert.equal(captionless.status, 200); assert.equal(captionless.body[0].body, '');
  assert.deepEqual(captionless.body[0].attachments.map(row => row.id), [photo.id]);
  assert.equal(captionless.body[0].attachments[0].secret, undefined);
  const channel = await room(fixture, 'channel', ['alice']);
  await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: channel.id, target_user: 'alice', member_role: 'admin' });
  const announcement = (await fixture.call('alice', '/rest/v1/rpc/post_chat_message', { target_room: channel.id, message_body: 'news' })).body[0];
  await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: channel.id, target_user: 'alice', member_role: 'subscriber' });
  assert.equal((await edit('alice', announcement.id, 'still mine?')).status, 403, 'a demoted admin can no longer edit posts');
  assert.equal(fixture.repo.locks.size, 0);
});

test('authors and group staff delete messages for everyone, removing their encrypted photos', async () => {
  const fixture = setup(), group = await room(fixture, 'group', ['alice', 'bob']);
  const post = (user, body, ids = [], target = group.id) => fixture.call(user, '/rest/v1/rpc/post_chat_message', { target_room: target, message_body: body, attachment_ids: ids });
  const remove = (user, target) => fixture.call(user, '/rest/v1/rpc/delete_chat_message', { target_message: target });
  const list = user => fixture.call(user, '/rest/v1/chat_messages?room_id=eq.' + group.id, {}, 'GET');
  const { photo } = await upload(fixture, group.id);
  const own = (await post('owner', 'with photo', [photo.id])).body[0], fromAlice = (await post('alice', 'from alice')).body[0];
  assert.equal((await remove('bob', fromAlice.id)).status, 403, 'members cannot delete each other');
  assert.equal((await remove('eve', fromAlice.id)).status, 403, 'outsiders cannot delete');
  const result = await remove('owner', own.id);
  assert.equal(result.status, 200); assert.equal(result.body, true);
  assert.deepEqual((await list('alice')).body.map(row => row.id), [fromAlice.id]);
  assert.equal(await fixture.repo.get('attachments', photo.id), null);
  assert.equal(fixture.repo.files.has(photo.object_key), false); assert.equal(fixture.repo.files.has(photo.thumbnail_key), false);
  assert.equal((await fixture.call('alice', '/storage/v1/object/authenticated/chat-photos/' + photo.object_key, {}, 'GET')).status, 404);
  assert.equal((await remove('owner', own.id)).status, 404, 'deleting twice reports not found');
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/edit_chat_message', { target_message: own.id, message_body: 'x' })).status, 404);
  await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: group.id, target_user: 'bob', member_role: 'admin' });
  assert.equal((await remove('bob', fromAlice.id)).status, 200, 'admins moderate members');
  const fromBob = (await post('bob', 'from bob')).body[0];
  assert.equal((await remove('bob', fromBob.id)).status, 200, 'authors delete their own');
  const fromAliceAgain = (await post('alice', 'leaving soon')).body[0];
  await fixture.call('owner', '/rest/v1/rpc/remove_chat_member', { target_room: group.id, target_user: 'alice' });
  assert.equal((await remove('alice', fromAliceAgain.id)).status, 403, 'former members cannot delete');
  const direct = await room(fixture, 'direct', ['alice']);
  const hello = (await post('alice', 'hi', [], direct.id)).body[0];
  assert.equal((await remove('owner', hello.id)).status, 403, 'direct chats have no moderators');
  assert.equal((await remove('alice', hello.id)).status, 200);
  assert.equal(fixture.repo.locks.size, 0);
});
