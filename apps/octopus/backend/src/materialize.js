import { query } from './repository.js';
import { albumFields } from './albums.js';

const PUBLIC_READ = 'read("any")';
const equal = (field, value) => query('equal', field, [value]);
const excerpt = value => Array.from(typeof value === 'string' ? value : '').slice(0, 320).join('');
const text = value => typeof value === 'string' ? value : '';
const imageEdge = value => Number.isInteger(value) && value > 0 && value <= 65535;

// Collection permissions and document permissions are additive in Appwrite.
// Document permissions have no effect when documentSecurity is disabled.
export function publiclyReadable(document, collection) {
  return collection.enabled !== false && (collection.$permissions?.includes(PUBLIC_READ) ||
    (collection.documentSecurity && document.$permissions?.includes(PUBLIC_READ)));
}

export function feedDocument(post, avatarId, counts, now) {
  const album = albumFields(post);
  const hasDimensions = imageEdge(post.imageWidth) && imageEdge(post.imageHeight);
  return { sourceCreatedAt: post.$createdAt, title: text(post.title), content: excerpt(post.content),
    postType: ['text', 'photo', 'quote', 'link'].includes(post.postType) ? post.postType : 'text',
    authorId: text(post.authorId), authorName: text(post.authorName) || 'Someone',
    tags: Array.isArray(post.tags) ? post.tags.slice(0, 10).map(tag => text(tag).slice(0, 64)) : [],
    ...album,
    imageId: album.imageIds[0] || text(post.imageId),
    imageWidth: album.imageIds.length ? album.imageWidths[0] || null : hasDimensions ? post.imageWidth : null,
    imageHeight: album.imageIds.length ? album.imageHeights[0] || null : hasDimensions ? post.imageHeight : null,
    linkUrl: text(post.linkUrl), quoteSource: excerpt(post.quoteSource),
    userText: excerpt(post.userText), avatarId: text(avatarId), ...counts, statsUpdatedAt: now };
}

export async function refreshPost(repo, id, now = () => new Date().toISOString()) {
  for (let attempt = 0; attempt < 3; attempt++) {
    const [post, collection] = await Promise.all([repo.get('posts', id), repo.collection('posts')]);
    if (!post || !publiclyReadable(post, collection)) {
      await repo.remove(repo.config.feedId, id);
      return 'removed';
    }
    const [likeCount, commentCount, repostCount, profile] = await Promise.all([
      repo.countPublic('likes', [equal('targetType', 'post'), equal('targetId', id)]),
      repo.countPublic('comments', [equal('postId', id)]),
      repo.countPublic('reposts', [equal('postId', id)]),
      repo.get('profiles', post.authorId, false),
    ]);
    // Ignore an old event payload and retry if the source changed during reads.
    const current = await repo.get('posts', id);
    if (!current || current.$updatedAt !== post.$updatedAt) continue;
    await repo.put(repo.config.feedId, id,
      feedDocument(post, profile?.avatarId, { likeCount, commentCount, repostCount }, now()), [PUBLIC_READ]);
    return 'updated';
  }
  throw new Error('Post changed repeatedly; reconciliation will retry.');
}

export async function reconcileBatch(repo, deadline = Date.now() + 360000) {
  let state = await repo.loadCheckpoint();
  if (!['posts', 'feed'].includes(state.phase)) state = { phase: 'posts', cursor: null };
  const collection = state.phase === 'posts' ? 'posts' : repo.config.feedId;
  const filters = [query('orderAsc', '$id'), query('limit', null, [repo.config.batchSize]), query('select', null, ['$id'])];
  if (state.cursor) filters.push(query('cursorAfter', null, [state.cursor]));
  let page;
  try { page = await repo.list(collection, filters); }
  catch (error) {
    if (error.status !== 404 || !state.cursor) throw error;
    // A deleted cursor is safe to restart: materialization is idempotent.
    await repo.checkpoint({ phase: state.phase, cursor: null });
    return { processed: 0, cycleComplete: false };
  }
  let processed = 0;
  const failed = [];
  for (let offset = 0; offset < page.documents.length; offset += 4) {
    if (Date.now() > deadline) return { processed, failed, cycleComplete: false };
    const group = page.documents.slice(offset, offset + 4);
    const failures = await Promise.all(group.map(async doc => {
      // Failed rows advance with their group and are retried next cycle, so one
      // unavailable dependency cannot permanently starve the rest of the sweep.
      try { await refreshPost(repo, doc.$id); return null; }
      catch (error) { return { id: doc.$id, status: error.status || 500 }; }
    }));
    failed.push(...failures.filter(Boolean));
    // All earlier rows have settled before publishing this contiguous cursor.
    // An interrupted group is safely repeated from the previous checkpoint.
    state.cursor = group.at(-1).$id;
    await repo.checkpoint(state);
    processed += group.length;
  }
  const complete = page.documents.length < repo.config.batchSize;
  if (complete) await repo.checkpoint({ phase: state.phase === 'posts' ? 'feed' : 'posts', cursor: null });
  return { processed: page.documents.length, failed, cycleComplete: complete && state.phase === 'feed' };
}

function eventTarget(event, payload, databaseId) {
  // Appwrite's legacy database API emits databases.* events. Recognize newer
  // tablesdb equivalents too; the deployment manifest targets the legacy API.
  const prefixes = [`databases.${databaseId}.collections.`, `tablesdb.${databaseId}.tables.`];
  const prefix = prefixes.find(value => event.startsWith(value));
  if (!prefix) return null;
  const resource = event.slice(prefix.length);
  if (/^(posts|likes|comments|reposts|profiles)\.update$/.test(resource)) return { profile: true };
  // Appwrite IDs may contain dots (including readable account/profile IDs).
  const match = resource.match(/^(posts|likes|comments|reposts|profiles)\.(?:documents|rows)\..+\.(?:create|update|upsert|delete)$/);
  if (!match) return null;
  const collection = match[1];
  if (collection === 'posts') return { id: payload.$id };
  if (collection === 'likes') return payload.targetType === 'post' ? { id: payload.targetId } : null;
  if (collection === 'comments' || collection === 'reposts') return { id: payload.postId };
  if (collection === 'profiles') return { profile: true };
  return null;
}

export async function execute(repo, { trigger, event = '', payload = {}, reset = false }) {
  if (trigger !== 'event' && trigger !== 'schedule' && trigger !== 'admin')
    return { status: 403, body: { error: 'Only internal events, schedules and administrator reconciliation are accepted.' } };
  const target = trigger === 'event' ? eventTarget(event, payload, repo.config.databaseId) : null;
  if (trigger === 'event' && !target) return { status: 200, body: { ignored: true } };
  // Profile and collection policy changes are handled by the recurring sweep.
  // Resetting its cursor on every profile update could starve later posts.
  if (target?.profile) return { status: 200, body: { reconciliationPending: true } };
  if (!await repo.acquire()) return { status: 202, body: { busy: true, reconciliationPending: true } };
  try {
    if (reset && trigger === 'admin') await repo.checkpoint({ phase: 'posts', cursor: null });
    if (target?.id) return { status: 200, body: { result: await refreshPost(repo, target.id) } };
    const body = await reconcileBatch(repo);
    return { status: body.failed?.length ? 500 : 200, body };
  } finally { await repo.release(); }
}
