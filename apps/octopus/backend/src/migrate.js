import { pathToFileURL } from 'node:url';
import { configuration, createRepository, query } from './repository.js';

export const imageAttributes = ['imageWidth', 'imageHeight']
  .map(key => ({ key, type: 'integer', min: 1, max: 65535, required: false }));
export const albumAttributes = [
  { key: 'imageIds', type: 'string', size: 36, required: false, array: true },
  ...['imageWidths', 'imageHeights'].map(key => ({ key, type: 'integer', min: 0, max: 65535, required: false, array: true })),
];

export const feedAttributes = [
  { key: 'sourceCreatedAt', type: 'datetime', required: true },
  ...Object.entries({ title: 256, content: 320, postType: 16, authorId: 36, authorName: 256,
    imageId: 36, linkUrl: 2048, quoteSource: 320, userText: 320, avatarId: 36 })
    .map(([key, size]) => ({ key, size, type: 'string', required: false, default: '' })),
  { key: 'tags', type: 'string', size: 64, required: false, array: true },
  ...imageAttributes,
  ...albumAttributes,
  ...['likeCount', 'commentCount', 'repostCount'].map(key => ({ key, type: 'integer', min: 0, required: false, default: 0 })),
  { key: 'statsUpdatedAt', type: 'datetime', required: true },
];

export function indexes(config) {
  return [
    { collection: config.feedId, key: 'feed_created', attributes: ['sourceCreatedAt'], orders: ['DESC'] },
    { collection: config.feedId, key: 'feed_author_created', attributes: ['authorId', 'sourceCreatedAt'], orders: ['ASC', 'DESC'] },
    // Appwrite Cloud rejects custom indexes on array attributes. Tag queries use
    // the native array predicate and the separate creation-time index.
    { collection: config.feedId, key: 'feed_title_search', type: 'fulltext', attributes: ['title'] },
    { collection: 'posts', key: 'feed_author_created', attributes: ['authorId', '$createdAt'], orders: ['ASC', 'DESC'] },
    { collection: 'reposts', key: 'feed_user_created', attributes: ['userId', '$createdAt'], orders: ['ASC', 'DESC'] },
    { collection: 'likes', key: 'feed_target', attributes: ['targetType', 'targetId'] },
    { collection: 'likes', key: 'feed_viewer_target', attributes: ['targetType', 'targetId', 'userId'] },
    { collection: 'comments', key: 'feed_post', attributes: ['postId'] },
    { collection: 'reposts', key: 'feed_post', attributes: ['postId'] },
    { collection: 'reposts', key: 'feed_viewer_post', attributes: ['postId', 'userId'] },
  ];
}

export function compatibleAttribute(actual, expected) {
  return actual.type === expected.type && Boolean(actual.array) === Boolean(expected.array) &&
    Boolean(actual.required) === Boolean(expected.required) &&
    (expected.size === undefined || actual.size >= expected.size) &&
    (expected.min === undefined || actual.min === expected.min) &&
    (expected.max === undefined || actual.max === expected.max);
}

export function coversIndex(actual, expected) {
  const actualFields = actual.attributes || [];
  const equalityPrefix = actualFields.slice(0, expected.attributes.length);
  const sameFields = expected.orders || expected.type === 'fulltext'
    ? JSON.stringify(actualFields) === JSON.stringify(expected.attributes)
    : equalityPrefix.length === expected.attributes.length && expected.attributes.every(field => equalityPrefix.includes(field));
  return ['available', 'processing'].includes(actual.status) &&
    (actual.type === expected.type || (expected.type === 'key' && actual.type === 'unique')) &&
    sameFields &&
    (!expected.orders || JSON.stringify(actual.orders) === JSON.stringify(expected.orders));
}

export async function migrate(repo, { verify = false, wait = ms => new Promise(resolve => setTimeout(resolve, ms)) } = {}) {
  const config = repo.config;
  const root = `/databases/${encodeURIComponent(config.databaseId)}/collections`;
  async function collection(id, attributes) {
    let existing;
    try { existing = await repo.collection(id); }
    catch (error) {
      if (error.status !== 404 || verify) throw error;
      existing = await repo.request('POST', root, { collectionId: id, name: id, documentSecurity: true, permissions: [], enabled: true });
    }
    if (!existing.documentSecurity || existing.enabled === false || existing.$permissions?.length)
      throw new Error(`${id} must use document security, be enabled, and have no collection permissions.`);
    await ensureAttributes(id, attributes);
  }
  async function ensureAttributes(id, attributes) {
    for (const attribute of attributes) {
      const path = `${repo.collectionPath(id)}/attributes/${encodeURIComponent(attribute.key)}`;
      let actual;
      try { actual = await repo.request('GET', path); }
      catch (error) {
        if (error.status !== 404 || verify) throw error;
        actual = await repo.request('POST', `${repo.collectionPath(id)}/attributes/${attribute.type}`, {
          ...attribute, type: undefined });
      }
      for (let tries = 0; actual.status === 'processing' && tries < 30; tries++) {
        await wait(500);
        actual = await repo.request('GET', path);
      }
      if (actual.status !== 'available' || !compatibleAttribute(actual, attribute))
        throw new Error(`Incompatible or unavailable attribute ${id}.${attribute.key}; resolve without deleting data.`);
    }
  }
  // Source posts retain their existing collection permissions. Optional columns
  // allow old records and old clients to keep publishing without dimensions.
  await ensureAttributes('posts', [...imageAttributes, ...albumAttributes]);
  await collection(config.feedId, feedAttributes);
  await collection(config.controlId, [{ key: 'value', type: 'string', size: 1024, required: true }]);
  for (const expected of indexes(config)) {
    expected.type ||= 'key';
    const path = `${repo.collectionPath(expected.collection)}/indexes`;
    const list = await repo.request('GET', path, undefined, [query('limit', null, [100])]);
    if (list.total > 100) throw new Error('Too many indexes to verify in a single page.');
    let actual = list.indexes.find(index => coversIndex(index, expected));
    if (!actual) {
      if (verify) throw new Error(`Missing index ${expected.collection}.${expected.key}.`);
      if (list.indexes.some(index => index.key === expected.key)) throw new Error(`Conflicting index ${expected.collection}.${expected.key}.`);
      actual = await repo.request('POST', path, { key: expected.key, type: expected.type,
        attributes: expected.attributes, ...(expected.orders ? { orders: expected.orders } : {}) });
    }
    for (let tries = 0; actual.status === 'processing' && tries < 30; tries++) {
      await wait(500);
      actual = await repo.request('GET', `${path}/${encodeURIComponent(actual.key)}`);
    }
    if (actual.status !== 'available') throw new Error(`Index ${expected.collection}.${actual.key} is not available; retry verification later.`);
  }
  return { verified: true };
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const repo = createRepository(configuration(), process.env.APPWRITE_API_KEY);
    console.log(JSON.stringify(await migrate(repo, { verify: process.argv.includes('--verify') })));
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
