// Manual end-to-end suite for the deployed Chat backend. It is deliberately not
// part of `make test` or CI: it talks to a real Appwrite project with real accounts.
//   make test-live                      # all supported features
//   CHAT_LIVE_ONLY=discovery,direct make test-live
import { after } from 'node:test';
import { cleanup, config, world } from './world.js';

export const SUITES = ['auth', 'profiles', 'discovery', 'direct', 'groups', 'channels', 'invites', 'messages', 'photos', 'security', 'parity'];
const only = process.env.CHAT_LIVE_ONLY?.split(',').map(name => name.trim()).filter(Boolean);
const unknown = only?.filter(name => !SUITES.includes(name));
if (unknown?.length) { console.error(`Unknown suite(s): ${unknown.join(', ')}. Choose from: ${SUITES.join(', ')}`); process.exit(2); }

const cfg = config();
console.log(`# live Chat backend: ${cfg.endpoint} project=${cfg.projectId} function=${cfg.functionId}`
  + ` cleanup=${cfg.apiKey ? (cfg.keep ? 'kept' : 'server key') : 'none'}`);
try { await world(); }
catch (error) { console.error(`Live test setup failed: ${error.message}`); process.exit(1); }

after(cleanup);
// Deferred product ideas are probes, not acceptance checks. Run them explicitly
// with CHAT_LIVE_ONLY=parity; the default run covers the supported feature set.
for (const suite of SUITES) if (only ? only.includes(suite) : suite !== 'parity') await import(`./suites/${suite}.js`);
