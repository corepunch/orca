// Runs the live suite against the in-memory emulator (offline harness check).
// It validates the suite and the Function source together; it says nothing
// about the deployed Function or real Appwrite behavior.
import { mkdtempSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { start } from './emulator.js';

const emulator = await start();
emulator.server.unref();
Object.assign(process.env, {
  CHAT_LIVE_ENDPOINT: emulator.endpoint, CHAT_LIVE_PROJECT_ID: emulator.projectId, CHAT_LIVE_FUNCTION_ID: emulator.functionId,
  APPWRITE_API_KEY: emulator.apiKey, CHAT_LIVE_ACCOUNTS: join(mkdtempSync(join(tmpdir(), 'chat-live-')), 'accounts.json'),
  // The Function entrypoint reads its runtime endpoint from these variables.
  APPWRITE_FUNCTION_API_ENDPOINT: emulator.endpoint, APPWRITE_FUNCTION_PROJECT_ID: emulator.projectId,
});
await import('./setup.js');
await import('./run.js');
