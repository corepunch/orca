import { createHash, randomUUID } from 'node:crypto';

export class Failure extends Error {
  constructor(status, code, message) { super(message); this.status = status; this.code = code; }
}
export const reject = (status, code, message) => { throw new Failure(status, code, message); };
export const key = value => createHash('sha256').update(value).digest('hex').slice(0, 36);
export const eq = (attribute, value) => ({ method: 'equal', attribute, values: [value] });
export const order = (attribute, descending = false) => ({ method: descending ? 'orderDesc' : 'orderAsc', attribute });
const enc = encodeURIComponent;

export class AppwriteRest {
  constructor({ endpoint, projectId, apiKey, fetch: fetcher = fetch }) {
    this.endpoint = endpoint.replace(/\/$/, ''); this.projectId = projectId; this.apiKey = apiKey; this.fetch = fetcher;
  }
  async request(method, path, { body, jwt, admin = true, binary = false, signal } = {}) {
    const headers = { 'X-Appwrite-Project': this.projectId };
    if (admin) headers['X-Appwrite-Key'] = this.apiKey;
    if (jwt) headers['X-Appwrite-JWT'] = jwt;
    const multipart = body instanceof FormData;
    if (body !== undefined && !multipart) headers['Content-Type'] = 'application/json';
    const response = await this.fetch(this.endpoint + path, {
      method, headers, body: body === undefined ? undefined : multipart ? body : JSON.stringify(body),
      signal: signal || AbortSignal.timeout(20000), redirect: 'error',
    });
    if (!response.ok) {
      let error = {}; try { error = await response.json(); } catch { /* Non-JSON upstream failure. */ }
      throw new Failure(response.status, error.type || 'appwrite_error', error.message || 'Appwrite rejected the request.');
    }
    if (binary) return Buffer.from(await response.arrayBuffer());
    if (response.status === 204) return true;
    const source = await response.text(); return source ? JSON.parse(source) : true;
  }
}

function attributes(collection, row) {
  switch (collection) {
    case 'directory': return { handle: row.handle || null, email: row.email || null, phone: row.phone || null };
    case 'rooms': return { kind: row.kind, visibility: row.visibility, direct_key: row.direct_key || null, name_search: row.name.toLowerCase() };
    case 'members': return { room_id: row.room_id, user_id: row.user_id };
    case 'messages': return { room_id: row.room_id, created_at: row.created_at };
    case 'attachments': return { room_id: row.room_id, uploader_id: row.uploader_id, object_key: row.object_key, thumbnail_key: row.thumbnail_key };
    case 'invites': return { room_id: row.room_id };
    case 'state': return { kind: row.kind };
    default: throw new Error('Unknown collection');
  }
}
const unpack = doc => ({ ...JSON.parse(doc.payload), id: doc.$id });

// Logical service names are an internal API; storage belongs to the shared app.
// Only public display metadata lives in profiles. Identifier discovery and
// preferences stay in an administrator-only collection.
export const COLLECTIONS = Object.freeze({ profiles: 'profiles', directory: 'account-directory',
  rooms: 'conversations', members: 'conversation-members', messages: 'messages',
  attachments: 'attachments', invites: 'invitations', state: 'service-state' });
export const privateIdentity = row => ({ email: row.email || null, phone: row.phone || null,
  discoverable_email: !!row.discoverable_email, discoverable_phone: !!row.discoverable_phone });

export const directoryIdentity = row => ({ ...privateIdentity(row), handle: row.username ?? row.handle ?? null });

export class Repository {
  constructor(rest, databaseId, bucketId) { this.rest = rest; this.databaseId = databaseId; this.bucketId = bucketId; }
  path(collection, id) {
    if (!COLLECTIONS[collection]) throw new Error('Unknown collection');
    return `/databases/${enc(this.databaseId)}/collections/${COLLECTIONS[collection]}/documents${id ? '/' + enc(id) : ''}`;
  }
  async document(collection, id) {
    try { return await this.rest.request('GET', this.path(collection, id)); }
    catch (error) { if (error.status === 404) return null; throw error; }
  }
  async get(collection, id) {
    if (collection !== 'profiles') {
      const doc = await this.document(collection, id);
      return doc ? unpack(doc) : null;
    }
    const [profile, directory] = await Promise.all([this.document('profiles', id), this.document('directory', id)]);
    if (!profile && !directory) return null;
    return { ...privateIdentity(directory ? unpack(directory) : {}), id,
      display_name: profile?.username || '', username: directory?.handle || null,
      avatar_id: profile?.avatarId || null };
  }
  async saveProfile(id, row) {
    // Claim the handle through the private unique index before publishing a label.
    // Public profiles are owner writable and cannot establish identity authority.
    const identity = directoryIdentity(row), existing = await this.document('directory', id);
    try {
      if (!existing) await this.create('directory', identity, id);
      else if (JSON.stringify(directoryIdentity(unpack(existing))) !== JSON.stringify(identity))
        await this.update('directory', id, identity);
    } catch (error) {
      if (error.status === 409) reject(409, 'handle_taken', 'This account name is already in use. Choose another.');
      throw error;
    }
    const previous = await this.document('profiles', id), data = { username: row.display_name || '' };
    if (previous) {
      if (previous.username !== data.username)
        await this.rest.request('PATCH', this.path('profiles', id), { body: { data } });
    } else {
      await this.rest.request('POST', this.path('profiles'), { body: { documentId: id,
        data: { ...data, userId: id, bio: '' }, permissions: ['read("any")', `update("user:${id}")`] } });
    }
    return { ...privateIdentity(identity), id, display_name: data.username, username: identity.handle,
      avatar_id: previous?.avatarId || null };
  }
  async create(collection, row, id = randomUUID()) {
    if (collection === 'profiles') return this.saveProfile(id, row);
    return unpack(await this.rest.request('POST', this.path(collection), {
      body: { documentId: id, data: { ...attributes(collection, row), payload: JSON.stringify(row) }, permissions: [] },
    }));
  }
  async update(collection, id, row) {
    if (collection === 'profiles') return this.saveProfile(id, row);
    return unpack(await this.rest.request('PATCH', this.path(collection, id), {
      body: { data: { ...attributes(collection, row), payload: JSON.stringify(row) } },
    }));
  }
  async remove(collection, id) {
    try { await this.rest.request('DELETE', this.path(collection, id)); return true; }
    catch (error) { if (error.status === 404) return false; throw error; }
  }
  async list(collection, queries = [], limit, options = {}) {
    if (collection === 'profiles') {
      const filters = queries.map(query => ({ ...query,
        ...(query.attribute === 'username' ? { attribute: 'handle' } : {}) }));
      const docs = await this.listDocuments('directory', filters, limit, options);
      return (await Promise.all(docs.map(doc => this.get('profiles', doc.$id)))).filter(Boolean);
    }
    return (await this.listDocuments(collection, queries, limit, options)).map(unpack);
  }
  async listDocuments(collection, queries = [], limit, options = {}) {
    const rows = []; let cursor = queries.find(query => query.method === 'cursorAfter')?.values?.[0];
    queries = queries.filter(query => query.method !== 'cursorAfter');
    // Cursor pagination prevents Appwrite's default 25-row page from hiding members.
    // The explicit cap fails the request rather than returning a partial directory.
    for (let page = 0; page < 100; page++) {
      const count = limit ? Math.min(100, limit - rows.length) : 100;
      const filters = [...queries, { method: 'limit', values: [count] }];
      if (collection === 'messages') filters.push({ method: 'select', values: ['$id', 'payload'] });
      if (cursor) filters.push({ method: 'cursorAfter', values: [cursor] });
      const params = new URLSearchParams({ total: 'false' }); filters.forEach(query => params.append('queries[]', JSON.stringify(query)));
      const result = await this.rest.request('GET', this.path(collection) + '?' + params, options);
      rows.push(...result.documents);
      if (result.documents.length < count || (limit && rows.length === limit)) return rows;
      cursor = result.documents.at(-1).$id;
    }
    reject(503, 'directory_too_large', 'This directory is too large to load safely. Contact the administrator.');
  }
  async lock(name, action) {
    const id = key('lock:' + name), token = randomUUID();
    try { await this.create('state', { kind: 'lock', name, token, acquired_at: new Date().toISOString() }, id); }
    catch (error) { if (error.status === 409) reject(409, 'busy', 'Another operation is in progress. Please retry.'); throw error; }
    try { return await action(); }
    finally {
      // Never steal or automatically expire locks: a slow worker must not overlap a new one.
      const held = await this.get('state', id);
      if (held?.token === token) await this.remove('state', id);
    }
  }
  fileId(objectKey) { return key('photo:' + objectKey); }
  async putFile(objectKey, bytes) {
    const data = new FormData(); data.set('fileId', this.fileId(objectKey));
    data.set('file', new Blob([bytes], { type: 'image/jpeg' }), this.fileId(objectKey) + '.jpg');
    return this.rest.request('POST', `/storage/buckets/${enc(this.bucketId)}/files`, { body: data });
  }
  getFile(objectKey) { return this.rest.request('GET', `/storage/buckets/${enc(this.bucketId)}/files/${this.fileId(objectKey)}/download`, { binary: true }); }
  async removeFile(objectKey) {
    try { await this.rest.request('DELETE', `/storage/buckets/${enc(this.bucketId)}/files/${this.fileId(objectKey)}`); }
    catch (error) { if (error.status !== 404) throw error; }
  }
}
