// Creates (or reconnects) the persistent live-test accounts and saves their
// credentials in a git-ignored file. With --clean and APPWRITE_API_KEY it also
// deletes every conversation those accounts belong to.
import { randomBytes } from 'node:crypto';
import { ChatClient } from './client.js';
import { Admin, NAMES, accountSpec, config, normalize, readAccounts, writeAccounts } from './world.js';

const cfg = config(), clean = process.argv.includes('--clean');
const saved = readAccounts(cfg) ?? { password: process.env.CHAT_LIVE_PASSWORD || randomBytes(18).toString('base64url'), accounts: {} };
console.log(`Appwrite ${cfg.endpoint}, project ${cfg.projectId}, function ${cfg.functionId}`);
console.log(`Accounts file ${cfg.accountsFile}`);

for (const name of NAMES) {
  const spec = saved.accounts[name] ??= accountSpec(cfg.prefix, name, cfg.domain);
  const password = spec.password || saved.password, client = new ChatClient(cfg, name);
  let state = 'session restored';
  if (!(spec.cookie && await client.restore(spec.cookie))) {
    try { await client.signIn(spec.email, password); state = 'signed in'; }
    catch (error) {
      if (error.status !== 401) throw error;
      try { await client.signUp(spec.email, password, spec.display_name); }
      catch (signup) {
        if (signup.status === 409) {
          throw new Error(`${spec.email} already exists with a different password. Put its password in ${cfg.accountsFile} `
            + `("password" or accounts.${name}.password), or choose another CHAT_LIVE_PREFIX.`);
        }
        throw signup;
      }
      await client.signIn(spec.email, password); state = 'created';
    }
  }
  spec.cookie = client.cookie; spec.id = client.id;
  writeAccounts(cfg, saved);
  Object.assign(client, { username: spec.username, displayName: spec.display_name });
  try { await normalize(client); }
  catch (error) {
    if (error.status === 409) throw new Error(`Username ${spec.username} belongs to someone else; choose another CHAT_LIVE_PREFIX.`);
    throw error;
  }
  console.log(`  ${name.padEnd(6)} ${spec.email.padEnd(36)} @${spec.username.padEnd(22)} ${spec.id}  (${state})`);
}

if (clean) {
  if (!cfg.apiKey) { console.error('--clean needs APPWRITE_API_KEY with documents.read/write and files.write scopes.'); process.exit(1); }
  const admin = new Admin(cfg), testers = NAMES.map(name => saved.accounts[name].id);
  let removed = 0, kept = 0;
  for (const roomId of await admin.roomsOf(testers)) {
    // Never touch a real person's conversation that merely includes a test account.
    const room = await admin.repo.get('rooms', roomId);
    if (room && !testers.includes(room.created_by)) { kept++; continue; }
    await admin.deleteRoom(roomId); removed++;
  }
  console.log(`Removed ${removed} conversation(s) created by the live-test accounts; kept ${kept} created by others.`);
}
