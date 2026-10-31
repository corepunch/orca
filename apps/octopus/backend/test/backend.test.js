import test from 'node:test';
import assert from 'node:assert/strict';
import { readFile } from 'node:fs/promises';
import { createRepository } from '../src/repository.js';
import { execute, feedDocument, publiclyReadable, refreshPost, reconcileBatch } from '../src/materialize.js';
import { compatibleAttribute, coversIndex, migrate, feedAttributes, imageAttributes, albumAttributes, indexes } from '../src/migrate.js';
import { albumFields } from '../src/albums.js';
import { backfillAlbums } from '../src/backfill-albums.js';

const now = '2026-10-28T10:00:00.000Z';
const post = (id = 'p1', overrides = {}) => ({ $id: id, $createdAt: now, $updatedAt: now,
  $permissions: ['read("any")'], content: 'Long post', title: 'Title', authorId: 'alice', authorName: 'Alice', ...overrides });

function fixture(documents = [post()]) {
  const collections = { posts: new Map(documents.map(doc => [doc.$id, doc])), feed: new Map(),
    profiles: new Map([['alice', { avatarId: 'avatar' }]]) };
  const counts = { likes: 1201, comments: 3000, reposts: 2000 };
  const calls = [];
  let locked = false, cursor = { phase: 'posts', cursor: null };
  const repo = {
    config: { databaseId: 'octopus-db', feedId: 'feed', controlId: 'control', batchSize: 2 },
    collections, counts, calls,
    async get(collection, id, privileged = true) { calls.push(['get', collection, privileged]); return collections[collection]?.get(id) || null; },
    async collection() { return { enabled: true, documentSecurity: true, $permissions: [] }; },
    async countPublic(collection, filters) { calls.push(['count', collection, filters]); return counts[collection]; },
    async put(collection, id, data, permissions) { collections[collection].set(id, { ...data, $id: id, $permissions: permissions }); },
    async remove(collection, id) { collections[collection].delete(id); },
    async acquire() { if (locked) return false; locked = true; return true; },
    async release() { locked = false; },
    async checkpoint(value) { cursor = structuredClone(value); },
    async loadCheckpoint() { return structuredClone(cursor); },
    async list(collection, filters) {
      const queries = filters.map(value => JSON.parse(value));
      const after = queries.find(q => q.method === 'cursorAfter')?.values[0];
      const limit = queries.find(q => q.method === 'limit').values[0];
      return { documents: [...collections[collection].values()].sort((a, b) => a.$id.localeCompare(b.$id))
        .filter(doc => !after || doc.$id > after).slice(0, limit) };
    },
  };
  return repo;
}

const event = (collection, action = 'create') => `databases.octopus-db.collections.${collection}.documents.any.${action}`;

test('albums preserve nine ordered images, legacy first image and paired unknown dimensions', () => {
  assert.deepEqual(albumFields({ imageId: 'old', imageWidth: 600, imageHeight: 400 }),
    { imageIds: ['old'], imageWidths: [600], imageHeights: [400] });
  const imageIds = Array.from({ length: 9 }, (_, i) => 'photo-' + i);
  const source = { imageIds, imageWidths: [800, 500], imageHeights: [600] };
  const album = albumFields(source);
  assert.deepEqual(album.imageIds, imageIds);
  assert.deepEqual(album.imageWidths, [800, 0, 0, 0, 0, 0, 0, 0, 0]);
  assert.deepEqual(album.imageHeights, [600, 0, 0, 0, 0, 0, 0, 0, 0]);
  const summary = feedDocument(post('p1', source), '', {}, now);
  assert.deepEqual(summary.imageIds, imageIds);
  assert.equal(summary.imageId, 'photo-0');
  assert.equal(summary.imageWidth, 800);
  assert.deepEqual(albumFields({ imageIds: ['a', 'a', '', 'b'], imageWidths: [1, 2, 3, 4], imageHeights: [1, 2, 3, 4] }),
    { imageIds: ['a', 'b'], imageWidths: [1, 4], imageHeights: [1, 4] });
});

test('album backfill pages, rereads replacements, preserves albums and patches only new columns idempotently', async () => {
  const repo = fixture([post('a', { imageId: 'old', imageWidth: 600, imageHeight: 400 }),
    post('b', { imageIds: ['one', 'two'], imageWidths: [1, 2], imageHeights: [1, 2] }), post('c')]);
  repo.collections.feed.set('a', post('a', { imageId: 'old', imageWidth: 600, imageHeight: 400 }));
  const patches = [];
  let disabled = false;
  repo.collectionPath = collection => '/collections/' + collection;
  repo.request = async (method, path, body) => {
    if (method === 'PUT') { assert.equal(path, '/collections/posts'); disabled = !body.enabled; return; }
    assert.equal(method, 'PATCH');
    assert.equal(disabled, true, 'direct source writes must be frozen during migration');
    assert.deepEqual(Object.keys(body), ['data']);
    assert.deepEqual(Object.keys(body.data).sort(), ['imageHeights', 'imageIds', 'imageWidths']);
    patches.push(path);
    const [, collection, id] = path.match(/\/collections\/(.+)\/documents\/(.+)$/);
    Object.assign(repo.collections[collection].get(id), body.data);
  };
  const originalGet = repo.get;
  repo.get = async (collection, id, ...args) => {
    const current = await originalGet(collection, id, ...args);
    if (collection === 'posts' && id === 'a') current.imageId = 'replaced';
    return current;
  };
  assert.deepEqual(await backfillAlbums(repo, { pageSize: 2 }), { scanned: 4, updatedPosts: 1, updatedSummaries: 1 });
  assert.deepEqual(repo.collections.posts.get('a').imageIds, ['replaced']);
  assert.deepEqual(repo.collections.posts.get('b').imageIds, ['one', 'two']);
  assert.deepEqual(repo.collections.posts.get('a').$permissions, ['read("any")']);
  assert.equal(repo.collections.posts.get('a').content, 'Long post');
  assert.deepEqual(await backfillAlbums(repo, { pageSize: 2 }), { scanned: 4, updatedPosts: 0, updatedSummaries: 0 });
  assert.equal(patches.length, 2);
  assert.equal(disabled, false);
  repo.request = async (method, path, body) => {
    if (method === 'PUT') { disabled = !body.enabled; return; }
    throw new Error('unavailable');
  };
  repo.collections.posts.set('new', post('new', { imageId: 'new-photo' }));
  await assert.rejects(backfillAlbums(repo), /unavailable/);
  assert.equal(disabled, false, 'failure must restore source availability');
  assert.equal(await repo.acquire(), true, 'failure must release feed lock');
  await assert.rejects(backfillAlbums(repo), /busy/);
  await repo.release();
  repo.request = async (method, path, body) => {
    assert.equal(method, 'PUT');
    disabled = !body.enabled;
    if (disabled) throw new Error('disable response lost');
  };
  await assert.rejects(backfillAlbums(repo), /disable response lost/);
  assert.equal(disabled, false, 'ambiguous disable response must still restore availability');
});

test('deployment subscribes to current and legacy source mutations, including upserts, without feed recursion', async () => {
  const manifest = JSON.parse(await readFile(new URL('../appwrite.config.json', import.meta.url), 'utf8'));
  const feed = manifest.functions[0], events = feed.events;
  assert.equal(feed.schedule, '', 'Free-plan repair uses an interval rather than cron');
  assert.equal(feed.interval, 60, 'scheduled repair runs hourly on the Free plan');
  for (const [prefix, container, record] of [['databases', 'collections', 'documents'], ['tablesdb', 'tables', 'rows']]) {
    for (const collection of ['posts', 'likes', 'comments', 'reposts', 'profiles']) {
      for (const action of ['create', 'update', 'upsert', 'delete']) {
        assert(events.includes(`${prefix}.octopus-db.${container}.${collection}.${record}.*.${action}`));
      }
    }
  }
  assert(!events.some(event => event.includes('post-feed')));
});

test('summary bounds Unicode without splitting a surrogate pair and carries trusted counts', () => {
  const summary = feedDocument(post('p1', { content: '😀'.repeat(500), userText: '🪸'.repeat(500) }), 'avatar',
    { likeCount: 1201, commentCount: 3000, repostCount: 2000 }, now);
  assert.equal(Array.from(summary.content).length, 320);
  assert.equal(Array.from(summary.userText).length, 320);
  assert.equal(summary.content, '😀'.repeat(320));
  assert.equal(summary.likeCount, 1201);
  assert.equal(summary.sourceCreatedAt, now);
});

test('summary carries paired image dimensions and clears malformed or absent metadata', () => {
  const summary = overrides => feedDocument(post('p1', overrides), '', {}, now);
  assert.equal(summary({ imageWidth: 1200, imageHeight: 800 }).imageWidth, 1200);
  assert.equal(summary({ imageWidth: 1200, imageHeight: 800 }).imageHeight, 800);
  for (const dimensions of [{}, { imageWidth: 1200 }, { imageWidth: 0, imageHeight: 800 },
    { imageWidth: 65536, imageHeight: 800 }, { imageWidth: 1.5, imageHeight: 800 },
    { imageWidth: '1200', imageHeight: 800 }, { imageWidth: NaN, imageHeight: Infinity }]) {
    assert.equal(summary(dimensions).imageWidth, null);
    assert.equal(summary(dimensions).imageHeight, null);
  }
});

test('public policy respects collection grants, disabled collections and document security', () => {
  assert.equal(publiclyReadable(post(), { documentSecurity: false, $permissions: [] }), false);
  assert.equal(publiclyReadable(post(), { documentSecurity: true, $permissions: [] }), true);
  assert.equal(publiclyReadable(post('p', { $permissions: [] }), { documentSecurity: false, $permissions: ['read("any")'] }), true);
  assert.equal(publiclyReadable(post(), { enabled: false, documentSecurity: true }), false);
});

test('duplicate and out-of-order reaction events recount current source with totals above 1000', async () => {
  const repo = fixture();
  for (const action of ['create', 'create', 'delete']) {
    await execute(repo, { trigger: 'event', event: event('likes', action), payload: { targetId: 'p1', targetType: 'post' } });
  }
  assert.equal(repo.collections.feed.get('p1').likeCount, 1201);
  repo.counts.likes = 0;
  await execute(repo, { trigger: 'event', event: event('likes'), payload: { targetId: 'p1', targetType: 'post' } });
  assert.equal(repo.collections.feed.get('p1').likeCount, 0);
  assert.deepEqual(repo.collections.feed.get('p1').$permissions, ['read("any")']);
  assert.ok(repo.calls.some(call => call[0] === 'get' && call[1] === 'profiles' && call[2] === false));
});

test('deleted/private source removes stale feed and ignores obsolete event content', async () => {
  const repo = fixture();
  await refreshPost(repo, 'p1');
  repo.collections.posts.delete('p1');
  await execute(repo, { trigger: 'event', event: event('posts'), payload: post() });
  assert.equal(repo.collections.feed.size, 0);
  // Reactions can outlive a deleted post; an old reaction event must not
  // recreate its public summary from event data or leftover counter rows.
  await execute(repo, { trigger: 'event', event: event('comments'), payload: { postId: 'p1' } });
  assert.equal(repo.collections.feed.size, 0);
  repo.collections.posts.set('p1', post('p1', { $permissions: ['read("user:alice")'] }));
  await refreshPost(repo, 'p1');
  assert.equal(repo.collections.feed.size, 0);
});

test('source changing during counters is re-read before publishing', async () => {
  const repo = fixture();
  let changed = false;
  repo.countPublic = async () => {
    if (!changed) {
      changed = true;
      repo.collections.posts.set('p1', post('p1', { $updatedAt: 'new', title: 'New title' }));
    }
    return 10;
  };
  await refreshPost(repo, 'p1');
  assert.equal(repo.collections.feed.get('p1').title, 'New title');
});

test('busy execution is deferred and errors release the durable writer lock', async () => {
  const repo = fixture();
  await repo.acquire();
  const result = await execute(repo, { trigger: 'schedule' });
  assert.equal(result.body.busy, true);
  await repo.release();
  repo.countPublic = async () => { throw new Error('database unavailable'); };
  await assert.rejects(execute(repo, { trigger: 'event', event: event('posts'), payload: post() }));
  assert.equal(await repo.acquire(), true);
});

test('bounded reconciliation resumes, backfills and removes orphaned/private feed rows', async () => {
  const repo = fixture([post('a'), post('b'), post('c', { $permissions: [] })]);
  repo.collections.feed.set('orphan', { $id: 'orphan' });
  repo.collections.feed.set('c', { $id: 'c' });
  let completed = false;
  for (let run = 0; run < 10; run++) {
    const result = await execute(repo, { trigger: 'schedule' });
    if (result.body.cycleComplete) { completed = true; break; }
  }
  assert.ok(completed);
  assert.deepEqual([...repo.collections.feed.keys()].sort(), ['a', 'b']);
});

test('a failed post is reported and does not starve later posts during reconciliation', async () => {
  const repo = fixture([post('a'), post('b')]);
  const originalGet = repo.get.bind(repo);
  repo.get = async (collection, id, privileged) => {
    if (collection === 'posts' && id === 'a') throw new Error('bad source document');
    return originalGet(collection, id, privileged);
  };
  const result = await execute(repo, { trigger: 'schedule' });
  assert.equal(result.status, 500);
  assert.deepEqual(result.body.failed, [{ id: 'a', status: 500 }]);
  assert.ok(repo.collections.feed.has('b'));
  assert.equal((await repo.loadCheckpoint()).cursor, 'b');
});

test('reconciliation runs at most four workers and checkpoints only wholly settled groups', async t => {
  const ids = Array.from('abcdefghi'), repo = fixture(ids.map(id => post(id)));
  repo.config.batchSize = ids.length;
  const gates = new Map(), started = [], checkpoints = [];
  const originalGet = repo.get.bind(repo), originalPut = repo.put.bind(repo), originalCheckpoint = repo.checkpoint.bind(repo);
  let active = 0, maximum = 0;
  for (const id of ids) {
    let release;
    const waiting = new Promise(resolve => { release = resolve; });
    gates.set(id, { waiting, release });
  }
  t.after(() => { for (const gate of gates.values()) gate.release(); });
  repo.get = async (collection, id, privileged) => {
    if (collection === 'posts' && !started.includes(id)) {
      started.push(id); maximum = Math.max(maximum, ++active);
      await gates.get(id).waiting;
    }
    return originalGet(collection, id, privileged);
  };
  repo.put = async (...args) => { await originalPut(...args); active--; };
  repo.checkpoint = async state => {
    assert.equal(active, 0, 'no cursor can pass an unfinished row');
    checkpoints.push(structuredClone(state)); await originalCheckpoint(state);
  };
  const turn = () => new Promise(resolve => setImmediate(resolve));
  const running = reconcileBatch(repo);
  await turn();
  assert.deepEqual(started, ['a', 'b', 'c', 'd']); assert.equal(active, 4);
  for (const id of ['d', 'b', 'c']) gates.get(id).release();
  await turn();
  assert.equal(active, 1); assert.deepEqual(checkpoints, []);
  assert.deepEqual(started, ['a', 'b', 'c', 'd']);
  gates.get('a').release(); await turn();
  assert.deepEqual(checkpoints, [{ phase: 'posts', cursor: 'd' }]);
  assert.deepEqual(started, Array.from('abcdefgh'));
  for (const id of ['h', 'f', 'g', 'e']) gates.get(id).release();
  await turn();
  assert.deepEqual(checkpoints.map(state => state.cursor), ['d', 'h']);
  assert.deepEqual(started, ids);
  gates.get('i').release();
  assert.deepEqual(await running, { processed: 9, failed: [], cycleComplete: false });
  assert.equal(maximum, 4); assert.equal(active, 0);
  assert.deepEqual(checkpoints.map(state => state.cursor), ['d', 'h', 'i']);
});

test('reconciliation finishes its current group but starts no new group after the deadline', async t => {
  const repo = fixture(Array.from('abcdefgh', id => post(id)));
  repo.config.batchSize = 8;
  let clock = 0;
  t.mock.method(Date, 'now', () => clock);
  const originalPut = repo.put.bind(repo), started = [];
  const originalGet = repo.get.bind(repo);
  repo.get = async (collection, id, privileged) => {
    if (collection === 'posts' && !started.includes(id)) started.push(id);
    return originalGet(collection, id, privileged);
  };
  repo.put = async (...args) => { clock = 11; return originalPut(...args); };
  const result = await reconcileBatch(repo, 10);
  assert.deepEqual(result, { processed: 4, failed: [], cycleComplete: false });
  assert.deepEqual(started, Array.from('abcd'));
  assert.deepEqual(await repo.loadCheckpoint(), { phase: 'posts', cursor: 'd' });
  assert.deepEqual([...repo.collections.feed.keys()].sort(), Array.from('abcd'));
  const resumed = await reconcileBatch(repo, 100);
  assert.equal(resumed.processed, 4);
  assert.deepEqual([...repo.collections.feed.keys()].sort(), Array.from('abcdefgh'));
  assert.deepEqual(await repo.loadCheckpoint(), { phase: 'feed', cursor: null });
});

test('a failed group checkpoint retries all rows idempotently without skipping them', async () => {
  const repo = fixture(Array.from('abcde', id => post(id)));
  repo.config.batchSize = 5;
  const originalCheckpoint = repo.checkpoint.bind(repo);
  let unavailable = true;
  repo.checkpoint = async state => {
    if (unavailable) { unavailable = false; throw new Error('checkpoint unavailable'); }
    return originalCheckpoint(state);
  };
  await assert.rejects(reconcileBatch(repo), /checkpoint unavailable/);
  assert.deepEqual(await repo.loadCheckpoint(), { phase: 'posts', cursor: null });
  assert.deepEqual([...repo.collections.feed.keys()].sort(), Array.from('abcd'));
  const result = await reconcileBatch(repo);
  assert.deepEqual(result, { processed: 5, failed: [], cycleComplete: false });
  assert.deepEqual([...repo.collections.feed.keys()].sort(), Array.from('abcde'));
  assert.deepEqual(await repo.loadCheckpoint(), { phase: 'posts', cursor: 'e' });
});

test('profile updates preserve sweep progress; unrelated/comment-like events and HTTP are ignored', async () => {
  const repo = fixture();
  await repo.checkpoint({ phase: 'feed', cursor: 'stale' });
  await execute(repo, { trigger: 'event', event: event('profiles', 'update'), payload: { $id: 'alice' } });
  assert.deepEqual(await repo.loadCheckpoint(), { phase: 'feed', cursor: 'stale' });
  assert.equal((await execute(repo, { trigger: 'event', event: event('likes'), payload: { targetType: 'comment' } })).body.ignored, true);
  assert.equal((await execute(repo, { trigger: 'http' })).status, 403);
});

test('event routing accepts dotted database and document IDs in both vocabularies', async () => {
  const repo = fixture([post('p.1')]);
  repo.config.databaseId = 'octopus.db';
  const result = await execute(repo, { trigger: 'event', event: 'databases.octopus.db.collections.posts.documents.p.1.create', payload: { $id: 'p.1' } });
  assert.equal(result.body.result, 'updated');
  assert.ok(repo.collections.feed.has('p.1'));
  const profile = await execute(repo, { trigger: 'event', event: 'tablesdb.octopus.db.tables.profiles.rows.alice.jones.update', payload: { $id: 'alice.jones' } });
  assert.equal(profile.body.reconciliationPending, true);
});

test('REST counter asks for only one ID and exact total without administrator credentials', async () => {
  const requests = [];
  const repo = createRepository({ endpoint: 'https://example.invalid/v1', projectId: 'project', databaseId: 'db' }, 'secret',
    async (url, options) => { requests.push({ url, options }); return { ok: true, status: 200,
      json: async () => ({ total: 55555, documents: [{ $id: 'one' }] }) }; });
  assert.equal(await repo.countPublic('comments', []), 55555);
  const queries = requests[0].url.searchParams.getAll('queries[]').map(value => JSON.parse(value));
  assert.deepEqual(queries, [{ method: 'limit', values: [1] }, { method: 'select', values: ['$id'] }]);
  assert.equal(requests[0].options.headers['X-Appwrite-Key'], undefined);
});

test('private guest profiles are absent and private relation totals are zero', async () => {
  const repo = createRepository({ endpoint: 'https://example.invalid/v1', projectId: 'project', databaseId: 'db' }, 'secret',
    async () => ({ ok: false, status: 403, json: async () => ({}) }));
  assert.equal(await repo.get('profiles', 'alice', false), null);
  assert.equal(await repo.countPublic('likes', []), 0);
  await assert.rejects(repo.get('posts', 'p1'), error => error.status === 403);
});

test('a missing exact total fails instead of publishing incorrect zero counts', async () => {
  const repo = createRepository({ endpoint: 'https://example.invalid/v1', projectId: 'project', databaseId: 'db' }, 'secret',
    async () => ({ ok: true, status: 200, json: async () => ({ documents: [] }) }));
  await assert.rejects(repo.countPublic('comments', []), /exact total/);
});

test('schema/index verification accepts covering indexes and rejects unsafe schema drift', () => {
  assert.ok(compatibleAttribute({ type: 'string', size: 512, required: false }, { type: 'string', size: 320, required: false }));
  assert.equal(compatibleAttribute({ type: 'string', size: 40, required: false }, { type: 'string', size: 320, required: false }), false);
  assert.ok(compatibleAttribute({ ...imageAttributes[0] }, imageAttributes[0]));
  assert.equal(compatibleAttribute({ ...imageAttributes[0], max: 65536 }, imageAttributes[0]), false);
  assert.equal(compatibleAttribute({ ...imageAttributes[0], min: 0 }, imageAttributes[0]), false);
  assert.equal(compatibleAttribute({ ...imageAttributes[0], required: true }, imageAttributes[0]), false);
  assert.ok(coversIndex({ type: 'unique', status: 'available', attributes: ['postId'] }, { type: 'key', attributes: ['postId'] }));
  assert.ok(coversIndex({ type: 'unique', status: 'available', attributes: ['targetId', 'targetType', 'userId'] },
    { type: 'key', attributes: ['targetType', 'targetId'] }));
  assert.equal(coversIndex({ type: 'unique', status: 'available', attributes: ['userId', 'targetId', 'targetType'] },
    { type: 'key', attributes: ['targetType', 'targetId'] }), false);
  assert.equal(coversIndex({ type: 'key', status: 'failed', attributes: ['postId'] }, { type: 'key', attributes: ['postId'] }), false);
});

test('migration avoids unsupported array indexes, is idempotent and verify performs no writes', async () => {
  const config = { feedId: 'feed', controlId: 'control', databaseId: 'db' };
  const data = new Map(), collections = new Map();
  let writes = 0;
  const missing = () => { const error = new Error('not found'); error.status = 404; throw error; };
  const root = '/databases/db/collections';
  const repo = { config, collectionPath: id => `${root}/${id}`,
    async collection(id) { return collections.get(id) || missing(); },
    async request(method, path, body) {
      if (method === 'GET') {
        if (path.endsWith('/indexes')) return { indexes: [...data.values()].filter(value => value.path?.startsWith(path + '/')), total: 0 };
        return data.get(path) || missing();
      }
      writes++;
      if (path === root) {
        const value = { ...body, $permissions: body.permissions };
        collections.set(body.collectionId, value); return value;
      }
      if (path.endsWith('/indexes')) {
        assert.ok(!body.attributes.some(key => feedAttributes.some(attribute => attribute.key === key && attribute.array)),
          'Appwrite Cloud rejects custom indexes on array attributes');
        const value = { ...body, status: 'available', path: `${path}/${body.key}` };
        data.set(value.path, value); return value;
      }
      const base = path.slice(0, path.lastIndexOf('/'));
      const value = { ...body, type: path.slice(path.lastIndexOf('/') + 1), status: 'available' };
      data.set(`${base}/${body.key}`, value); return value;
    },
  };
  await migrate(repo);
  assert.equal(data.size, feedAttributes.length + imageAttributes.length + albumAttributes.length + 1 + indexes(config).length);
  assert.equal(collections.has('posts'), false, 'Source permissions must not be replaced by feed-only security rules');
  for (const collection of ['posts', 'feed']) {
    for (const attribute of [...imageAttributes, ...albumAttributes]) assert.deepEqual(data.get(`${root}/${collection}/attributes/${attribute.key}`),
      { ...attribute, status: 'available' });
  }
  const firstWrites = writes;
  await migrate(repo);
  await migrate(repo, { verify: true });
  assert.equal(writes, firstWrites);
  const width = data.get(`${root}/posts/attributes/imageWidth`);
  width.max = 65536;
  await assert.rejects(migrate(repo), /Incompatible or unavailable attribute posts.imageWidth/);
  assert.equal(writes, firstWrites, 'Conflicting metadata columns must never be silently replaced');
});
