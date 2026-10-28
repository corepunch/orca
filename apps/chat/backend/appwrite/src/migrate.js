// Operator-only migration. Never imported by the deployed request handler.
import { readFile } from 'node:fs/promises';
import { pathToFileURL } from 'node:url';
import { AppwriteRest, Failure, directoryIdentity } from './rest.js';

export const LEGACY = Object.freeze({ chat_profiles: 'account-directory', chat_rooms: 'conversations',
  chat_members: 'conversation-members', chat_messages: 'messages', chat_attachments: 'attachments',
  chat_invites: 'invitations', chat_state: 'service-state' });
const enc = encodeURIComponent;
const canonical = value => JSON.stringify(value);
const fail = message => { throw new Failure(409, 'migration_conflict', message); };
const missing = async action => { try { return await action(); } catch (error) { if (error.status === 404) return null; throw error; } };
function decode(doc) {
  let row; try { row = JSON.parse(doc.payload); } catch { fail(`Invalid legacy payload: ${doc.$id}.`); }
  if (!row || typeof row !== 'object' || Array.isArray(row)) fail(`Invalid legacy row: ${doc.$id}.`);
  return row;
}
const schema = JSON.parse(await readFile(new URL('../schema.json', import.meta.url), 'utf8'));

export async function migrateShared(rest, { databaseId = schema.databaseId, verify = false, migrateLegacy = false,
  dropLegacy = false, quiesced = false, wait = ms => new Promise(resolve => setTimeout(resolve, ms)) } = {}) {
  if (dropLegacy && (!migrateLegacy || verify)) fail('Dropping legacy collections requires a verified copy in the same writable run.');
  if ((migrateLegacy || dropLegacy) && !quiesced) fail('Disable the old Chat function and drain its executions before copying legacy data.');
  const base = `/databases/${enc(databaseId)}/collections`, path = id => `${base}/${enc(id)}`;
  const get = (id, doc) => missing(() => rest.request('GET', path(id) + (doc ? '/documents/' + enc(doc) : '')));
  async function ready(id, kind, key) {
    for (let attempt = 0; attempt < 60; attempt++) {
      const item = await rest.request('GET', path(id) + `/${kind}/${enc(key)}`);
      if (item.status === 'available') return item;
      if (['failed', 'stuck'].includes(item.status)) fail(`Schema build failed: ${id}/${key}.`);
      await wait(1000);
    }
    fail(`Schema build timed out: ${id}/${key}.`);
  }
  async function provision(spec, publicProfile = false) {
    let existing = await get(spec.id);
    if (!existing) {
      if (verify || publicProfile) fail(`Required collection is absent: ${spec.id}.`);
      existing = await rest.request('POST', base, { body: { collectionId: spec.id, name: spec.name,
        permissions: [], documentSecurity: false, enabled: true } });
    }
    if (existing.enabled === false) fail(`Collection is disabled: ${spec.id}.`);
    if (!publicProfile && (existing.documentSecurity !== false || (existing.$permissions || []).length))
      fail(`Private collection permissions differ: ${spec.id}.`);
    if (publicProfile && (existing.attributes || []).some(attr => ['payload', 'email', 'phone', 'discoverable_email', 'discoverable_phone'].includes(attr.key)))
      fail('Public profiles contains private identity attributes; resolve its visibility before migration.');
    for (const attribute of spec.attributes) {
      const found = (existing.attributes || []).find(item => item.key === attribute.key);
      if (found && (found.type !== attribute.type || found.size !== attribute.size || !!found.required !== !!attribute.required || !!found.array))
        fail(`Attribute differs: ${spec.id}/${attribute.key}.`);
      if (!found) {
        if (verify) fail(`Attribute absent: ${spec.id}/${attribute.key}.`);
        const { type, ...body } = attribute;
        await rest.request('POST', path(spec.id) + '/attributes/' + type, { body: { ...body, array: false } });
      }
      if (!found || found.status !== 'available') {
        if (verify) fail(`Attribute unavailable: ${spec.id}/${attribute.key}.`);
        await ready(spec.id, 'attributes', attribute.key);
      }
    }
    existing = await get(spec.id);
    for (const index of spec.indexes) {
      const same = item => item.type === index.type && canonical(item.attributes) === canonical(index.attributes) &&
        index.attributes.every((_, position) => (item.orders?.[position] || 'ASC') === (index.orders?.[position] || 'ASC'));
      const found = (existing.indexes || []).find(item => same(item));
      const named = (existing.indexes || []).find(item => item.key === index.key);
      if (!found && named) fail(`Index differs: ${spec.id}/${index.key}.`);
      if (!found) {
        if (verify) fail(`Index absent: ${spec.id}/${index.key}.`);
        await rest.request('POST', path(spec.id) + '/indexes', { body: index });
      }
      if (!found || found.status !== 'available') {
        if (verify) fail(`Index unavailable: ${spec.id}/${index.key}.`);
        await ready(spec.id, 'indexes', found?.key || index.key);
      }
    }
  }
  async function documents(id) {
    const docs = []; let cursor;
    for (;;) {
      const queries = [{ method: 'limit', values: [100] }, { method: 'orderAsc', attribute: '$id' }];
      if (cursor) queries.push({ method: 'cursorAfter', values: [cursor] });
      const params = new URLSearchParams(); queries.forEach(query => params.append('queries[]', JSON.stringify(query)));
      const result = await rest.request('GET', path(id) + '/documents?' + params);
      docs.push(...result.documents);
      if (result.documents.length < 100) return docs;
      cursor = result.documents.at(-1).$id;
    }
  }
  function dataOf(doc, spec) {
    const data = {}, payload = decode(doc);
    if (spec.id === 'messages') {
      const time = payload.created_at;
      if (typeof time !== 'string' || !Number.isFinite(Date.parse(time)) || new Date(time).toISOString() !== time)
        fail(`Message source time is invalid: ${doc.$id}.`);
      doc = { ...doc, created_at: time };
    }
    for (const attr of spec.attributes) {
      if (attr.required && doc[attr.key] == null) fail(`Required source attribute absent: ${spec.id}/${attr.key}.`);
      data[attr.key] = doc[attr.key] ?? null;
    }
    return data;
  }
  async function copy(target, id, data, permissions = []) {
    const current = await get(target, id);
    const same = current && Object.entries(data).every(([key, value]) => (current[key] ?? null) === value);
    if (current && !same) fail(`Destination document differs: ${target}/${id}.`);
    if (!current) {
      if (verify) fail(`Destination document absent: ${target}/${id}.`);
      await rest.request('POST', path(target) + '/documents', { body: { documentId: id, data, permissions } });
    }
    const saved = await get(target, id);
    if (!saved || !Object.entries(data).every(([key, value]) => (saved[key] ?? null) === value)) fail(`Copy verification failed: ${target}/${id}.`);
  }
  await provision(schema.publicProfile, true);
  for (const spec of schema.collections) await provision(spec);
  const report = { verified: true, collections: {} };
  if (!migrateLegacy) return report;
  const sources = new Map();
  for (const [source, target] of Object.entries(LEGACY)) {
    if (!await get(source)) continue;
    sources.set(source, await documents(source));
    report.collections[source] = { target, count: sources.get(source).length };
  }
  for (const doc of sources.get('chat_state') || [])
    if (decode(doc).kind === 'lock') fail('A legacy lock remains. Inspect it after draining executions before retrying.');
  // Validate all legacy identities before writing any user data. Existing public
  // labels remain authoritative. Handles come only from trusted legacy Chat data;
  // owner-writable public mirrors are ignored.
  const profiles = [];
  for (const doc of sources.get('chat_profiles') || []) {
    const row = decode(doc), allowed = ['id', 'display_name', 'username', 'email', 'phone', 'discoverable_email', 'discoverable_phone'];
    if (Object.keys(row).some(key => !allowed.includes(key))) fail(`Unrecognized private profile fields: ${doc.$id}.`);
    const handle = row.username || null, publicDoc = await get('profiles', doc.$id);
    if (publicDoc && publicDoc.userId !== doc.$id) fail(`Public profile owner differs: ${doc.$id}.`);
    if (handle && !/^[a-z0-9][a-z0-9._-]{0,35}$/.test(handle)) fail(`Legacy handle requires explicit resolution: ${doc.$id}.`);
    if (handle && profiles.some(profile => profile.handle === handle)) fail('Duplicate legacy handles require explicit resolution.');
    if (handle) {
      const params = new URLSearchParams(); params.append('queries[]', JSON.stringify({ method: 'equal', attribute: 'handle', values: [handle] }));
      params.append('queries[]', JSON.stringify({ method: 'limit', values: [2] }));
      const owners = await rest.request('GET', path('account-directory') + '/documents?' + params);
      if (owners.documents.some(owner => owner.$id !== doc.$id)) fail(`Legacy handle is already claimed: ${doc.$id}.`);
      const account = await missing(() => rest.request('GET', '/users/' + enc(handle)));
      if (account && account.$id !== doc.$id) fail(`Legacy handle overlaps another account ID: ${doc.$id}.`);
    }
    profiles.push({ id: doc.$id, row, handle, publicDoc });
  }
  for (const { id, row, publicDoc } of profiles) {
    const identity = directoryIdentity(row);
    await copy('account-directory', id, { payload: JSON.stringify(identity), handle: identity.handle,
      email: identity.email, phone: identity.phone });
    if (!publicDoc) {
      const account = await rest.request('GET', '/users/' + enc(id));
      await copy('profiles', id, { userId: id, username: row.display_name || account.name || '', bio: '' },
        ['read("any")', `update("user:${id}")`]);
    }
  }
  for (const [source, docs] of sources) {
    if (source === 'chat_profiles') continue;
    const target = LEGACY[source], spec = schema.collections.find(item => item.id === target);
    for (const doc of docs) {
      const data = dataOf(doc, spec);
      if (source === 'chat_state' && decode(doc).kind === 'lock') fail('A legacy lock remains. Inspect it after draining executions before retrying.');
      await copy(target, doc.$id, data);
    }
  }
  if (dropLegacy) {
    // Re-read every source before any deletion, even when the operator asserted quiescence.
    for (const [source, before] of sources) {
      const after = await documents(source);
      if (JSON.stringify(before) !== JSON.stringify(after)) fail(`Legacy source changed during migration: ${source}.`);
    }
    for (const source of sources.keys()) await rest.request('DELETE', path(source));
    report.droppedLegacy = [...sources.keys()];
  }
  return report;
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  const flags = new Set(process.argv.slice(2));
  const valid = ['--verify', '--migrate-legacy', '--drop-legacy', '--quiesced'];
  if ([...flags].some(flag => !valid.includes(flag))) throw new Error('Unknown migration flag.');
  if (!process.env.APPWRITE_API_KEY) throw new Error('APPWRITE_API_KEY is required; do not place credentials in arguments.');
  const rest = new AppwriteRest({ endpoint: process.env.APPWRITE_ENDPOINT || 'https://fra.cloud.appwrite.io/v1',
    projectId: process.env.APPWRITE_PROJECT_ID || schema.projectId, apiKey: process.env.APPWRITE_API_KEY });
  try { console.log(JSON.stringify(await migrateShared(rest, { databaseId: process.env.CHAT_DATABASE_ID || schema.databaseId,
    verify: flags.has('--verify'), migrateLegacy: flags.has('--migrate-legacy'),
    dropLegacy: flags.has('--drop-legacy'), quiesced: flags.has('--quiesced') }))); }
  catch (error) { console.error(error.message); process.exitCode = 1; }
}
