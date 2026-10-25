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
  async request(method, path, { body, jwt, admin = true, binary = false } = {}) {
    const headers = { 'X-Appwrite-Project': this.projectId };
    if (admin) headers['X-Appwrite-Key'] = this.apiKey;
    if (jwt) headers['X-Appwrite-JWT'] = jwt;
    const multipart = body instanceof FormData;
    if (body !== undefined && !multipart) headers['Content-Type'] = 'application/json';
    const response = await this.fetch(this.endpoint + path, {
      method, headers, body: body === undefined ? undefined : multipart ? body : JSON.stringify(body),
      signal: AbortSignal.timeout(20000), redirect: 'error',
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
    case 'profiles': return { username: row.username || null, email: row.email || null, phone: row.phone || null };
    case 'rooms': return { kind: row.kind, visibility: row.visibility, direct_key: row.direct_key || null, name_search: row.name.toLowerCase() };
    case 'members': return { room_id: row.room_id, user_id: row.user_id };
    case 'messages': return { room_id: row.room_id };
    case 'attachments': return { room_id: row.room_id, uploader_id: row.uploader_id, object_key: row.object_key, thumbnail_key: row.thumbnail_key };
    case 'invites': return { room_id: row.room_id };
    case 'state': return { kind: row.kind };
    default: throw new Error('Unknown collection');
  }
}
const unpack = doc => ({ ...JSON.parse(doc.payload), id: doc.$id });

export class Repository {
  constructor(rest, databaseId, bucketId) { this.rest = rest; this.databaseId = databaseId; this.bucketId = bucketId; }
  path(collection, id) {
    return `/databases/${enc(this.databaseId)}/collections/chat_${collection}/documents${id ? '/' + enc(id) : ''}`;
  }
  async get(collection, id) {
    try { return unpack(await this.rest.request('GET', this.path(collection, id))); }
    catch (error) { if (error.status === 404) return null; throw error; }
  }
  async create(collection, row, id = randomUUID()) {
    return unpack(await this.rest.request('POST', this.path(collection), {
      body: { documentId: id, data: { ...attributes(collection, row), payload: JSON.stringify(row) }, permissions: [] },
    }));
  }
  async update(collection, id, row) {
    return unpack(await this.rest.request('PATCH', this.path(collection, id), {
      body: { data: { ...attributes(collection, row), payload: JSON.stringify(row) } },
    }));
  }
  async remove(collection, id) {
    try { await this.rest.request('DELETE', this.path(collection, id)); return true; }
    catch (error) { if (error.status === 404) return false; throw error; }
  }
  async list(collection, queries = [], limit) {
    const rows = []; let cursor;
    // Cursor pagination prevents Appwrite's default 25-row page from hiding members.
    // The explicit cap fails the request rather than returning a partial directory.
    for (let page = 0; page < 100; page++) {
      const count = limit ? Math.min(100, limit - rows.length) : 100;
      const filters = [...queries, { method: 'limit', values: [count] }];
      if (cursor) filters.push({ method: 'cursorAfter', values: [cursor] });
      const params = new URLSearchParams(); filters.forEach(query => params.append('queries[]', JSON.stringify(query)));
      const result = await this.rest.request('GET', this.path(collection) + '?' + params);
      rows.push(...result.documents.map(unpack));
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
