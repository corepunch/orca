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
    const cursor = queries.find(query => query.method === 'cursorAfter')?.values?.[0];
    if (cursor) rows = rows.slice(rows.findIndex(row => row.id === cursor) + 1);
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
function setup(serviceOptions = {}) {
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
  const service = new ChatService(repo, auth, { now: () => clock.now, ...serviceOptions });
  const call = (jwt, path, body = {}, method = 'POST', extra = {}) => execute({ method, path, body, ...extra }, service, { jwt, userId: jwt });
  return { repo, auth, service, clock, calls, users, call };
}
async function room(fixture, kind = 'group', others = ['alice'], visibility = 'private') {
  const response = await fixture.call('owner', '/rest/v1/rpc/create_chat_room', { room_name: 'Conversation', room_kind: kind, room_visibility: visibility, member_ids: others });
  assert.equal(response.status, 200); return response.body[0];
}

const pushCalls = fixture => fixture.calls.filter(call => call.path === '/messaging/messages/push');

test('notification previews default on, validate booleans, persist privately and survive profile synchronization', async () => {
  const fixture = setup();
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/get_chat_profile')).body.notification_previews, true);
  delete fixture.repo.table('profiles').get('alice').notification_previews;
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/get_chat_profile')).body.notification_previews, true);
  assert.equal((await fixture.repo.get('profiles', 'alice')).notification_previews, undefined, 'legacy default needs no write');
  for (const value of [null, 0, 1, 'false', {}, []]) {
    const response = await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { notification_previews: value } });
    assert.equal(response.status, 400);
  }
  const response = await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { notification_previews: false, discoverable_email: true, username: 'alice' } });
  assert.equal(response.status, 200); assert.equal(response.body.notification_previews, false);
  fixture.users.get('alice').emailVerification = false;
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/get_chat_profile')).body.notification_previews, false);
  const discovery = await fixture.call('owner', '/rest/v1/rpc/find_chat_people', { query: 'alice' });
  assert.equal(discovery.body[0].notification_previews, undefined);
  await room(fixture);
  const contacts = await fixture.call('owner', '/rest/v1/chat_profiles', {}, 'GET');
  assert.equal(contacts.body[0].notification_previews, undefined);
  assert.equal((await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { notification_previews: true } })).body.notification_previews, true);
});

test('push previews use saved sender and room labels, sanitized text and legacy recipient defaults', async () => {
  for (const kind of ['direct', 'group', 'channel']) {
    const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, kind);
    await fixture.call('alice', '/rest/v1/rpc/get_chat_profile');
    delete fixture.repo.table('profiles').get('alice').notification_previews;
    await fixture.call('owner', '/rest/v1/rpc/update_chat_profile', { values: { display_name: 'Sender Name' } });
    const response = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', {
      target_room: conversation.id, message_body: ' Hello\n\tworld\u0000!\u202e ', sender_name: 'Spoof', room_name: 'Spoof',
    });
    assert.equal(response.status, 200);
    const payload = pushCalls(fixture)[0].options.body;
    assert.equal(payload.title, kind === 'direct' ? 'Sender Name' : 'Conversation');
    assert.equal(payload.body, kind === 'group' ? 'Sender Name: Hello world !' : 'Hello world !');
    assert.equal(payload.image, undefined);
    if (kind === 'channel') assert.ok(!JSON.stringify(payload).includes('Sender Name'), 'channel push never reveals the real author');
  }
});

test('push previews respect each recipient preference without leaking text or identity through data', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, 'group', ['alice', 'bob']);
  await fixture.call('alice', '/rest/v1/rpc/update_chat_profile', { values: { notification_previews: false } });
  await fixture.call('bob', '/rest/v1/rpc/get_chat_profile');
  await fixture.call('owner', '/rest/v1/rpc/update_chat_profile', { values: { notification_previews: false } });
  await fixture.call('owner', '/rest/v1/rpc/post_chat_message', {
    target_room: conversation.id, message_body: 'Secret content', notification_previews: true,
  });
  const alice = pushCalls(fixture).find(call => call.options.body.users[0] === 'alice').options.body;
  const bob = pushCalls(fixture).find(call => call.options.body.users[0] === 'bob').options.body;
  assert.equal(alice.title, 'ORCA Chat'); assert.equal(alice.body, 'New message');
  assert.deepEqual(Object.keys(alice.data).sort(), ['conversationId', 'messageId', 'userId']);
  assert.ok(!JSON.stringify(alice).includes('Secret content')); assert.ok(!JSON.stringify(alice).includes('owner'));
  assert.equal(bob.title, 'Conversation'); assert.equal(bob.body, 'owner: Secret content');
});

test('push previews describe single photos and albums while captions take priority', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, 'direct');
  await fixture.call('alice', '/rest/v1/rpc/get_chat_profile');
  for (const count of [1, 3]) {
    const ids = [];
    for (let index = 0; index < count; index++) ids.push((await upload(fixture, conversation.id)).photo.id);
    assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, attachment_ids: ids })).status, 200);
    assert.equal(pushCalls(fixture).at(-1).options.body.body, count === 1 ? 'Sent a photo' : 'Sent 3 photos');
  }
  const { photo } = await upload(fixture, conversation.id);
  await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, attachment_ids: [photo.id], message_body: 'Caption' });
  assert.equal(pushCalls(fixture).at(-1).options.body.body, 'Caption');
});

test('push preview bounds retain valid Unicode and sender fallbacks', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, 'direct');
  await fixture.call('alice', '/rest/v1/rpc/get_chat_profile');
  await fixture.call('owner', '/rest/v1/rpc/update_chat_profile', { values: { display_name: '😀'.repeat(128) } });
  await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: '😀'.repeat(1000) });
  const payload = pushCalls(fixture).at(-1).options.body;
  assert.equal(payload.title, '😀'.repeat(119) + '…'); assert.equal(payload.body, '😀'.repeat(239) + '…');
  assert.ok(Buffer.byteLength(JSON.stringify(payload)) < 3000);
  await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: '\\"'.repeat(2000) });
  assert.ok(Buffer.byteLength(JSON.stringify(pushCalls(fixture).at(-1).options.body)) < 3000, 'JSON escaping stays within the payload budget');
  const sender = fixture.repo.table('profiles').get('owner');
  sender.display_name = '\u0000'; sender.username = 'account_name';
  await fixture.service.notifyMessage({ id: 'test-fallback', room_id: conversation.id, sender_id: 'owner', body: 'Text' });
  assert.equal(pushCalls(fixture).at(-1).options.body.title, 'account_name');
  sender.username = null;
  await fixture.service.notifyMessage({ id: 'test-member', room_id: conversation.id, sender_id: 'owner', body: 'Text' });
  assert.equal(pushCalls(fixture).at(-1).options.body.title, 'Member');
});

test('push preview metadata failures fall back to generic alerts and use the shared deadline', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, 'direct');
  await fixture.call('alice', '/rest/v1/rpc/get_chat_profile');
  const get = fixture.repo.get.bind(fixture.repo); let fail;
  const signals = new Set();
  fixture.repo.get = async (name, id, options) => {
    if (options?.signal) {
      signals.add(options.signal);
      if (name + ':' + id === fail) throw new Failure(503, 'offline', 'Private content');
    }
    return get(name, id);
  };
  for (fail of ['rooms:' + conversation.id, 'profiles:owner', 'profiles:alice']) {
    const response = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Still committed' });
    assert.equal(response.status, 200);
    assert.equal(pushCalls(fixture).at(-1).options.body.title, 'ORCA Chat');
    assert.equal(pushCalls(fixture).at(-1).options.body.body, 'New message');
  }
  assert.equal(signals.size, 3, 'each send has one shared deadline for metadata and delivery');
});

test('push is disabled by default and rejects client-selected recipients', async () => {
  const fixture = setup(), conversation = await room(fixture);
  const result = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', {
    target_room: conversation.id, message_body: 'Private text', users: ['eve'], pushEnabled: true,
  });
  assert.equal(result.status, 200); assert.equal(pushCalls(fixture).length, 0);
});

test('push queues generic per-recipient notifications only after text and photo publication', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture, 'group', ['alice', 'bob']);
  const request = fixture.auth.rest.request.bind(fixture.auth.rest);
  fixture.auth.rest.request = async (method, path, options) => {
    if (path === '/messaging/messages/push') {
      assert.ok(await fixture.repo.get('messages', options.body.data.messageId));
      assert.ok(fixture.repo.locks.has('room:' + conversation.id));
    }
    return request(method, path, options);
  };
  const post = body => fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, ...body });
  const first = await post({ message_body: 'Private text', users: ['eve'] });
  assert.equal(first.status, 200);
  const deliveries = pushCalls(fixture);
  assert.deepEqual(deliveries.map(call => call.options.body.users[0]).sort(), ['alice', 'bob']);
  for (const call of deliveries) {
    const payload = call.options.body;
    assert.equal(call.method, 'POST'); assert.equal(payload.title, 'ORCA Chat'); assert.equal(payload.body, 'New message');
    assert.equal(payload.sound, 'default'); assert.equal(payload.draft, false);
    assert.match(payload.messageId, /^[a-f0-9]{36}$/);
    assert.deepEqual(payload.data, { conversationId: conversation.id, messageId: first.body[0].id, userId: payload.users[0] });
    assert.ok(await fixture.repo.get('messages', payload.data.messageId));
  }
  const { photo } = await upload(fixture, conversation.id);
  const next = await post({ attachment_ids: [photo.id] });
  assert.equal(next.status, 200); assert.equal(pushCalls(fixture).length, 4);
  assert.equal(pushCalls(fixture).at(-1).options.body.data.messageId, next.body[0].id);
  await fixture.call('owner', '/rest/v1/rpc/edit_chat_message', { target_message: first.body[0].id, message_body: 'Changed text' });
  await fixture.call('owner', '/rest/v1/rpc/delete_chat_message', { target_message: first.body[0].id });
  assert.equal(pushCalls(fixture).length, 4, 'edits and deletion do not notify');
});

test('push never queues failed publication or unauthorized posts and enumerates the complete member list', async () => {
  const fixture = setup({ pushEnabled: true }), conversation = await room(fixture);
  for (let index = 0; index < 125; index++) await fixture.repo.create('members', {
    room_id: conversation.id, user_id: 'recipient-' + index, role: 'member', created_at: fixture.service.timestamp(),
  });
  await fixture.call('eve', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Rejected' });
  assert.equal(pushCalls(fixture).length, 0);
  const create = fixture.repo.create.bind(fixture.repo);
  fixture.repo.create = async (name, ...args) => { if (name === 'messages') throw new Error('Failed commit'); return create(name, ...args); };
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Rejected' })).status, 503);
  assert.equal(pushCalls(fixture).length, 0);
  fixture.repo.create = create;
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Published' })).status, 200);
  assert.equal(pushCalls(fixture).length, 126);
  assert.equal(new Set(pushCalls(fixture).map(call => call.options.body.users[0])).size, 126);
});

test('push failures preserve committed sends and replay uses stable delivery IDs with current membership', async () => {
  const reports = [], fixture = setup({ pushEnabled: true, pushError: message => reports.push(message) });
  const conversation = await room(fixture, 'group', ['alice', 'bob']), request = fixture.auth.rest.request.bind(fixture.auth.rest);
  const accepted = new Set(); let failAlice = true;
  fixture.auth.rest.request = async (method, path, options) => {
    if (path !== '/messaging/messages/push') return request(method, path, options);
    if (options.body.users[0] === 'alice' && failAlice) throw new Failure(503, 'unavailable', 'Secret upstream body and token');
    if (accepted.has(options.body.messageId)) throw new Failure(409, 'message_already_exists', 'Exists');
    accepted.add(options.body.messageId); return request(method, path, options);
  };
  const result = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Private text' });
  assert.equal(result.status, 200); assert.ok(await fixture.repo.get('messages', result.body[0].id));
  assert.deepEqual(reports, ['Chat push could not be queued (HTTP 503).']);
  const message = await fixture.repo.get('messages', result.body[0].id), firstId = pushCalls(fixture)[0].options.body.messageId;
  failAlice = false;
  await fixture.repo.lock('room:' + conversation.id, () => fixture.service.notifyMessage(message));
  assert.equal(accepted.size, 2); assert.equal(pushCalls(fixture).length, 2);
  assert.equal(pushCalls(fixture)[0].options.body.messageId, firstId);
  await fixture.call('owner', '/rest/v1/rpc/remove_chat_member', { target_room: conversation.id, target_user: 'alice' });
  const requests = [];
  fixture.auth.rest.request = async (method, path, options) => {
    if (path === '/messaging/messages/push') requests.push(options.body.users[0]);
    return request(method, path, options);
  };
  await fixture.repo.lock('room:' + conversation.id, () => fixture.service.notifyMessage(message));
  assert.deepEqual(requests, ['bob']);
});

test('push shares one deadline across bounded fan-out and swallows lookup or diagnostic failures', async () => {
  const fixture = setup({ pushEnabled: true, pushTimeoutMs: 20, pushError: () => { throw new Error('Logger failed'); } });
  const conversation = await room(fixture), request = fixture.auth.rest.request.bind(fixture.auth.rest);
  let active = 0, maximum = 0, signals = new Set();
  for (let index = 0; index < 12; index++) await fixture.repo.create('members', { room_id: conversation.id, user_id: 'recipient-' + index });
  fixture.auth.rest.request = async (method, path, options) => {
    if (path !== '/messaging/messages/push') return request(method, path, options);
    signals.add(options.signal); active++; maximum = Math.max(maximum, active);
    try {
      await new Promise((resolve, reject) => {
        const timer = setTimeout(resolve, 100);
        options.signal.addEventListener('abort', () => { clearTimeout(timer); reject(options.signal.reason); }, { once: true });
      });
    } finally { active--; }
  };
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Saved' })).status, 200);
  assert.equal(maximum, 8); assert.equal(active, 0); assert.equal(signals.size, 1);
  const list = fixture.repo.list.bind(fixture.repo);
  fixture.repo.list = async (name, ...args) => { if (name === 'members') throw new Error('Directory unavailable'); return list(name, ...args); };
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: conversation.id, message_body: 'Also saved' })).status, 200);
});

test('push recipient pagination forwards its deadline through every REST page', async () => {
  const signal = AbortSignal.timeout(3000), requests = [], members = Array.from({ length: 137 }, (_, index) => ({
    $id: 'member-' + index, payload: JSON.stringify({ room_id: 'room', user_id: 'user-' + index }),
  }));
  const rest = new AppwriteRest({ endpoint: 'https://example.invalid/v1', projectId: 'project', apiKey: 'test-key',
    fetch: async (url, options) => {
      requests.push(url); assert.equal(options.signal, signal);
      const queries = new URL(url).searchParams.getAll('queries[]').map(JSON.parse);
      const cursor = queries.find(query => query.method === 'cursorAfter')?.values[0];
      const start = cursor ? members.findIndex(row => row.$id === cursor) + 1 : 0;
      return new Response(JSON.stringify({ documents: members.slice(start, start + 100) }));
    } });
  const rows = await new Repository(rest, 'database', 'bucket').list('members', [], undefined, { signal });
  assert.equal(rows.length, 137); assert.equal(requests.length, 2);
});

test('every protected request validates the platform JWT, including deleted or forged accounts', async () => {
  const fixture = setup();
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).body.id, 'owner');
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).status, 200);
  assert.equal(fixture.calls.filter(call => call.path === '/account').length, 2);
  assert.equal((await fixture.call('forged', '/rest/v1/chat_rooms', {}, 'GET')).status, 401);
  fixture.users.delete('owner'); assert.equal((await fixture.call('owner', '/rest/v1/rpc/get_chat_profile')).status, 401);
});

test('unchanged profiles need one read and no persistent lock or writes on protected requests', async () => {
  const fixture = setup();
  await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  const previous = await fixture.repo.get('profiles', 'owner');
  fixture.repo.table('profiles').set('owner', { id: previous.id, ...previous });
  const get = fixture.repo.get.bind(fixture.repo); let reads = 0;
  fixture.repo.get = async (name, id) => { reads++; return get(name, id); };
  for (const method of ['lock', 'create', 'update', 'remove']) fixture.repo[method] = async () => assert.fail('Unchanged profile must not ' + method);
  const response = await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  assert.equal(response.status, 200); assert.deepEqual(response.body, previous); assert.equal(reads, 1);
  assert.equal(fixture.calls.filter(call => call.path === '/account').length, 2);
});

test('profile identifier changes re-read under lock and preserve concurrent profile edits', async () => {
  const fixture = setup(), user = fixture.users.get('owner');
  user.phone = '+49123456789'; user.phoneVerification = true;
  await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  await fixture.call('owner', '/rest/v1/rpc/update_chat_profile', { values: { discoverable_email: true, discoverable_phone: true } });
  user.email = 'changed@example.invalid';
  const lock = fixture.repo.lock.bind(fixture.repo); let locked = 0;
  fixture.repo.lock = async (name, action) => {
    assert.equal(name, 'profile:owner'); locked++;
    const current = await fixture.repo.get('profiles', 'owner');
    await fixture.repo.update('profiles', 'owner', { ...current, display_name: 'Latest name', username: 'latest_name', discoverable_phone: false });
    return lock(name, action);
  };
  const response = await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  assert.equal(response.status, 200); assert.equal(locked, 1);
  assert.equal(response.body.display_name, 'Latest name'); assert.equal(response.body.username, 'latest_name');
  assert.equal(response.body.email, user.email); assert.equal(response.body.discoverable_email, false);
  assert.equal(response.body.phone, user.phone); assert.equal(response.body.discoverable_phone, false);
  fixture.repo.lock = lock; user.emailVerification = false; user.phoneVerification = false;
  const revoked = await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  assert.equal(revoked.body.email, null); assert.equal(revoked.body.phone, null);
  assert.equal(revoked.body.discoverable_email, false); assert.equal(revoked.body.discoverable_phone, false);
});

test('profile creation re-checks a concurrent creator before writing', async () => {
  const fixture = setup(), lock = fixture.repo.lock.bind(fixture.repo);
  fixture.repo.lock = async (name, action) => {
    await fixture.repo.create('profiles', { display_name: 'Created elsewhere', username: 'created_elsewhere',
      email: 'owner@example.invalid', phone: null, discoverable_email: true, discoverable_phone: false }, 'owner');
    return lock(name, action);
  };
  const response = await fixture.call('owner', '/rest/v1/rpc/get_chat_profile');
  assert.equal(response.status, 200); assert.equal(response.body.display_name, 'Created elsewhere');
  assert.equal(response.body.username, 'created_elsewhere'); assert.equal(response.body.discoverable_email, true);
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

test('contacts fetch each peer once across shared rooms without reloading the caller profile', async () => {
  const fixture = setup();
  await room(fixture, 'direct'); await room(fixture, 'group', ['alice', 'bob']);
  for (const user of ['alice', 'bob']) await fixture.call(user, '/rest/v1/rpc/get_chat_profile');
  const get = fixture.repo.get.bind(fixture.repo), list = fixture.repo.list.bind(fixture.repo), profileReads = [], memberLists = [];
  let activePeers = 0, peakPeers = 0;
  fixture.repo.get = async (name, id) => {
    if (name === 'profiles') profileReads.push(id);
    if (name !== 'profiles' || id === 'owner') return get(name, id);
    peakPeers = Math.max(peakPeers, ++activePeers);
    await new Promise(resolve => setImmediate(resolve));
    try { return await get(name, id); } finally { activePeers--; }
  };
  fixture.repo.list = async (name, queries, limit) => {
    if (name === 'members') memberLists.push(queries);
    return list(name, queries, limit);
  };
  const response = await fixture.call('owner', '/rest/v1/chat_profiles', {}, 'GET');
  assert.equal(response.status, 200); assert.deepEqual(response.body.map(value => value.id), ['alice', 'bob']);
  assert.deepEqual(profileReads.sort(), ['alice', 'bob', 'owner']); assert.equal(peakPeers, 2);
  assert.equal(memberLists.length, 3);
  assert.equal(memberLists.filter(queries => queries[0].attribute === 'user_id').length, 1);
  assert.equal(memberLists.filter(queries => queries[0].attribute === 'room_id').length, 2);
});

test('contacts skip uncommitted rooms and never expose their members', async () => {
  const fixture = setup();
  for (const user_id of ['owner', 'eve']) await fixture.repo.create('members', {
    room_id: 'unfinished', user_id, role: 'member', created_at: fixture.service.timestamp(),
  });
  const list = fixture.repo.list.bind(fixture.repo);
  fixture.repo.list = async (name, queries, limit) => {
    if (name === 'members') assert.equal(queries[0].attribute, 'user_id');
    return list(name, queries, limit);
  };
  const response = await fixture.call('owner', '/rest/v1/chat_profiles', {}, 'GET');
  assert.equal(response.status, 200); assert.deepEqual(response.body, []);
});

test('contacts recheck membership before exposing a room after removal', async () => {
  const fixture = setup(), conversation = await room(fixture), get = fixture.repo.get.bind(fixture.repo);
  fixture.repo.get = async (name, id) => {
    if (name === 'rooms' && id === conversation.id) {
      const own = [...fixture.repo.table('members').values()].find(row => row.room_id === id && row.user_id === 'owner');
      fixture.repo.table('members').delete(own.id);
    }
    return get(name, id);
  };
  assert.equal((await fixture.call('owner', '/rest/v1/chat_profiles', {}, 'GET')).status, 403);
});

test('contacts use current channel roles and hide other subscribers after demotion', async () => {
  const fixture = setup(), channel = await room(fixture, 'channel', ['alice', 'bob']);
  await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: channel.id, target_user: 'alice', member_role: 'admin' });
  const get = fixture.repo.get.bind(fixture.repo), profiles = [];
  fixture.repo.get = async (name, id) => {
    if (name === 'profiles') profiles.push(id);
    if (name === 'rooms' && id === channel.id) {
      const own = [...fixture.repo.table('members').values()].find(row => row.room_id === id && row.user_id === 'alice');
      own.role = 'subscriber';
    }
    return get(name, id);
  };
  const response = await fixture.call('alice', '/rest/v1/chat_profiles', {}, 'GET');
  assert.equal(response.status, 200); assert.deepEqual(response.body.map(value => value.id), ['owner']);
  assert.ok(!profiles.includes('bob'));
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

test('group/channel leave revokes access and rejoining does not restore staff rights', async () => {
  for (const kind of ['group', 'channel']) {
    const fixture = setup(), conversation = await room(fixture, kind, ['alice', 'bob']);
    const body = { target_room: conversation.id };
    await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { ...body, target_user: 'alice', member_role: 'admin' });
    const invite = (await fixture.call('owner', '/rest/v1/rpc/create_chat_invite', body)).body;
    for (const user of ['owner', 'eve']) assert.equal((await fixture.call(user, '/rest/v1/rpc/leave_chat_room', body)).status, 403);
    for (const user of ['alice', 'bob']) {
      assert.deepEqual(await fixture.call(user, '/rest/v1/rpc/leave_chat_room', body), { status: 200, body: true });
      assert.ok(!(await fixture.call(user, '/rest/v1/chat_rooms', {}, 'GET')).body.some(row => row.id === conversation.id));
      assert.ok(!(await fixture.call('owner', '/rest/v1/rpc/list_chat_members', body)).body.some(row => row.user_id === user));
      assert.equal((await fixture.call(user, '/rest/v1/chat_messages?room_id=eq.' + conversation.id, {}, 'GET')).status, 403);
      assert.equal((await fixture.call(user, '/rest/v1/rpc/post_chat_message', { ...body, message_body: 'Gone' })).status, 403);
      assert.equal((await fixture.call(user, '/rest/v1/rpc/leave_chat_room', body)).status, 403);
      const joined = await fixture.call(user, '/rest/v1/rpc/join_chat_invite', { invite_code: invite.code });
      assert.equal(joined.status, 200); assert.equal(joined.body.role, kind === 'channel' ? 'subscriber' : 'member');
    }
  }
  const fixture = setup(), direct = await room(fixture, 'direct');
  for (const user of ['owner', 'alice']) assert.equal((await fixture.call(user, '/rest/v1/rpc/leave_chat_room', { target_room: direct.id })).status, 403);
});

test('only owners rename groups/channels and names persist without changing membership or room metadata', async () => {
  for (const kind of ['group', 'channel']) {
    const fixture = setup(), conversation = await room(fixture, kind, ['alice', 'bob']);
    const rename = (user, name, extra = {}) => fixture.call(user, '/rest/v1/rpc/update_chat_room', { target_room: conversation.id, room_name: name, ...extra });
    await fixture.call('owner', '/rest/v1/rpc/set_chat_member_role', { target_room: conversation.id, target_user: 'alice', member_role: 'admin' });
    for (const user of ['alice', 'bob', 'eve']) assert.equal((await rename(user, 'Forbidden')).status, 403);
    for (const name of ['', '   ', 'я'.repeat(81), null, 123]) assert.equal((await rename('owner', name)).status, 400);
    assert.equal((await rename('owner', 'Name', { description: 'Unsupported' })).status, 400);
    assert.equal((await rename('owner', 'Name', { room_kind: 'direct' })).status, 400);
    const name = 'я'.repeat(80), renamed = await rename('owner', name);
    assert.equal(renamed.status, 200);
    assert.deepEqual(renamed.body, { ...conversation, name });
    for (const user of ['owner', 'alice', 'bob']) assert.equal((await fixture.call(user, '/rest/v1/chat_rooms', {}, 'GET')).body.find(row => row.id === conversation.id).name, name);
    assert.equal((await rename('owner', '  Trimmed  ')).body.name, 'Trimmed');
  }
  const fixture = setup(), direct = await room(fixture, 'direct');
  assert.equal((await fixture.call('owner', '/rest/v1/rpc/update_chat_room', { target_room: direct.id, room_name: 'Changed' })).status, 403);
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

test('nine-photo messages preserve selection order through history and editing and delete every photo', async () => {
  const fixture = setup(), group = await room(fixture), photos = [];
  for (let index = 0; index < 9; index++) photos.push((await upload(fixture, group.id)).photo);
  const ordered = [...photos].reverse(), attachmentIds = ordered.map(photo => photo.id);
  const sent = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: attachmentIds });
  assert.equal(sent.status, 200);
  const message = sent.body[0];
  assert.deepEqual(message.attachments.map(photo => photo.id), attachmentIds);
  assert.equal(message.attachment_ids, undefined);
  for (const photo of message.attachments) {
    assert.equal(photo.status, 'attached'); assert.equal(photo.message_id, message.id); assert.equal(photo.secret, undefined);
  }
  const history = await fixture.service.listMessages(fixture.users.get('alice'), group.id);
  assert.deepEqual(history[0].attachments.map(photo => photo.id), attachmentIds);
  const edited = await fixture.call('owner', '/rest/v1/rpc/edit_chat_message', { target_message: message.id, message_body: 'Nine photos' });
  assert.equal(edited.status, 200); assert.deepEqual(edited.body[0].attachments.map(photo => photo.id), attachmentIds);
  const deleted = await fixture.call('owner', '/rest/v1/rpc/delete_chat_message', { target_message: message.id });
  assert.equal(deleted.status, 200); assert.equal(await fixture.repo.get('messages', message.id), null);
  assert.equal(fixture.repo.files.size, 0); assert.equal(fixture.repo.table('attachments').size, 0);
});

test('photo messages reject ten attachments and duplicates before changing reservations', async () => {
  const fixture = setup(), group = await room(fixture), photos = [];
  for (let index = 0; index < 10; index++) photos.push((await upload(fixture, group.id)).photo);
  const invalidLists = [photos.map(photo => photo.id), [...photos.slice(0, 8).map(photo => photo.id), photos[0].id]];
  for (const attachmentIds of invalidLists) {
    const response = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: attachmentIds });
    assert.equal(response.status, 400);
    assert.equal(fixture.repo.table('messages').size, 0);
    for (const photo of photos) assert.equal((await fixture.repo.get('attachments', photo.id)).message_id, undefined);
  }
  const valid = await fixture.call('owner', '/rest/v1/rpc/post_chat_message', { target_room: group.id, attachment_ids: photos.slice(0, 9).map(photo => photo.id) });
  assert.equal(valid.status, 200); assert.equal(valid.body[0].attachments.length, 9);
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


test('message page RPC returns bounded chronological pages while legacy GET keeps rows', async () => {
  const fixture = setup(), conversation = await room(fixture);
  for (let index = 0; index < 35; index++) {
    const messageId = 'message-' + String(index).padStart(3, '0');
    await fixture.repo.create('messages', { room_id: conversation.id, sender_id: 'owner', body: messageId,
      created_at: new Date(fixture.clock.now + index * 1000).toISOString(), attachment_ids: [] }, messageId);
  }
  const first = await fixture.call('owner', '/rest/v1/rpc/list_chat_messages', { target_room: conversation.id });
  assert.equal(first.status, 200); assert.equal(first.body.messages.length, 30); assert.equal(first.body.has_more, true);
  assert.equal(first.body.messages[0].id, 'message-005'); assert.equal(first.body.next_cursor, 'message-005');
  const older = await fixture.call('owner', '/rest/v1/rpc/list_chat_messages', { target_room: conversation.id, before: first.body.next_cursor });
  assert.equal(older.body.messages.length, 5); assert.equal(older.body.has_more, false); assert.equal(older.body.next_cursor, null);
  const legacy = await fixture.call('owner', '/rest/v1/chat_messages?room_id=eq.' + conversation.id + '&limit=2&before=message-005', {}, 'GET');
  assert.equal(legacy.status, 200); assert.deepEqual(legacy.body.map(row => row.id), ['message-004', 'message-003']);
});
