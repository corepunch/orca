const segment = encodeURIComponent;
export const query = (method, attribute, values) => JSON.stringify({ method,
  ...(attribute ? { attribute } : {}), ...(values ? { values } : {}) });

export function configuration(env = process.env) {
  const config = {
    endpoint: env.APPWRITE_ENDPOINT || env.APPWRITE_FUNCTION_API_ENDPOINT,
    projectId: env.APPWRITE_PROJECT_ID || env.APPWRITE_FUNCTION_PROJECT_ID,
    databaseId: env.OCTOPUS_DATABASE_ID || 'octopus-db',
    feedId: env.OCTOPUS_FEED_COLLECTION_ID || 'post-feed',
    controlId: env.OCTOPUS_FEED_CONTROL_COLLECTION_ID || 'post-feed-control',
    batchSize: Number(env.OCTOPUS_FEED_RECONCILE_BATCH_SIZE || 10),
  };
  if (!config.endpoint || !config.projectId) throw new Error('Appwrite endpoint and project are required.');
  if (!Number.isInteger(config.batchSize) || config.batchSize < 1 || config.batchSize > 100)
    throw new Error('Reconciliation batch size must be between 1 and 100.');
  return config;
}

export function createRepository(config, key, fetcher = fetch) {
  if (!key) throw new Error('A server API key is required.');
  const base = `/databases/${segment(config.databaseId)}/collections/`;
  const collectionPath = id => base + segment(id);
  async function request(method, path, body, queries = [], privileged = true) {
    const url = new URL(config.endpoint.replace(/\/$/, '') + path);
    for (const q of queries) url.searchParams.append('queries[]', q);
    const response = await fetcher(url, { method,
      headers: { 'X-Appwrite-Project': config.projectId, 'Content-Type': 'application/json',
        ...(privileged ? { 'X-Appwrite-Key': key } : {}) },
      ...(body ? { body: JSON.stringify(body) } : {}), signal: AbortSignal.timeout(15000) });
    const data = response.status === 204 ? {} : await response.json();
    if (!response.ok) {
      // Do not include request bodies, server messages or headers in Function logs.
      const error = new Error(`Appwrite ${method} failed (${response.status}).`);
      error.status = response.status;
      throw error;
    }
    return data;
  }
  const get = async (collection, id, privileged = true) => {
    try { return await request('GET', `${collectionPath(collection)}/documents/${segment(id)}`, undefined, [], privileged); }
    catch (error) {
      if (error.status === 404 || (!privileged && [401, 403].includes(error.status))) return null;
      throw error;
    }
  };
  async function remove(collection, id) {
    try { await request('DELETE', `${collectionPath(collection)}/documents/${segment(id)}`); }
    catch (error) { if (error.status !== 404) throw error; }
  }
  async function put(collection, id, data, permissions = []) {
    try {
      return await request('PATCH', `${collectionPath(collection)}/documents/${segment(id)}`, { data, permissions });
    } catch (error) {
      if (error.status !== 404) throw error;
      return request('POST', `${collectionPath(collection)}/documents`, { documentId: id, data, permissions });
    }
  }
  return {
    config, request, collectionPath, get,
    collection: id => request('GET', collectionPath(id)),
    list: (collection, queries, privileged = true) => request('GET', `${collectionPath(collection)}/documents`, undefined, queries, privileged),
    remove, put,
    async countPublic(collection, filters) {
      // Guest reads count only publicly visible reactions/comments. Return one ID,
      // never every comment body or every reaction document.
      let data;
      try { data = await this.list(collection, [...filters, query('limit', null, [1]), query('select', null, ['$id'])], false); }
      catch (error) { if ([401, 403].includes(error.status)) return 0; throw error; }
      if (!Number.isSafeInteger(data.total) || data.total < 0) throw new Error('Appwrite omitted an exact total.');
      return data.total;
    },
    async acquire() {
      try {
        await request('POST', `${collectionPath(config.controlId)}/documents`, {
          documentId: 'writer-lock', data: { value: JSON.stringify({ startedAt: new Date().toISOString() }) }, permissions: [] });
        return true;
      } catch (error) { if (error.status === 409) return false; throw error; }
    },
    release: () => remove(config.controlId, 'writer-lock'),
    async checkpoint(state) { await put(config.controlId, 'reconcile-cursor', { value: JSON.stringify(state) }); },
    async loadCheckpoint() {
      const doc = await get(config.controlId, 'reconcile-cursor');
      return doc ? JSON.parse(doc.value) : { phase: 'posts', cursor: null };
    },
  };
}
