import test from 'node:test';
import assert from 'node:assert/strict';
import { parseImageDimensions, readImageDimensions } from '../src/image-metadata.js';
import { backfillImageMetadata } from '../src/backfill-image-metadata.js';

function png(width = 640, height = 480) {
  const bytes = Buffer.alloc(33);
  Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]).copy(bytes);
  bytes.writeUInt32BE(13, 8); bytes.write('IHDR', 12);
  bytes.writeUInt32BE(width, 16); bytes.writeUInt32BE(height, 20);
  return bytes;
}
function segment(marker, data) {
  const header = Buffer.from([255, marker, 0, 0]);
  header.writeUInt16BE(data.length + 2, 2);
  return Buffer.concat([header, data]);
}
function jpeg(orientation, little = true, afterFrame = false) {
  const frame = Buffer.from([8, 1, 224, 2, 128, 1, 1, 17, 0]); // 640 x 480
  const exif = Buffer.alloc(32); exif.write('Exif\0\0'); exif.write(little ? 'II' : 'MM', 6);
  const u16 = (value, offset) => little ? exif.writeUInt16LE(value, offset) : exif.writeUInt16BE(value, offset);
  const u32 = (value, offset) => little ? exif.writeUInt32LE(value, offset) : exif.writeUInt32BE(value, offset);
  u16(42, 8); u32(8, 10); u16(1, 14);
  u16(0x112, 16); u16(3, 18); u32(1, 20); u16(orientation, 24);
  const parts = [segment(0xe1, exif), segment(0xc0, frame)];
  if (afterFrame) parts.reverse();
  return Buffer.concat([Buffer.from([255, 216]), ...parts, Buffer.from([255, 218])]);
}

test('PNG and JPEG header dimensions account for all EXIF display rotations and byte orders', () => {
  assert.deepEqual(parseImageDimensions(png()), { imageWidth: 640, imageHeight: 480 });
  for (const little of [true, false]) for (let orientation = 1; orientation <= 8; orientation++) {
    const expected = orientation >= 5 ? { imageWidth: 480, imageHeight: 640 } : { imageWidth: 640, imageHeight: 480 };
    assert.deepEqual(parseImageDimensions(jpeg(orientation, little)), expected);
    assert.deepEqual(parseImageDimensions(jpeg(orientation, little, true)), expected);
  }
});

test('header parser waits for complete metadata and rejects invalid formats, sizes and EXIF', () => {
  const source = jpeg(6);
  for (let length = 0; length < source.length; length++) assert.equal(parseImageDimensions(source.subarray(0, length)), null);
  assert.equal(parseImageDimensions(png().subarray(0, 32)), null);
  assert.throws(() => parseImageDimensions(Buffer.from('unsupported')), /Only JPEG, PNG, GIF and WebP/);
  assert.throws(() => parseImageDimensions(png(0, 20)), /between 1 and 65535/);
  assert.throws(() => parseImageDimensions(png(65536, 20)), /between 1 and 65535/);
  const badPNG = png(); badPNG.writeUInt32BE(99, 8);
  assert.throws(() => parseImageDimensions(badPNG), /IHDR/);
  assert.throws(() => parseImageDimensions(jpeg(9)), /orientation/);
  const badEXIF = jpeg(1); badEXIF.writeUInt32LE(9999, 16);
  assert.throws(() => parseImageDimensions(badEXIF), /directory offset/);
  assert.throws(() => parseImageDimensions(Buffer.from([255, 216, 255, 192, 0, 1])), /segment length/);
});

test('GIF and all WebP frame types expose true dimensions without decoding image content', () => {
  for (const signature of ['GIF87a', 'GIF89a']) {
    const bytes = Buffer.alloc(13); bytes.write(signature); bytes.writeUInt16LE(320, 6); bytes.writeUInt16LE(180, 8);
    assert.deepEqual(parseImageDimensions(bytes), { imageWidth: 320, imageHeight: 180 });
  }
  for (const type of ['VP8 ', 'VP8L', 'VP8X']) {
    const bytes = Buffer.alloc(30); bytes.write('RIFF'); bytes.writeUInt32LE(22, 4); bytes.write('WEBP', 8); bytes.write(type, 12);
    bytes.writeUInt32LE(type === 'VP8L' ? 5 : 10, 16);
    if (type === 'VP8 ') { bytes.set([0x9d, 1, 0x2a], 23); bytes.writeUInt16LE(320, 26); bytes.writeUInt16LE(180, 28); }
    if (type === 'VP8L') { bytes[20] = 0x2f; bytes.writeUInt32LE(319 + (179 << 14), 21); }
    if (type === 'VP8X') { bytes.writeUIntLE(319, 24, 3); bytes.writeUIntLE(179, 27, 3); }
    assert.deepEqual(parseImageDimensions(bytes), { imageWidth: 320, imageHeight: 180 });
    if (type === 'VP8X') { bytes[20] = 8; assert.throws(() => parseImageDimensions(bytes), /EXIF orientation cannot be verified/); }
  }
});

function streamingFetch(chunks, status = 206) {
  const state = { reads: 0, cancelled: false, signal: null, request: null };
  const fetch = async (url, request) => {
    state.url = url; state.request = request; state.signal = request.signal;
    return { ok: status >= 200 && status < 300, status, body: { getReader: () => ({
      async read() { const value = chunks[state.reads++]; return value ? { done: false, value } : { done: true }; },
      async cancel() { state.cancelled = true; },
    }) } };
  };
  return { fetch, state };
}
const downloadOptions = { endpoint: 'https://example.invalid/v1', projectId: 'project', apiKey: 'hidden', bucketId: 'post-images', fileId: 'file' };

test('header reader requests a bounded Range and cancels immediately after dimensions even if Range is ignored', async () => {
  const { fetch, state } = streamingFetch([png().subarray(0, 12), png().subarray(12), Buffer.alloc(1000000)], 200);
  assert.deepEqual(await readImageDimensions({ ...downloadOptions, fetch }), { imageWidth: 640, imageHeight: 480 });
  assert.equal(state.request.headers.Range, 'bytes=0-262143');
  assert.equal(state.request.redirect, 'error');
  assert.match(state.url, /\/storage\/buckets\/post-images\/files\/file\/download$/);
  assert.equal(state.reads, 2);
  assert.equal(state.cancelled, true);
  assert.equal(state.signal.aborted, true);
});

test('header reader cancels oversized and truncated streams without exposing downloaded bytes', async () => {
  // Valid JPEG APP2 segment cannot fit in our deliberately small test cap.
  const prefix = Buffer.from([255, 216, 255, 226, 255, 255]);
  const large = streamingFetch([Buffer.concat([prefix, Buffer.alloc(500)])]);
  await assert.rejects(readImageDimensions({ ...downloadOptions, ...large, maxBytes: 64 }), /64-byte header limit/);
  assert.equal(large.state.reads, 1); assert.equal(large.state.cancelled, true); assert.equal(large.state.signal.aborted, true);
  const truncated = streamingFetch([jpeg(6).subarray(0, 15)]);
  await assert.rejects(readImageDimensions({ ...downloadOptions, ...truncated }), /ended before its dimensions/);
  assert.equal(truncated.state.cancelled, true); assert.equal(truncated.state.signal.aborted, true);
  const failed = streamingFetch([], 403);
  await assert.rejects(readImageDimensions({ ...downloadOptions, ...failed }), /HTTP 403/);
  assert.equal(failed.state.signal.aborted, true);
  await assert.rejects(readImageDimensions({ ...downloadOptions, fetch: async () => { throw new Error('secret file bytes'); } }),
    error => /check storage access/.test(error.message) && !error.message.includes('secret'));
});

function fixture(posts, feeds = [], beforeGet = () => {}) {
  const source = new Map(posts.map(row => [row.$id, { ...row }]));
  const summary = new Map(feeds.map(row => [row.$id, { ...row }]));
  const calls = [], updates = [];
  const rest = { async request(method, path, options) {
    calls.push({ method, path, options });
    const url = new URL(path, 'https://example.invalid');
    const [, collection, id] = url.pathname.match(/\/collections\/([^/]+)\/documents(?:\/([^/]+))?$/);
    const rows = collection === 'posts' ? source : summary;
    if (method === 'GET' && !id) {
      const queries = url.searchParams.getAll('queries[]').map(q => JSON.parse(q));
      assert.equal(url.searchParams.get('total'), 'false');
      assert.deepEqual(queries.find(q => q.method === 'select').values, ['$id', 'imageId', 'imageWidth', 'imageHeight']);
      assert.equal(queries.find(q => q.method === 'orderAsc').attribute, '$id');
      const limit = queries.find(q => q.method === 'limit').values[0];
      const cursor = queries.find(q => q.method === 'cursorAfter')?.values[0];
      return { documents: [...rows.values()].sort((a, b) => a.$id.localeCompare(b.$id))
        .filter(row => !cursor || row.$id > cursor).slice(0, limit).map(row => ({ ...row })) };
    }
    if (method === 'GET') {
      beforeGet(collection, id, rows);
      if (!rows.has(id)) throw Object.assign(new Error('missing'), { status: 404 });
      return { ...rows.get(id) };
    }
    assert.equal(method, 'PATCH');
    assert.deepEqual(Object.keys(options.body), ['data']);
    assert.deepEqual(Object.keys(options.body.data).sort(), ['imageHeight', 'imageWidth']);
    updates.push({ collection, id, data: options.body.data });
    Object.assign(rows.get(id), options.body.data);
    return rows.get(id);
  } };
  return { rest, source, summary, calls, updates };
}
const config = { databaseId: 'octopus-db', feedId: 'post-feed' };

test('backfill deduplicates shared files across bounded cursor pages, preserves fields, skips valid pairs and repairs summaries', async () => {
  const posts = [
    { $id: 'a', imageId: 'shared', caption: 'keep', $permissions: ['read(any)'] },
    { $id: 'b', imageId: 'shared' },
    { $id: 'c', imageId: 'already', imageWidth: 99, imageHeight: 101 },
    { $id: 'd', imageId: '' },
  ];
  const fixtureData = fixture(posts, posts.slice(0, 3).map(row => ({ $id: row.$id, imageId: row.imageId, likesCount: 77 })));
  const reads = [];
  const result = await backfillImageMetadata({ ...fixtureData, config, pageSize: 2,
    readDimensions: async id => { reads.push(id); return { imageWidth: 640, imageHeight: 480 }; } });
  assert.equal(result.complete, true); assert.equal(result.scanned, 4);
  assert.equal(result.updatedPosts, 2); assert.equal(result.updatedSummaries, 3); assert.equal(result.filesRead, 1);
  assert.deepEqual(reads, ['shared']); assert.deepEqual(result.failed, []);
  assert.equal(fixtureData.source.get('a').caption, 'keep');
  assert.deepEqual(fixtureData.source.get('a').$permissions, ['read(any)']);
  assert.equal(fixtureData.summary.get('a').likesCount, 77);
  assert.equal(fixtureData.summary.get('c').imageWidth, 99);
  assert.equal(fixtureData.calls.filter(c => c.path.includes('likes') || c.path.includes('comments')).length, 0);
  const repeated = await backfillImageMetadata({ ...fixtureData, config, readDimensions: async () => assert.fail('No image reread') });
  assert.equal(repeated.updatedPosts, 0); assert.equal(repeated.updatedSummaries, 0); assert.equal(repeated.filesRead, 0);
});

test('backfill rechecks source and summary image IDs and never patches a replaced image', async () => {
  const fixtureData = fixture([{ $id: 'a', imageId: 'old' }, { $id: 'b', imageId: 'shared' }],
    [{ $id: 'a', imageId: 'old' }, { $id: 'b', imageId: 'changed-feed' }], (collection, id, rows) => {
      if (collection === 'posts' && id === 'a') rows.get(id).imageId = 'new';
    });
  const result = await backfillImageMetadata({ ...fixtureData, config,
    readDimensions: async () => ({ imageWidth: 100, imageHeight: 200 }) });
  assert.equal(result.changedImages, 1); assert.equal(result.updatedPosts, 1); assert.equal(result.updatedSummaries, 0);
  assert.deepEqual(fixtureData.updates.map(u => [u.collection, u.id]), [['posts', 'b']]);
});

test('backfill resumes at the last bounded cursor and reports per-post failures without leaking upstream errors', async () => {
  const fixtureData = fixture([{ $id: 'a', imageId: 'bad' }, { $id: 'b', imageId: 'good' }, { $id: 'c', imageId: 'good' }]);
  const readDimensions = async id => { if (id === 'bad') throw new Error('secret bytes'); return { imageWidth: 20, imageHeight: 30 }; };
  const first = await backfillImageMetadata({ ...fixtureData, config, maxPosts: 2, pageSize: 2, readDimensions });
  assert.equal(first.complete, false); assert.equal(first.cursor, 'b'); assert.equal(first.scanned, 2);
  assert.equal(first.failed[0].postId, 'a'); assert.equal(first.failed[0].imageId, 'bad');
  assert.equal(JSON.stringify(first.failed).includes('secret'), false);
  const next = await backfillImageMetadata({ ...fixtureData, config, cursor: first.cursor, readDimensions });
  assert.equal(next.complete, true); assert.equal(next.scanned, 1); assert.equal(next.updatedPosts, 1);
});
