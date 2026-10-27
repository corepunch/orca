// Shared live-test world: configuration, the persistent test accounts and their
// saved sessions, per-run naming, and optional administrative cleanup.
import { chmodSync, existsSync, readFileSync, writeFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { AppwriteRest, Repository, eq } from '../src/rest.js';
import { ChatClient } from './client.js';

const here = dirname(fileURLToPath(import.meta.url));
export const NAMES = ['alice', 'bob', 'carol', 'dave'];

export function config(env = process.env) {
  const prefix = env.CHAT_LIVE_PREFIX || 'orca_live';
  if (!/^[a-z][a-z0-9_]{2,20}$/.test(prefix)) throw new Error('CHAT_LIVE_PREFIX must be 3–21 lowercase letters, digits or underscores.');
  return {
    endpoint: (env.CHAT_LIVE_ENDPOINT || env.APPWRITE_ENDPOINT || 'https://fra.cloud.appwrite.io/v1').replace(/\/$/, ''),
    projectId: env.CHAT_LIVE_PROJECT_ID || env.APPWRITE_PROJECT_ID || '69f1c06800389dc6a1a0',
    functionId: env.CHAT_LIVE_FUNCTION_ID || env.APPWRITE_CHAT_FUNCTION_ID || 'orca-chat-api',
    databaseId: env.CHAT_DATABASE_ID || 'octopus-db',
    bucketId: env.CHAT_BUCKET_ID || 'post-images',
    // Optional server key for cleanup and verified-identifier tests; never bundled in the app.
    apiKey: env.APPWRITE_API_KEY || '',
    // Outside the Function directory so a deployment archive can never include it.
    accountsFile: env.CHAT_LIVE_ACCOUNTS || join(here, '..', '..', '.live-accounts.json'),
    domain: env.CHAT_LIVE_EMAIL_DOMAIN || 'example.com',
    keep: env.CHAT_LIVE_KEEP === '1',
    prefix,
  };
}

export function accountSpec(prefix, name, domain) {
  return { email: `${prefix}.${name}@${domain}`, username: `${prefix}_${name}`,
    display_name: 'Live ' + name[0].toUpperCase() + name.slice(1) };
}

export function readAccounts(cfg) {
  if (!existsSync(cfg.accountsFile)) return null;
  return JSON.parse(readFileSync(cfg.accountsFile, 'utf8'));
}
export function writeAccounts(cfg, data) {
  writeFileSync(cfg.accountsFile, JSON.stringify(data, null, 2) + '\n', { mode: 0o600 });
  chmodSync(cfg.accountsFile, 0o600);
}

// Server-key operations used only by the test harness, never by the app.
export class Admin {
  constructor(cfg) {
    this.rest = new AppwriteRest({ endpoint: cfg.endpoint, projectId: cfg.projectId, apiKey: cfg.apiKey });
    this.repo = new Repository(this.rest, cfg.databaseId, cfg.bucketId);
  }
  setEmailVerified(userId, verified) {
    return this.rest.request('PATCH', `/users/${encodeURIComponent(userId)}/verification`, { body: { emailVerification: verified } });
  }
  // Removes a conversation and everything that references it, including encrypted photo objects.
  async deleteRoom(roomId) {
    for (const photo of await this.repo.list('attachments', [eq('room_id', roomId)])) {
      await this.repo.removeFile(photo.object_key); await this.repo.removeFile(photo.thumbnail_key);
      await this.repo.remove('attachments', photo.id);
    }
    for (const collection of ['messages', 'invites', 'members']) {
      for (const row of await this.repo.list(collection, [eq('room_id', roomId)])) await this.repo.remove(collection, row.id);
    }
    await this.repo.remove('rooms', roomId);
  }
  async roomsOf(userIds) {
    const rooms = new Set();
    for (const userId of userIds) for (const member of await this.repo.list('members', [eq('user_id', userId)])) rooms.add(member.room_id);
    return [...rooms];
  }
}

let shared;
// One signed-in world per process. Sessions are restored from the accounts file so
// a run does not spend Appwrite's per-email sign-in allowance.
export function world() { return shared ??= open(); }

async function open() {
  const cfg = config(), saved = readAccounts(cfg);
  if (!saved?.password || !saved.accounts) {
    throw new Error(`No live test accounts in ${cfg.accountsFile}. Run \`make test-live-setup\` first (see apps/chat/backend/appwrite/live/README.md).`);
  }
  const run = Date.now().toString(36), users = {};
  let changed = false;
  for (const name of NAMES) {
    const spec = saved.accounts[name];
    if (!spec) throw new Error(`Account "${name}" is missing from ${cfg.accountsFile}; rerun \`make test-live-setup\`.`);
    const client = new ChatClient(cfg, name);
    if (!(spec.cookie && await client.restore(spec.cookie))) {
      await client.signIn(spec.email, spec.password || saved.password);
      spec.cookie = client.cookie; changed = true;
    }
    if (spec.id !== client.id) { spec.id = client.id; changed = true; }
    Object.assign(client, { email: spec.email, password: spec.password || saved.password, username: spec.username, displayName: spec.display_name });
    users[name] = client;
  }
  if (changed) writeAccounts(cfg, saved);
  // An interrupted earlier run may have left a renamed profile or enabled discovery.
  for (const client of Object.values(users)) await normalize(client);
  const rooms = new Set();
  return {
    cfg, run, users, ...users, saved,
    admin: cfg.apiKey ? new Admin(cfg) : null,
    // Unique, recognizable names keep runs apart in the shared test accounts.
    name: label => `live ${run} ${label}`.slice(0, 80),
    track(room) { if (room?.id && room.kind !== 'direct') rooms.add(room.id); return room; },
    rooms,
    persist() { writeAccounts(cfg, saved); },
  };
}

export async function normalize(client) {
  const profile = await client.getProfile(), wanted = {};
  if (profile.username !== client.username) wanted.username = client.username;
  if (profile.display_name !== client.displayName) wanted.display_name = client.displayName;
  if (profile.discoverable_email) wanted.discoverable_email = false;
  if (profile.discoverable_phone) wanted.discoverable_phone = false;
  if (Object.keys(wanted).length) await client.updateProfile(wanted);
}

// Deletes this run's conversations when a server key is available.
export async function cleanup() {
  if (!shared) return;
  const w = await shared.catch(() => null);
  if (!w) return;
  for (const client of Object.values(w.users)) await normalize(client).catch(() => {});
  if (!w.admin) {
    console.log(`# ${w.rooms.size} conversation(s) from run ${w.run} remain in the test accounts; set APPWRITE_API_KEY to remove them.`);
    return;
  }
  if (w.cfg.keep) { console.log(`# CHAT_LIVE_KEEP=1: kept ${w.rooms.size} conversation(s) from run ${w.run}.`); return; }
  let removed = 0;
  for (const roomId of w.rooms) {
    try { await w.admin.deleteRoom(roomId); removed++; }
    catch (error) { console.log(`# cleanup of ${roomId} failed: ${error.message}`); }
  }
  console.log(`# removed ${removed} conversation(s) from run ${w.run}.`);
}
