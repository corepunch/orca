import { configuration, createRepository } from './repository.js';
import { execute } from './materialize.js';

try {
  if (process.argv[2] !== 'reconcile') throw new Error('Usage: node src/admin.js reconcile');
  const repo = createRepository(configuration(), process.env.APPWRITE_API_KEY);
  let processed = 0;
  let failures = 0;
  let reset = true;
  for (;;) {
    const result = await execute(repo, { trigger: 'admin', reset });
    if (result.body.busy) throw new Error('A writer is active; retry after it completes.');
    reset = false;
    processed += result.body.processed;
    failures += result.body.failed?.length || 0;
    if (result.body.cycleComplete) break;
  }
  if (failures) throw new Error(`Reconciliation completed with ${failures} failed post refreshes; fix the source/schema and repeat.`);
  console.log(JSON.stringify({ reconciled: true, processed }));
} catch (error) { console.error(error.message); process.exitCode = 1; }
