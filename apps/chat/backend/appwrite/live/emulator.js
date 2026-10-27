// A small in-memory stand-in for the parts of Appwrite the Chat backend uses:
// Account sessions/JWTs, Users, Databases documents, Storage files and
// synchronous Function executions. It runs the real Function entrypoint
// (src/main.js) so `make test-live-emulated` can check the live suite itself
// without network access or a cloud account. It is not a faithful Appwrite.
import { createServer } from 'node:http';
import { randomBytes, randomUUID } from 'node:crypto';
import handler from '../src/main.js';

const UNIQUE = { chat_profiles: ['username'], chat_rooms: ['direct_key'], chat_attachments: ['object_key', 'thumbnail_key'] };

class Reply { constructor(status, body, headers = {}) { this.status = status; this.body = body; this.headers = headers; } }
const fail = (status, type, message) => { throw new Reply(status, { message, code: status, type }); };

export async function start({ projectId = 'emulated-project', functionId = 'orca-chat-api', databaseId = 'octopus-db', bucketId = 'post-images' } = {}) {
  const apiKey = 'emulator-' + randomBytes(16).toString('hex');
  const users = new Map(), sessions = new Map(), jwts = new Map(), collections = new Map(), files = new Map();
  let clock = 0;
  const stamp = () => new Date(clock = Math.max(Date.now(), clock + 1)).toISOString();
  const table = name => collections.get(name) ?? collections.set(name, new Map()).get(name);
  const userView = ({ password, ...user }) => structuredClone(user);
  const cookieName = 'a_session_' + projectId;

  function session(headers) {
    const match = (headers.cookie || '').match(new RegExp(cookieName + '=([^;]+)'));
    return match && sessions.get(match[1]);
  }
  function caller(headers) {
    if (headers['x-appwrite-jwt']) {
      const userId = jwts.get(headers['x-appwrite-jwt']);
      return userId && users.get(userId) || fail(401, 'user_jwt_invalid', 'Invalid JWT.');
    }
    const current = session(headers);
    return current && users.get(current.userId) || fail(401, 'general_unauthorized_scope', 'User (role: guests) missing scope (account).');
  }
  function queries(url) {
    const result = [];
    for (const [name, value] of url.searchParams) if (name.startsWith('queries')) result.push(JSON.parse(value));
    return result;
  }
  function unique(collection, doc) {
    for (const field of UNIQUE[collection] || []) {
      if (doc[field] == null) continue;
      for (const other of table(collection).values()) {
        if (other.$id !== doc.$id && other[field] === doc[field]) fail(409, 'document_already_exists', 'Document with the requested unique attributes already exists.');
      }
    }
  }

  async function account(method, path, headers, body) {
    if (method === 'POST' && path === '/account') {
      if (typeof body.password !== 'string' || body.password.length < 8) fail(400, 'general_argument_invalid', 'Password must be at least 8 characters.');
      if ([...users.values()].some(user => user.email === body.email.toLowerCase())) fail(409, 'user_already_exists', 'A user with the same id, email, or phone already exists.');
      const user = { $id: randomUUID().replace(/-/g, '').slice(0, 20), name: body.name || '', email: body.email.toLowerCase(), password: body.password,
        emailVerification: false, phone: '', phoneVerification: false, status: true };
      users.set(user.$id, user);
      return new Reply(201, userView(user));
    }
    if (method === 'POST' && path === '/account/sessions/email') {
      const user = [...users.values()].find(row => row.email === String(body.email).toLowerCase());
      if (!user || user.password !== body.password) fail(401, 'user_invalid_credentials', 'Invalid credentials.');
      const token = randomBytes(24).toString('hex'), id = randomUUID();
      sessions.set(token, { $id: id, userId: user.$id });
      return new Reply(201, { $id: id, userId: user.$id, secret: '', current: true },
        { 'set-cookie': [`${cookieName}_legacy=${token}; path=/; httponly`, `${cookieName}=${token}; path=/; httponly; secure`] });
    }
    if (method === 'GET' && path === '/account') return new Reply(200, userView(caller(headers)));
    const current = session(headers) || fail(401, 'general_unauthorized_scope', 'User (role: guests) missing scope (account).');
    if (method === 'POST' && path === '/account/jwts') {
      const jwt = 'jwt.' + randomBytes(24).toString('hex');
      jwts.set(jwt, current.userId);
      return new Reply(201, { jwt });
    }
    if (method === 'GET' && path === '/account/sessions') {
      const own = [...sessions.values()].filter(row => row.userId === current.userId);
      return new Reply(200, { total: own.length, sessions: own.map(row => ({ $id: row.$id, userId: row.userId, current: row === current })) });
    }
    if (method === 'DELETE' && path === '/account/sessions/current') {
      for (const [token, row] of sessions) if (row === current) sessions.delete(token);
      return new Reply(204, null);
    }
    fail(404, 'general_route_not_found', 'Route not found.');
  }

  async function execute(headers, body) {
    const user = caller(headers);
    let result;
    await handler({
      req: { headers: { 'x-appwrite-user-jwt': headers['x-appwrite-jwt'], 'x-appwrite-user-id': user.$id, 'x-appwrite-key': apiKey }, bodyText: body.body ?? '' },
      res: { json: value => { result = value; return value; } },
    });
    return new Reply(201, { $id: randomUUID(), status: 'completed', responseStatusCode: 200, responseBody: JSON.stringify(result), duration: 0.01, errors: '' });
  }

  function admin(method, url, body, raw, contentType) {
    const path = url.pathname.slice(3), parts = path.split('/').filter(Boolean).map(decodeURIComponent);
    if (parts[0] === 'users') {
      if (method === 'GET' && parts.length === 1) {
        const filters = queries(url);
        return new Reply(200, { users: [...users.values()].filter(user => filters.every(query => query.method !== 'equal' || query.values.includes(user[query.attribute]))).map(userView) });
      }
      const user = users.get(parts[1]) || fail(404, 'user_not_found', 'User with the requested ID could not be found.');
      if (method === 'GET' && parts.length === 2) return new Reply(200, userView(user));
      if (method === 'PATCH' && parts[2] === 'verification') {
        if (parts[3] === 'phone') user.phoneVerification = !!body.phoneVerification; else user.emailVerification = !!body.emailVerification;
        return new Reply(200, userView(user));
      }
      if (method === 'PATCH' && ['email', 'phone'].includes(parts[2])) { user[parts[2]] = body[parts[2]]; user[parts[2] + 'Verification'] = false; return new Reply(200, userView(user)); }
      if (method === 'DELETE' && parts[2] === 'targets') return new Reply(204, null);
    }
    if (parts[0] === 'databases' && parts[1] === databaseId && parts[2] === 'collections' && parts[4] === 'documents') {
      const name = parts[3], rows = table(name), docId = parts[5];
      if (method === 'POST' && !docId) {
        if (rows.has(body.documentId)) fail(409, 'document_already_exists', 'Document with the requested ID already exists.');
        const doc = { ...structuredClone(body.data), $id: body.documentId, $createdAt: stamp() };
        unique(name, doc); rows.set(doc.$id, doc);
        return new Reply(201, structuredClone(doc));
      }
      if (method === 'GET' && !docId) {
        let list = [...rows.values()];
        const filters = queries(url);
        for (const query of filters) if (query.method === 'equal') list = list.filter(doc => query.values.includes(doc[query.attribute]));
        const orders = filters.filter(query => query.method === 'orderAsc' || query.method === 'orderDesc');
        list.sort((a, b) => {
          for (const query of orders) {
            const order = String(a[query.attribute]).localeCompare(String(b[query.attribute]));
            if (order) return query.method === 'orderDesc' ? -order : order;
          }
          return 0;
        });
        const cursor = filters.find(query => query.method === 'cursorAfter')?.values[0];
        if (cursor) list = list.slice(list.findIndex(doc => doc.$id === cursor) + 1);
        const limit = filters.find(query => query.method === 'limit')?.values[0] ?? 25;
        return new Reply(200, { total: list.length, documents: structuredClone(list.slice(0, limit)) });
      }
      const doc = rows.get(docId) || fail(404, 'document_not_found', 'Document with the requested ID could not be found.');
      if (method === 'GET') return new Reply(200, structuredClone(doc));
      if (method === 'PATCH') {
        const next = { ...doc, ...structuredClone(body.data) };
        unique(name, next); rows.set(docId, next);
        return new Reply(200, structuredClone(next));
      }
      if (method === 'DELETE') { rows.delete(docId); return new Reply(204, null); }
    }
    if (parts[0] === 'storage' && parts[1] === 'buckets' && parts[2] === bucketId && parts[3] === 'files') {
      return storage(method, parts[4], parts[5], raw, contentType);
    }
    fail(404, 'general_route_not_found', 'Route not found.');
  }

  async function storage(method, fileId, action, raw, contentType) {
    if (method === 'POST' && !fileId) {
      const form = await new Response(raw, { headers: { 'content-type': contentType } }).formData();
      const id = form.get('fileId');
      if (files.has(id)) fail(409, 'storage_file_already_exists', 'A storage file with the requested ID already exists.');
      files.set(id, Buffer.from(await form.get('file').arrayBuffer()));
      return new Reply(201, { $id: id });
    }
    const bytes = files.get(fileId) || fail(404, 'storage_file_not_found', 'The requested file could not be found.');
    if (method === 'GET' && (action === 'download' || action === 'view')) return new Reply(200, Buffer.from(bytes));
    if (method === 'DELETE' && !action) { files.delete(fileId); return new Reply(204, null); }
    fail(404, 'general_route_not_found', 'Route not found.');
  }

  async function route(req, raw) {
    const url = new URL(req.url, 'http://emulator'), method = req.method, headers = req.headers;
    if (!url.pathname.startsWith('/v1/')) fail(404, 'general_route_not_found', 'Route not found.');
    if (headers['x-appwrite-project'] !== projectId) fail(404, 'project_not_found', 'Project not found.');
    const json = () => raw.length && /json/.test(headers['content-type'] || '') ? JSON.parse(raw.toString('utf8')) : {};
    const path = url.pathname.slice(3);
    if (path.startsWith('/account')) return account(method, path, headers, json());
    if (method === 'POST' && path === `/functions/${functionId}/executions`) return execute(headers, json());
    // Public reads: Octopus profile photos and the public-read photo bucket.
    if (method === 'GET' && path === `/databases/${databaseId}/collections/profiles/documents`) return new Reply(200, { total: 0, documents: [] });
    if (method === 'GET' && new RegExp(`^/storage/buckets/${bucketId}/files/[^/]+/view$`).test(path)) return storage('GET', path.split('/')[4], 'view');
    if (headers['x-appwrite-key'] !== apiKey) fail(401, 'user_unauthorized', 'The current user is not authorized to perform the requested action.');
    return admin(method, url, json(), raw, headers['content-type']);
  }

  const server = createServer(async (req, res) => {
    const chunks = [];
    for await (const chunk of req) chunks.push(chunk);
    let reply;
    try { reply = await route(req, Buffer.concat(chunks)); }
    catch (error) { reply = error instanceof Reply ? error : new Reply(500, { message: String(error?.stack || error), code: 500, type: 'general_unknown' }); }
    const binary = Buffer.isBuffer(reply.body);
    res.writeHead(reply.status, { ...reply.headers, ...(reply.body === null ? {} : { 'content-type': binary ? 'application/octet-stream' : 'application/json' }) });
    res.end(reply.body === null ? undefined : binary ? reply.body : JSON.stringify(reply.body));
  });
  await new Promise(resolve => server.listen(0, '127.0.0.1', resolve));
  const endpoint = `http://127.0.0.1:${server.address().port}/v1`;
  return { endpoint, projectId, functionId, databaseId, bucketId, apiKey, server, close: () => new Promise(resolve => server.close(resolve)) };
}
