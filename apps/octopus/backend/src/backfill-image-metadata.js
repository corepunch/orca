import { readFile } from 'node:fs/promises';
import { pathToFileURL } from 'node:url';
import { AppwriteRest } from '../../../chat/backend/appwrite/src/rest.js';
import { configuration, query } from './repository.js';
import { ImageMetadataError, readImageDimensions, validDimensions } from './image-metadata.js';

const select = ['$id', 'imageId', 'imageWidth', 'imageHeight'];
const dimensions = row => ({ imageWidth: row.imageWidth, imageHeight: row.imageHeight });
const same = (row, expected) => row.imageWidth === expected.imageWidth && row.imageHeight === expected.imageHeight;
const reason = error => error instanceof ImageMetadataError ? error.message
  : Number.isInteger(error.status) ? `Appwrite request failed (HTTP ${error.status}); check database access and schema.`
    : 'Unable to read or update image metadata; check storage/database access and retry.';

// Explicit operator action only: normal Function execution never reads image files.
export async function backfillImageMetadata({ rest, config = { databaseId: 'octopus-db', feedId: 'post-feed' }, readDimensions,
  bucketId = 'post-images', pageSize = 25, maxPosts = 1000, cursor = null,
  progress = () => {} }) {
  if (!Number.isInteger(pageSize) || pageSize < 1 || pageSize > 100)
    throw new ImageMetadataError('invalid_arguments', 'Page size must be between 1 and 100.');
  if (!Number.isInteger(maxPosts) || maxPosts < 1 || maxPosts > 10000)
    throw new ImageMetadataError('invalid_arguments', 'Maximum posts must be between 1 and 10000.');
  readDimensions ??= fileId => readImageDimensions({ endpoint: rest.endpoint, projectId: rest.projectId,
    apiKey: rest.apiKey, bucketId, fileId });
  const base = `/databases/${encodeURIComponent(config.databaseId)}/collections/`;
  const path = (collection, id) => `${base}${encodeURIComponent(collection)}/documents${id ? '/' + encodeURIComponent(id) : ''}`;
  const fields = '?queries[]=' + encodeURIComponent(query('select', null, select));
  const get = async (collection, id) => {
    try { return await rest.request('GET', path(collection, id) + fields); }
    catch (error) { if (error.status === 404) return null; throw error; }
  };
  const patch = (collection, id, pair) => rest.request('PATCH', path(collection, id), { body: { data: pair } });
  const cache = new Map();
  const result = { scanned: 0, updatedPosts: 0, updatedSummaries: 0, changedImages: 0, filesRead: 0, failed: [], cursor, complete: false };
  while (result.scanned < maxPosts) {
    const limit = Math.min(pageSize, maxPosts - result.scanned);
    const queries = [query('limit', null, [limit]), query('orderAsc', '$id'), query('select', null, select),
      ...(result.cursor ? [query('cursorAfter', null, [result.cursor])] : [])];
    const params = new URLSearchParams({ total: 'false' });
    for (const q of queries) params.append('queries[]', q);
    const page = await rest.request('GET', path('posts') + '?' + params);
    if (!Array.isArray(page.documents) || page.documents.length > limit) throw new Error('Appwrite returned an invalid or unbounded post page.');
    for (const post of page.documents) {
      if (!post.$id || post.$id === result.cursor) throw new Error('Appwrite returned an invalid post cursor.');
      result.scanned++;
      result.cursor = post.$id;
      if (!post.imageId) continue;
      try {
        let pair;
        if (validDimensions(post)) pair = dimensions(post);
        else {
          if (!cache.has(post.imageId)) {
            result.filesRead++;
            cache.set(post.imageId, Promise.resolve().then(() => readDimensions(post.imageId)));
          }
          pair = await cache.get(post.imageId);
          if (!validDimensions(pair)) throw new ImageMetadataError('invalid_image_header', 'Image reader returned invalid dimensions.');
        }
        // Never apply dimensions from a stale list snapshot to a replaced image.
        const current = await get('posts', post.$id);
        if (!current || current.imageId !== post.imageId) { result.changedImages++; continue; }
        if (validDimensions(current)) pair = dimensions(current);
        else { await patch('posts', post.$id, pair); result.updatedPosts++; }
        const feed = await get(config.feedId, post.$id);
        if (feed?.imageId === post.imageId && !same(feed, pair)) {
          await patch(config.feedId, post.$id, pair);
          result.updatedSummaries++;
        }
      } catch (error) { result.failed.push({ postId: post.$id, imageId: post.imageId, reason: reason(error) }); }
    }
    progress({ scanned: result.scanned, updatedPosts: result.updatedPosts, updatedSummaries: result.updatedSummaries,
      filesRead: result.filesRead, failed: result.failed.length, cursor: result.cursor });
    if (page.documents.length < limit) { result.complete = true; break; }
  }
  return result;
}

export async function runBackfill({ env = process.env, pageSize = 25, maxPosts = 1000, cursor = null, progress = console.log } = {}) {
  if (!env.APPWRITE_API_KEY) throw new ImageMetadataError('invalid_arguments', 'APPWRITE_API_KEY is required.');
  const manifest = JSON.parse(await readFile(new URL('../appwrite.config.json', import.meta.url), 'utf8'));
  const config = configuration({ ...env, APPWRITE_ENDPOINT: env.APPWRITE_ENDPOINT || manifest.endpoint,
    APPWRITE_PROJECT_ID: env.APPWRITE_PROJECT_ID || manifest.projectId });
  const bucketId = env.OCTOPUS_BUCKET_ID || 'post-images';
  const rest = new AppwriteRest({ endpoint: config.endpoint, projectId: config.projectId, apiKey: env.APPWRITE_API_KEY });
  progress(`Backfilling image metadata in ${config.projectId}/${config.databaseId}; bounded original headers only.`);
  return backfillImageMetadata({ rest, config, pageSize, maxPosts, cursor,
    readDimensions: fileId => readImageDimensions({ ...config, apiKey: env.APPWRITE_API_KEY, bucketId, fileId }),
    progress: update => progress(JSON.stringify(update)) });
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const flags = new Map();
    for (let i = 2; i < process.argv.length; i += 2) {
      if (!['--page-size', '--max-posts', '--cursor'].includes(process.argv[i]) || !process.argv[i + 1])
        throw new ImageMetadataError('invalid_arguments', 'Usage: node src/backfill-image-metadata.js [--page-size 25] [--max-posts 1000] [--cursor ID]');
      flags.set(process.argv[i], process.argv[i + 1]);
    }
    const result = await runBackfill({ pageSize: Number(flags.get('--page-size') || 25),
      maxPosts: Number(flags.get('--max-posts') || 1000), cursor: flags.get('--cursor') || null });
    console.log(JSON.stringify(result));
    if (result.failed.length) process.exitCode = 1;
  } catch (error) { console.error(reason(error)); process.exitCode = 1; }
}
