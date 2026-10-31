import { pathToFileURL } from 'node:url';
import { albumFields } from './albums.js';
import { configuration, createRepository, query } from './repository.js';

export const albumSelect = ['$id', 'imageId', 'imageWidth', 'imageHeight', 'imageIds', 'imageWidths', 'imageHeights'];

// Patch only new columns. In particular, never replace content, timestamps,
// counters or document permissions, and never discard an existing album.
async function backfillRows(repo, { pageSize = 25, maxRows = 10000, progress = () => {} } = {}) {
  if (!Number.isInteger(pageSize) || pageSize < 1 || pageSize > 100 || !Number.isInteger(maxRows) || maxRows < 1)
    throw new Error('Use a page size from 1 to 100 and a positive maximum row count.');
  const result = { scanned: 0, updatedPosts: 0, updatedSummaries: 0 };
  for (const collection of ['posts', repo.config.feedId]) {
    let cursor;
    for (;;) {
      const queries = [query('limit', null, [pageSize]), query('orderAsc', '$id'), query('select', null, albumSelect)];
      if (cursor) queries.push(query('cursorAfter', null, [cursor]));
      const page = await repo.list(collection, queries);
      if (!Array.isArray(page.documents) || page.documents.length > pageSize) throw new Error('Invalid album migration page.');
      for (const snapshot of page.documents) {
        if (++result.scanned > maxRows) throw new Error('Album migration row limit reached; rerun with a higher maximum.');
        if (!snapshot.$id || snapshot.$id === cursor) throw new Error('Invalid album migration cursor.');
        cursor = snapshot.$id;
        // A fresh read prevents a replaced legacy image from using the list snapshot.
        const current = await repo.get(collection, snapshot.$id);
        if (!current || current.imageIds?.length) continue;
        const data = albumFields(current);
        if (!data.imageIds.length) continue;
        if (Object.entries(data).every(([key, value]) => JSON.stringify(current[key]) === JSON.stringify(value))) continue;
        await repo.request('PATCH', `${repo.collectionPath(collection)}/documents/${encodeURIComponent(snapshot.$id)}`, { data });
        result[collection === 'posts' ? 'updatedPosts' : 'updatedSummaries']++;
      }
      progress({ ...result });
      if (page.documents.length < pageSize) break;
    }
  }
  return result;
}

export async function backfillAlbums(repo, options) {
  if (!await repo.acquire()) throw new Error('Feed writer is busy; retry the album migration later.');
  let source, disabled = false;
  try {
    source = await repo.collection('posts');
    if (source.enabled !== false) {
      // Functions are not the only writers: website/mobile clients write posts
      // directly. Freeze their requests for the short metadata-only upgrade.
      disabled = true;
      await repo.request('PUT', repo.collectionPath('posts'), { name: source.name || 'posts', enabled: false });
    }
    return await backfillRows(repo, options);
  } finally {
    try {
      if (disabled) {
        try { await repo.request('PUT', repo.collectionPath('posts'), { name: source.name || 'posts', enabled: true }); }
        catch { throw new Error('Album migration could not restore the posts collection. Reenable it in Appwrite Console before resuming clients.'); }
      }
    } finally { await repo.release(); }
  }
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try { console.log(JSON.stringify(await backfillAlbums(createRepository(configuration(), process.env.APPWRITE_API_KEY)))); }
  catch (error) { console.error(error.message); process.exitCode = 1; }
}
