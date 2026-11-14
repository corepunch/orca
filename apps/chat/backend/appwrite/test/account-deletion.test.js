import test from 'node:test';
import assert from 'node:assert/strict';
import { AccountDeletion, deletionId, recoverDeletions } from '../src/account-deletion.js';
import { Auth } from '../src/auth.js';
import { Failure, Repository } from '../src/rest.js';
import { ChatService } from '../src/service.js';
import { execute } from '../src/main.js';

const copy = value => structuredClone(value);
function fixture(options = {}) {
  const tables = new Map(), files = new Map(), calls = [], users = new Map(), identities = [];
  const clock = { now: 1700000000000 };
  const table = name => { if (!tables.has(name)) tables.set(name, new Map()); return tables.get(name); };
  const raw = (name, id, values) => table(name).set(id, { $id: id, ...copy(values) });
  for (const name of ['alice', 'bob', 'carol']) users.set(name, { $id: name, status: true, name,
    email: name + '@example.invalid', emailVerification: true });
  const select = (rows, url) => {
    const queries = url.searchParams.getAll('queries[]').map(JSON.parse);
    for (const q of queries) if (q.method === 'equal') rows = rows.filter(row => q.values.includes(row[q.attribute]));
    rows.sort((a, b) => a.$id.localeCompare(b.$id));
    const cursor = queries.find(q => q.method === 'cursorAfter')?.values[0];
    if (cursor) {
      const index = rows.findIndex(row => row.$id === cursor);
      if (index < 0) throw new Failure(404, 'not_found', 'Deleted cursor.');
      rows = rows.slice(index + 1);
    }
    return rows.slice(0, queries.find(q => q.method === 'limit')?.values[0] || 25).map(copy);
  };
  const rest = { async request(method, path, { body, jwt, signal } = {}) {
    calls.push({ method, path });
    if (options.fail) await options.fail(method, path, body, signal);
    const url = new URL(path, 'https://appwrite.invalid'), parts = url.pathname.split('/').filter(Boolean);
    if (path === '/account') {
      const user = users.get(jwt);
      if (!user?.status) throw new Failure(401, 'unauthenticated', 'Sign in.');
      return copy(user);
    }
    if (parts[0] === 'users' && parts[1] === 'identities') return { identities: identities.filter(identity =>
      JSON.parse(url.searchParams.get('queries[]')).values.includes(identity.userId)).map(copy) };
    if (parts[0] === 'users') {
      if (!users.has(parts[1])) throw new Failure(404, 'not_found', 'Missing account.');
      if (method === 'PATCH' && parts[2] === 'status') { users.get(parts[1]).status = body.status; return copy(users.get(parts[1])); }
      if (method === 'DELETE') { users.delete(parts[1]); return true; }
      if (method === 'GET') return copy(users.get(parts[1]));
    }
    if (parts[0] === 'databases' && parts[4] === 'documents') {
      const rows = table(parts[3]), id = parts[5];
      if (method === 'GET' && !id) return { documents: select([...rows.values()], url) };
      if (method === 'GET') { if (!rows.has(id)) throw new Failure(404, 'not_found', 'Missing document.'); return copy(rows.get(id)); }
      if (method === 'POST') {
        if (rows.has(body.documentId)) throw new Failure(409, 'conflict', 'Already exists.');
        raw(parts[3], body.documentId, { ...body.data, $permissions: body.permissions }); return copy(rows.get(body.documentId));
      }
      if (method === 'PATCH') {
        if (!rows.has(id)) throw new Failure(404, 'not_found', 'Missing document.');
        raw(parts[3], id, { ...rows.get(id), ...body.data }); return copy(rows.get(id));
      }
      if (method === 'DELETE') { rows.delete(id); return true; }
    }
    if (parts[0] === 'storage' && parts[3] === 'files') {
      if (method === 'GET' && !parts[4]) return { files: select([...files.values()], url) };
      if (method === 'DELETE') { files.delete(parts[4]); return true; }
    }
    throw new Error('Unexpected test route: ' + method + ' ' + path);
  } };
  const repo = new Repository(rest, 'db', 'bucket'), auth = new Auth(rest, repo);
  const deletion = new AccountDeletion(repo, auth, { now: () => clock.now, ...options });
  const service = new ChatService(repo, auth); service.deletions = deletion;
  const call = (userId, body = { confirmation: 'DELETE' }, caller = userId) => execute({
    method: 'POST', path: '/rest/v1/rpc/delete_chat_account', body }, service, { jwt: userId, userId: caller });
  const seed = (name, id, values) => repo.create(name, values, id);
  const file = (id, userId) => files.set(id, { $id: id, $permissions: userId ? [`delete("user:${userId}")`] : [] });
  async function drain(userId = 'alice') {
    clock.now += 65001;
    for (let turn = 0; turn < 1000; turn++) {
      if (!await repo.get('deletions', deletionId(userId))) return;
      await deletion.run(deletionId(userId));
      clock.now += 65001;
    }
    assert.fail('Deletion did not converge.');
  }
  return { table, raw, repo, auth, deletion, service, seed, file, files, calls, users, identities, clock, call, drain };
}

test('account deletion requires caller identity and explicit confirmation, with durable acceptance before blocking', async () => {
  const f = fixture();
  for (const body of [{}, { confirmation: true }, { confirmation: 'DELETE', user_id: 'bob' }]) {
    assert.equal((await f.call('alice', body)).status, 400);
    assert.equal(f.table('account-deletions').size, 0); assert.equal(f.users.get('alice').status, true);
  }
  assert.equal((await f.call('alice', { confirmation: 'DELETE' }, 'bob')).status, 401);
  const accepted = await f.call('alice');
  assert.deepEqual(accepted, { status: 200, body: { accepted: true, deleted: false } });
  assert.ok(await f.repo.get('deletions', deletionId('alice')));
  assert.equal(f.users.get('alice').status, false); assert.equal(f.users.get('bob').status, true);
  const blocked = await execute({ method: 'POST', path: '/rest/v1/rpc/get_chat_profile', body: {} }, f.service, { jwt: 'alice', userId: 'alice' });
  assert.equal(blocked.status, 401);
  const create = f.calls.findIndex(call => call.method === 'POST' && call.path.includes('/account-deletions/'));
  const block = f.calls.findIndex(call => call.path === '/users/alice/status');
  assert.ok(create >= 0 && block > create);
});

test('account deletion removes own Chat and Octopus data and preserves peers and their files', async () => {
  const f = fixture();
  await f.seed('rooms', 'direct', { name: 'Private names', kind: 'direct', visibility: 'private', created_by: 'alice', direct_key: 'pair' });
  await f.seed('members', 'ma', { room_id: 'direct', user_id: 'alice', role: 'owner' });
  await f.seed('members', 'mb', { room_id: 'direct', user_id: 'bob', role: 'member' });
  for (let n = 0; n < 63; n++) await f.seed('messages', 'm' + String(n).padStart(3, '0'), {
    room_id: 'direct', created_at: new Date(n).toISOString(), sender_id: n % 2 ? 'bob' : 'alice', body: 'Content' });
  await f.seed('attachments', 'photo', { room_id: 'direct', uploader_id: 'alice', object_key: 'own/photo.jpg', thumbnail_key: 'own/thumbnail.jpg' });
  f.file(f.repo.fileId('own/photo.jpg')); f.file(f.repo.fileId('own/thumbnail.jpg'));
  await f.seed('invites', 'invite', { room_id: 'direct', created_by: 'alice', code: 'private-code' });
  await f.seed('state', 'challenge', { kind: 'link', user_id: 'alice', identifier: 'private@example.invalid' });
  await f.seed('state', 'peer-state', { kind: 'rate_claim', name: 'discovery:bob' });
  await f.seed('state', 'own-report-rate', { kind: 'rate_claim', name: 'reports:alice' });
  await f.seed('blocks', 'outgoing-block', { blocker_id: 'alice', blocked_id: 'bob' });
  await f.seed('blocks', 'incoming-block', { blocker_id: 'bob', blocked_id: 'alice' });
  await f.seed('blocks', 'unrelated-block', { blocker_id: 'bob', blocked_id: 'carol' });
  await f.seed('devices', 'own-device', { user_id: 'alice', device_id: 'a' });
  await f.seed('devices', 'peer-device', { user_id: 'bob', device_id: 'b' });
  await f.seed('reports', 'own-report', { reporter_id: 'alice', target_user: 'bob', status: 'open' });
  await f.seed('reports', 'target-report', { reporter_id: 'bob', target_user: 'alice', status: 'open' });
  await f.seed('reports', 'unrelated-report', { reporter_id: 'bob', target_user: 'carol', status: 'open' });
  f.raw('posts', 'own-post', { authorId: 'alice', imageId: 'peer-photo' });
  f.raw('posts', 'peer-post', { authorId: 'bob' });
  f.raw('post-feed', 'own-post', { authorId: 'alice' });
  f.raw('post-feed', 'orphan-feed', { authorId: 'alice' });
  f.raw('post-feed', 'peer-post', { authorId: 'bob' });
  f.raw('comments', 'own-comment', { authorId: 'alice', postId: 'peer-post' });
  f.raw('comments', 'child-comment', { authorId: 'bob', postId: 'own-post' });
  f.raw('comments', 'peer-comment', { authorId: 'bob', postId: 'peer-post' });
  for (const [id, values] of [['own-like', { userId: 'alice', targetId: 'peer-post', targetType: 'post' }],
    ['child-like', { userId: 'bob', targetId: 'own-comment', targetType: 'comment' }],
    ['post-like', { userId: 'bob', targetId: 'own-post', targetType: 'post' }],
    ['peer-like', { userId: 'bob', targetId: 'peer-post', targetType: 'post' }]]) f.raw('likes', id, values);
  f.raw('reposts', 'own-repost', { userId: 'alice', postId: 'peer-post' });
  f.raw('reposts', 'child-repost', { userId: 'bob', postId: 'own-post' });
  f.raw('follows', 'outgoing', { followerId: 'alice', followingId: 'bob' });
  f.raw('follows', 'incoming', { followerId: 'bob', followingId: 'alice' });
  f.raw('follows', 'unrelated', { followerId: 'bob', followingId: 'carol' });
  f.file('own-avatar', 'alice'); f.file('old-orphan-upload', 'alice'); f.file('peer-photo', 'bob');
  assert.equal((await f.call('alice')).status, 200);
  await f.drain();
  assert.equal(f.users.has('alice'), false); assert.equal(f.users.has('bob'), true);
  assert.equal((await f.repo.list('messages')).length, 31);
  assert.ok((await f.repo.list('messages')).every(row => row.sender_id === 'bob'));
  assert.equal((await f.repo.get('members', 'mb')).role, 'owner');
  assert.equal((await f.repo.get('rooms', 'direct')).name, 'Deleted account');
  for (const collection of ['attachments', 'invites', 'account-directory', 'profiles', 'account-deletions']) assert.equal(f.table(collection).size, 0);
  assert.deepEqual([...f.files.keys()], ['peer-photo'], 'owner-writable image ID cannot authorize deleting another user’s file');
  assert.deepEqual([...f.table('posts').keys()], ['peer-post']);
  assert.deepEqual([...f.table('comments').keys()], ['peer-comment']);
  assert.deepEqual([...f.table('likes').keys()], ['peer-like']);
  assert.deepEqual([...f.table('follows').keys()], ['unrelated']);
  assert.deepEqual([...f.table('post-feed').keys()], ['peer-post']);
  assert.deepEqual([...f.table('service-state').keys()], ['peer-state']);
  assert.deepEqual([...f.table('user-blocks').keys()], ['unrelated-block']);
  assert.deepEqual([...f.table('moderation-reports').keys()], ['unrelated-report']);
  assert.deepEqual([...f.table('device-keys').keys()], ['peer-device']);
  const authDelete = f.calls.findIndex(call => call.method === 'DELETE' && call.path === '/users/alice');
  assert.ok(authDelete > 0);
  assert.deepEqual(f.calls.slice(authDelete + 1).filter(call => call.method !== 'GET' && !call.path.includes('/service-state/')).map(call => call.path),
    ['/databases/db/collections/account-deletions/documents/' + deletionId('alice')], 'only the private job is removed after Auth');
});

test('account deletion transfers group ownership to staff and reclaims only abandoned rooms', async () => {
  const f = fixture();
  for (const roomId of ['group', 'empty']) {
    await f.seed('rooms', roomId, { name: roomId, kind: 'group', visibility: 'private', created_by: 'alice' });
    await f.seed('members', 'alice-' + roomId, { room_id: roomId, user_id: 'alice', role: 'owner' });
  }
  await f.seed('members', 'bob-group', { room_id: 'group', user_id: 'bob', role: 'member' });
  await f.seed('members', 'carol-group', { room_id: 'group', user_id: 'carol', role: 'admin' });
  await f.seed('messages', 'peer-history', { room_id: 'group', sender_id: 'bob', body: 'Keep this', created_at: new Date(0).toISOString() });
  await f.seed('messages', 'abandoned-history', { room_id: 'empty', sender_id: 'bob', body: 'Nobody remains', created_at: new Date(0).toISOString() });
  await f.call('alice'); await f.drain();
  assert.equal((await f.repo.get('members', 'carol-group')).role, 'owner');
  assert.equal((await f.repo.get('members', 'bob-group')).role, 'member');
  assert.ok(await f.repo.get('messages', 'peer-history'));
  assert.equal(await f.repo.get('rooms', 'empty'), null);
  assert.equal(await f.repo.get('messages', 'abandoned-history'), null);
});

test('account deletion keeps recoverable metadata and Auth on storage failure, then resumes idempotently', async () => {
  let fail = true;
  const f = fixture({ fail: (method, path) => {
    if (fail && method === 'DELETE' && path.endsWith('/' + f.repo.fileId('own/thumbnail.jpg'))) throw new Failure(503, 'temporary', 'Retry.');
  } });
  await f.seed('attachments', 'photo', { room_id: 'direct', uploader_id: 'alice', object_key: 'own/photo.jpg', thumbnail_key: 'own/thumbnail.jpg' });
  f.file(f.repo.fileId('own/photo.jpg')); f.file(f.repo.fileId('own/thumbnail.jpg'));
  await f.call('alice'); await f.deletion.run(deletionId('alice'), { maximumSteps: 1 }); f.clock.now += 65001;
  await assert.rejects(() => f.deletion.run(deletionId('alice')), error => error.status === 503);
  assert.ok(await f.repo.get('attachments', 'photo')); assert.ok(await f.repo.get('deletions', deletionId('alice')));
  assert.equal(f.users.get('alice').status, false); assert.equal(f.files.size, 1);
  fail = false; await f.drain();
  assert.equal(f.files.size, 0); assert.equal(f.users.has('alice'), false);
  assert.deepEqual(await f.deletion.run(deletionId('alice')), { processed: false });
});

test('account deletion waits for in-flight writers and recovers when a retained scan cursor was deleted', async () => {
  const f = fixture();
  for (let n = 0; n < 30; n++) await f.seed('messages', 'm' + n.toString().padStart(2, '0'), {
    room_id: 'room', sender_id: 'bob', body: 'Keep', created_at: new Date(n).toISOString() });
  await f.call('alice');
  await f.deletion.run(deletionId('alice'), { maximumSteps: 1 });
  const count = f.calls.filter(call => !call.path.includes('/service-state/')).length;
  await f.deletion.run(deletionId('alice'));
  assert.equal(f.calls.filter(call => !call.path.includes('/service-state/')).length, count + 1, 'grace period reads job but performs no cleanup');
  f.clock.now += 65001;
  await f.deletion.run(deletionId('alice'), { maximumSteps: 2 });
  const job = await f.repo.get('deletions', deletionId('alice'));
  assert.equal(job.phase, 'messages'); assert.ok(job.cursor);
  await f.repo.remove('messages', job.cursor);
  await f.deletion.run(deletionId('alice'), { maximumSteps: 1 });
  assert.equal((await f.repo.get('deletions', job.id)).cursor, null);
  await f.drain(); assert.equal(f.users.has('alice'), false);
});

test('account deletion revokes Sign in with Apple tokens before Auth removal and fails closed without configuration', async () => {
  const f = fixture();
  f.identities.push({ provider: 'apple', userId: 'alice', providerRefreshToken: 'test-token' });
  const denied = await f.call('alice');
  assert.equal(denied.status, 503); assert.equal(f.users.get('alice').status, true); assert.equal(f.table('account-deletions').size, 0);
  const revocations = [];
  const g = fixture({ appleClientId: 'test.client', appleClientSecret: 'test-secret', fetch: async (url, options) => {
    revocations.push({ url, options }); return { ok: true };
  } });
  g.identities.push({ provider: 'apple', userId: 'alice', providerRefreshToken: 'test-token' });
  await g.call('alice'); await g.drain();
  assert.equal(revocations.length, 1); assert.equal(revocations[0].url, 'https://appleid.apple.com/auth/revoke');
  assert.equal(revocations[0].options.body.get('token_type_hint'), 'refresh_token');
  assert.equal(g.users.has('alice'), false);
});

test('account deletion serializes duplicate event and recovery workers', async () => {
  const f = fixture(); await f.call('alice');
  let entered, release;
  const started = new Promise(resolve => { entered = resolve; }), held = new Promise(resolve => { release = resolve; });
  const step = f.deletion.step;
  f.deletion.step = async function(job) { entered(); await held; return step.call(this, job); };
  const first = f.deletion.run(deletionId('alice'), { maximumSteps: 1 });
  await started;
  assert.deepEqual(await f.deletion.run(deletionId('alice')), { processed: false, busy: true });
  release(); await first;
  assert.equal((await f.repo.get('deletions', deletionId('alice'))).phase, 'apple');
  assert.equal(f.table('service-state').size, 0);
});

test('account deletion work deadline releases locks with fresh signals and permits retry', async () => {
  let fail = false;
  const f = fixture({ fail: async (method, path, body, signal) => {
    if (fail && method === 'GET' && path.includes('/collections/messages/documents')) await new Promise((resolve, reject) => {
      const hold = setTimeout(resolve, 1000);
      signal.addEventListener('abort', () => { clearTimeout(hold); reject(signal.reason); }, { once: true });
    });
    if (signal && method === 'DELETE' && path.includes('/service-state/documents/')) assert.equal(signal.aborted, false);
  } });
  await f.call('alice');
  const job = await f.repo.get('deletions', deletionId('alice'));
  await f.repo.update('deletions', job.id, { ...job, phase: 'messages' });
  fail = true;
  await assert.rejects(() => f.deletion.run(job.id, { milliseconds: 15 }), error => error.name === 'TimeoutError');
  assert.equal(f.table('service-state').size, 0, 'work cancellation cannot strand its job lock');
  assert.ok(await f.repo.get('deletions', job.id)); assert.ok(f.users.has('alice'));
  fail = false; await f.drain(); assert.equal(f.users.has('alice'), false);
});

test('account deletion recovery rotates past failed jobs and checkpoints before starting work', async () => {
  const f = fixture(), attempted = [];
  for (let i = 0; i < 12; i++) await f.seed('deletions', 'j' + i.toString().padStart(2, '0'), { kind: 'account_deletion' });
  const worker = { async run(id) {
    assert.ok((await f.repo.get('state', 'account-deletion-sweep')).cursor);
    attempted.push(id); throw new Failure(503, 'temporary', 'Retry.');
  } };
  for (let n = 0; n < 3; n++) await recoverDeletions(f.repo, worker);
  assert.deepEqual(attempted, Array.from({ length: 12 }, (_, i) => 'j' + i.toString().padStart(2, '0')));
});
