import { eq, key, order, reject } from './rest.js';

const enc = encodeURIComponent;
export const deletionId = userId => key('delete-account:' + userId);
const PHASES = ['disable', 'apple', 'messages', 'attachments', 'invites', 'members', 'rooms',
  'posts', 'comments', 'likes', 'reposts', 'following', 'followers', 'feed', 'files', 'blocks', 'blocked', 'reports', 'state', 'profile', 'account'];
const pending = () => ({ accepted: true, deleted: false });

export function deletionTransport(rest, { now = Date.now, milliseconds = 35000 } = {}) {
  const end = now() + milliseconds, locks = new Set();
  return { isDeletionTransport: true, signal() {
    const remaining = Math.min(10000, end - now());
    if (remaining <= 0) reject(503, 'deletion_deadline', 'Account deletion will continue in a later worker.');
    return AbortSignal.timeout(Math.max(1, Math.ceil(remaining)));
  }, async request(method, path, options = {}) {
    let lock;
    if (method === 'POST' && path.includes('/service-state/documents')) {
      try { if (JSON.parse(options.body?.data?.payload).kind === 'lock') lock = options.body.documentId; } catch { /* Ordinary state. */ }
    }
    const controlId = path.split('/service-state/documents/')[1];
    const releasing = ['GET', 'DELETE'].includes(method) && locks.has(controlId);
    const remaining = releasing ? 3000 : Math.min(20000, end - now());
    if (remaining <= 0) reject(503, 'deletion_deadline', 'Account deletion will continue in a later worker.');
    // Lock release must outlive work cancellation. Two nested room/job releases
    // need at most 12 seconds beyond the 35-second work deadline.
    const signal = AbortSignal.timeout(Math.max(1, Math.ceil(remaining)));
    const result = await rest.request(method, path, { ...options,
      signal: releasing || !options.signal ? signal : AbortSignal.any([options.signal, signal]) });
    if (lock) locks.add(lock);
    if (releasing && method === 'DELETE') locks.delete(controlId);
    return result;
  } };
}

// Account deletion is durable work, rather than a request-sized best effort.
// Rows and files disappear before their identifying metadata, and Auth is last.
export class AccountDeletion {
  constructor(repo, auth, { now = Date.now, appleClientId, appleClientSecret, fetch: fetcher = fetch } = {}) {
    this.repo = repo; this.auth = auth; this.now = now;
    this.appleClientId = appleClientId; this.appleClientSecret = appleClientSecret; this.fetch = fetcher;
  }
  async missing(action) {
    try { return await action(); } catch (error) { if (error.status === 404) return null; throw error; }
  }
  async request(user, body) {
    if (body?.confirmation !== 'DELETE' || Object.keys(body).some(field => field !== 'confirmation'))
      reject(400, 'validation', 'Confirm permanent account deletion.');
    const id = deletionId(user.$id);
    let job = await this.repo.get('deletions', id);
    if (!job) {
      const params = new URLSearchParams();
      params.append('queries[]', JSON.stringify(eq('userId', user.$id)));
      params.append('queries[]', JSON.stringify({ method: 'limit', values: [100] }));
      const identities = await this.auth.rest.request('GET', '/users/identities?' + params);
      if (identities.total > (identities.identities || []).length)
        reject(503, 'deletion_configuration', 'Account deletion needs administrator assistance. Please contact support.');
      const apple = (identities.identities || []).filter(identity => identity.provider === 'apple');
      if (apple.length && (!this.appleClientId || !this.appleClientSecret || apple.some(identity =>
        !identity.providerRefreshToken && !identity.providerAccessToken)))
        reject(503, 'deletion_configuration', 'Account deletion needs the Apple sign-in revocation service. Please contact support.');
      const tokens = apple.map(identity => ({ token: identity.providerRefreshToken || identity.providerAccessToken,
        hint: identity.providerRefreshToken ? 'refresh_token' : 'access_token' }));
      job = { kind: 'account_deletion', user_id: user.$id, email: user.email || '', phone: user.phone || '',
        phase: PHASES[0], cursor: null, apple_tokens: tokens, requested_at: new Date(this.now()).toISOString() };
      if (Buffer.byteLength(JSON.stringify(job)) > 15000)
        reject(503, 'deletion_configuration', 'Account deletion needs administrator assistance. Please contact support.');
      try { job = await this.repo.create('deletions', job, id); }
      catch (error) { if (error.status !== 409) throw error; job = await this.repo.get('deletions', id); }
    }
    // The event worker repeats this if the request dies after saving the job.
    // Blocking the shared account also stops direct Octopus/website writers.
    await this.missing(() => this.auth.rest.request('PATCH', `/users/${enc(user.$id)}/status`, { body: { status: false } }));
    return pending();
  }
  rawPath(collection, id) {
    return `/databases/${enc(this.repo.databaseId)}/collections/${enc(collection)}/documents${id ? '/' + enc(id) : ''}`;
  }
  async rawList(collection, queries = [], limit = 25) {
    const params = new URLSearchParams({ total: 'false' });
    [...queries, { method: 'limit', values: [limit] }, order('$id')].forEach(query => params.append('queries[]', JSON.stringify(query)));
    const result = await this.missing(() => this.auth.rest.request('GET', this.rawPath(collection) + '?' + params));
    return result?.documents || [];
  }
  rawRemove(collection, id) { return this.missing(() => this.auth.rest.request('DELETE', this.rawPath(collection, id))); }
  async save(job, changes) { return this.repo.update('deletions', job.id, { ...job, ...changes }); }
  async advance(job) { return this.save(job, { phase: PHASES[PHASES.indexOf(job.phase) + 1], cursor: null }); }
  async scan(job, collection, predicate, remove, raw = false) {
    const filters = job.cursor ? [{ method: 'cursorAfter', values: [job.cursor] }] : [];
    let rows;
    try { rows = raw ? await this.rawList(collection, filters) : await this.repo.list(collection, [...filters, order('$id')], 25); }
    catch (error) {
      // Another user can delete a retained cursor while this job is paused.
      if (job.cursor && error.status === 404) return this.save(job, { cursor: null });
      throw error;
    }
    let cursor = job.cursor;
    for (const row of rows) {
      if (predicate(row)) await remove(row);
      else cursor = raw ? row.$id : row.id;
    }
    // A cursor must reference a retained row; deleted cursor IDs are invalid.
    if (rows.length < 25) return this.advance(job);
    return this.save(job, { cursor });
  }
  async removePhotos(row) {
    if (row.object_key) await this.repo.removeFile(row.object_key);
    if (row.thumbnail_key) await this.repo.removeFile(row.thumbnail_key);
    await this.repo.remove('attachments', row.id);
  }
  async relationBatch(collection, filters) {
    const rows = await this.rawList(collection, filters);
    for (const row of rows) await this.rawRemove(collection, row.$id);
    return rows.length > 0;
  }
  async removeComment(row) {
    if (await this.relationBatch('likes', [eq('targetType', 'comment'), eq('targetId', row.$id)])) return false;
    await this.rawRemove('comments', row.$id); return true;
  }
  async memberStep(job) {
    if (job.empty_room) {
      const roomId = job.empty_room;
      return this.repo.lock('room:' + roomId, async () => {
        const messages = await this.repo.list('messages', [eq('room_id', roomId)], 1);
        if (messages.length) { await this.repo.remove('messages', messages[0].id); return this.save(job, {}); }
        const photos = await this.repo.list('attachments', [eq('room_id', roomId)], 1);
        if (photos.length) { await this.removePhotos(photos[0]); return this.save(job, {}); }
        const invites = await this.repo.list('invites', [eq('room_id', roomId)], 1);
        if (invites.length) { await this.repo.remove('invites', invites[0].id); return this.save(job, {}); }
        for (const row of await this.repo.list('members', [eq('room_id', roomId)])) await this.repo.remove('members', row.id);
        await this.repo.remove('rooms', roomId);
        return this.save(job, { empty_room: null });
      });
    }
    const [membership] = await this.repo.list('members', [eq('user_id', job.user_id)], 1);
    if (!membership) return this.advance(job);
    return this.repo.lock('room:' + membership.room_id, async () => {
      const room = await this.repo.get('rooms', membership.room_id);
      const members = await this.repo.list('members', [eq('room_id', membership.room_id)]);
      const peers = members.filter(member => member.user_id !== job.user_id).sort((a, b) =>
        Number(b.role === 'admin') - Number(a.role === 'admin') || a.user_id.localeCompare(b.user_id));
      if (room && !peers.length) {
        await this.repo.update('rooms', room.id, { ...room, visibility: 'private' });
        return this.save(job, { empty_room: room.id });
      }
      if (room) {
        if (membership.role === 'owner' && !peers.some(member => member.role === 'owner'))
          await this.repo.update('members', peers[0].id, { ...peers[0], role: 'owner' });
        if (room.created_by === job.user_id || room.kind === 'direct') await this.repo.update('rooms', room.id,
          { ...room, created_by: room.created_by === job.user_id ? peers[0].user_id : room.created_by,
            ...(room.kind === 'direct' ? { name: 'Deleted account', direct_key: null } : {}) });
      }
      await this.repo.remove('members', membership.id);
      return this.save(job, {});
    });
  }
  async postStep(job) {
    if (!job.post_id) {
      const [post] = await this.rawList('posts', [eq('authorId', job.user_id)], 1);
      return post ? this.save(job, { post_id: post.$id }) : this.advance(job);
    }
    const [comment] = await this.rawList('comments', [eq('postId', job.post_id)], 1);
    if (comment) { await this.removeComment(comment); return this.save(job, {}); }
    if (await this.relationBatch('likes', [eq('targetType', 'post'), eq('targetId', job.post_id)])) return this.save(job, {});
    if (await this.relationBatch('reposts', [eq('postId', job.post_id)])) return this.save(job, {});
    await this.rawRemove('post-feed', job.post_id);
    await this.rawRemove('posts', job.post_id);
    return this.save(job, { post_id: null });
  }
  async step(job) {
    const userId = job.user_id, userPath = '/users/' + enc(userId);
    switch (job.phase) {
      case 'disable':
        await this.missing(() => this.auth.rest.request('PATCH', userPath + '/status', { body: { status: false } }));
        return this.save(job, { phase: 'apple', cursor: null, not_before: this.now() + 65000 });
      case 'apple': {
        const [token] = job.apple_tokens || [];
        if (!token) return this.advance(job);
        const response = await this.fetch('https://appleid.apple.com/auth/revoke', { method: 'POST', redirect: 'error',
          headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
          signal: this.repo.rest?.isDeletionTransport ? this.repo.rest.signal() : AbortSignal.timeout(10000),
          body: new URLSearchParams({ client_id: this.appleClientId, client_secret: this.appleClientSecret,
            token: token.token, token_type_hint: token.hint }) });
        if (!response.ok) reject(503, 'apple_revocation_failed', 'Apple sign-in revocation needs a retry.');
        return this.save(job, { apple_tokens: job.apple_tokens.slice(1) });
      }
      case 'messages': return this.scan(job, 'messages', row => row.sender_id === userId,
        row => this.repo.lock('room:' + row.room_id, () => this.repo.remove('messages', row.id)));
      case 'attachments': {
        const [photo] = await this.repo.list('attachments', [eq('uploader_id', userId)], 1);
        if (!photo) return this.advance(job);
        await this.repo.lock('room:' + photo.room_id, () => this.removePhotos(photo));
        return this.save(job, {});
      }
      case 'invites': return this.scan(job, 'invites', row => row.created_by === userId, row => this.repo.remove('invites', row.id));
      case 'members': return this.memberStep(job);
      case 'rooms': return this.scan(job, 'rooms', row => row.created_by === userId, async row => {
        const [owner] = (await this.repo.list('members', [eq('room_id', row.id)])).filter(member => member.role === 'owner');
        await this.repo.update('rooms', row.id, { ...row, created_by: owner?.user_id || null });
      });
      case 'posts': return this.postStep(job);
      case 'comments': {
        const [comment] = await this.rawList('comments', [eq('authorId', userId)], 1);
        if (!comment) return this.advance(job);
        await this.removeComment(comment); return this.save(job, {});
      }
      case 'likes': case 'reposts':
        if (!await this.relationBatch(job.phase, [eq('userId', userId)])) return this.advance(job);
        return this.save(job, {});
      case 'following': case 'followers':
        if (!await this.relationBatch('follows', [eq(job.phase === 'following' ? 'followerId' : 'followingId', userId)])) return this.advance(job);
        return this.save(job, {});
      case 'feed':
        if (!await this.relationBatch('post-feed', [eq('authorId', userId)])) return this.advance(job);
        return this.save(job, {});
      case 'blocks': case 'blocked': {
        const rows = await this.repo.list('blocks', [eq(job.phase === 'blocks' ? 'blocker_id' : 'blocked_id', userId)], 25);
        if (!rows.length) return this.advance(job);
        for (const row of rows) await this.repo.remove('blocks', row.id);
        return this.save(job, {});
      }
      case 'reports': return this.scan(job, 'reports', row => row.reporter_id === userId || row.target_user === userId,
        row => this.repo.remove('reports', row.id));
      case 'files': {
        const path = `/storage/buckets/${enc(this.repo.bucketId)}/files`, filters = job.cursor ? [{ method: 'cursorAfter', values: [job.cursor] }] : [];
        const params = new URLSearchParams();
        [...filters, { method: 'limit', values: [25] }, order('$id')].forEach(query => params.append('queries[]', JSON.stringify(query)));
        let result;
        try { result = await this.auth.rest.request('GET', path + '?' + params); }
        catch (error) { if (job.cursor && error.status === 404) return this.save(job, { cursor: null }); throw error; }
        let cursor = job.cursor;
        for (const file of result.files) {
          // IDs in owner-writable profile/post fields are not deletion authority.
          // Storage ACLs establish ownership, including abandoned older uploads.
          if ((file.$permissions || []).includes(`delete("user:${userId}")`) || (file.$permissions || []).includes(`write("user:${userId}")`))
            await this.missing(() => this.auth.rest.request('DELETE', path + '/' + enc(file.$id)));
          else cursor = file.$id;
        }
        return result.files.length < 25 ? this.advance(job) : this.save(job, { cursor });
      }
      case 'state': return this.scan(job, 'state', row => row.kind !== 'lock' && (row.user_id === userId || row.id === key('link:' + userId)
        || ['discovery:' + userId, 'reports:' + userId, 'link:' + userId, 'link-target:' + job.email, 'link-target:' + job.phone].includes(row.name)), row => this.repo.remove('state', row.id));
      case 'profile':
        await this.repo.remove('directory', userId); await this.repo.remove('profiles', userId);
        return this.advance(job);
      case 'account':
        await this.missing(() => this.auth.rest.request('DELETE', userPath));
        await this.repo.remove('deletions', job.id); return null;
      default: reject(503, 'deletion_state', 'Account deletion needs administrator assistance.');
    }
  }
  async run(id, { maximumSteps = 8, milliseconds = 12000 } = {}) {
    if (this.repo.rest && !this.repo.rest.isDeletionTransport) {
      const rest = deletionTransport(this.repo.rest, { now: this.now, milliseconds });
      const repo = Object.assign(Object.create(this.repo), { rest });
      const worker = Object.assign(Object.create(this), { repo, auth: { rest } });
      return worker.run(id, { maximumSteps, milliseconds });
    }
    try {
      return await this.repo.lock('delete-account:' + id, async () => {
        const end = this.now() + milliseconds;
        let job = await this.repo.get('deletions', id), steps = 0;
        if (!job || job.kind !== 'account_deletion') return { processed: false };
        // Drain invocations authenticated before the account was blocked. The
        // recovery interval resumes after the 60-second invocation timeout.
        while (job && steps++ < maximumSteps && this.now() < end) {
          if (job.not_before && this.now() < job.not_before) break;
          job = await this.step(job);
        }
        return { processed: true, complete: !job };
      });
    } catch (error) {
      if (error.status === 409 && error.code === 'busy') return { processed: false, busy: true };
      throw error;
    }
  }
}

export async function recoverDeletions(repo, deletions, report = () => {}) {
  const id = 'account-deletion-sweep', previous = await repo.get('state', id);
  const filters = [eq('kind', 'account_deletion'), order('$id')];
  let jobs;
  try { jobs = await repo.list('deletions', previous?.cursor ? [...filters, { method: 'cursorAfter', values: [previous.cursor] }] : filters, 5); }
  catch (error) { if (error.status !== 404 || !previous?.cursor) throw error; }
  if (!jobs?.length) jobs = await repo.list('deletions', filters, 5);
  // Checkpoint the sweep before processing. Failed or killed jobs cannot keep
  // every later account behind the same first page on subsequent intervals.
  const cursor = { kind: 'deletion_sweep', cursor: jobs.at(-1)?.id || null };
  if (previous) await repo.update('state', id, cursor); else await repo.create('state', cursor, id);
  let processed = 0;
  for (const job of jobs) {
    try { await deletions.run(job.id, { maximumSteps: 1 }); processed++; }
    catch { report('Account deletion needs a retry.'); }
  }
  return { processed };
}
