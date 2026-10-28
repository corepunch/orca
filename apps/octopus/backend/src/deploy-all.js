import { readFile } from 'node:fs/promises';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { AppwriteRest } from '../../../chat/backend/appwrite/src/rest.js';
import { migrateShared } from '../../../chat/backend/appwrite/src/migrate.js';
import { configuration, createRepository } from './repository.js';
import { migrate } from './migrate.js';
import { execute } from './materialize.js';
import { configureFunction, deployFunction, waitForIdle, enableFunctions, paused } from './deploy.js';

const feedDirectory = fileURLToPath(new URL('..', import.meta.url));
const chatDirectory = fileURLToPath(new URL('../../../chat/backend/appwrite/', import.meta.url));
const manifest = async directory => JSON.parse(await readFile(directory + '/appwrite.config.json', 'utf8'));

export async function deployAll({ env = process.env, dropLegacy = false, progress = console.log } = {}) {
  const [feed, chat] = await Promise.all([manifest(feedDirectory), manifest(chatDirectory)]);
  if (!env.APPWRITE_API_KEY) throw new Error('APPWRITE_API_KEY is required.');
  const endpoint = env.APPWRITE_ENDPOINT || feed.endpoint;
  const projectId = env.APPWRITE_PROJECT_ID || feed.projectId;
  const databaseId = env.OCTOPUS_DATABASE_ID || 'octopus-db';
  if (projectId !== feed.projectId || projectId !== chat.projectId || databaseId !== 'octopus-db')
    throw new Error('Deployment target differs from the shared manifests.');
  const rest = new AppwriteRest({ endpoint, projectId, apiKey: env.APPWRITE_API_KEY });
  const repo = createRepository(configuration({ ...env, APPWRITE_ENDPOINT: endpoint, APPWRITE_PROJECT_ID: projectId }), env.APPWRITE_API_KEY);
  const chatConfig = chat.functions[0], feedConfig = feed.functions[0];
  progress(`Deploying shared Octopus backend to ${projectId} (${endpoint}).`);
  // Disable writers before copying, then leave them disabled if any step fails.
  // A rerun safely resumes schema provisioning and verified legacy copies.
  for (const config of [chatConfig, feedConfig]) {
    await configureFunction(rest, paused(config));
    await waitForIdle(rest, config.$id);
  }
  progress('Provisioning shared profiles and messaging collections.');
  const shared = await migrateShared(rest, { databaseId, migrateLegacy: true, dropLegacy: false, quiesced: true });
  progress('Provisioning feed summaries and indexes.');
  await migrate(repo);
  progress('Backfilling public feed summaries.');
  let reset = true, processed = 0;
  for (;;) {
    const result = await execute(repo, { trigger: 'admin', reset });
    reset = false;
    if (result.body.busy) throw new Error('A feed writer is still active; wait or follow documented lock recovery.');
    if (result.body.failed?.length) throw new Error(`Feed backfill failed for ${result.body.failed.length} posts; fix source/schema and rerun.`);
    processed += result.body.processed || 0;
    progress(`Feed backfill: ${processed} source/cache rows checked.`);
    if (result.body.cycleComplete) break;
  }
  await migrateShared(rest, { databaseId, verify: true });
  await migrate(repo, { verify: true });
  const deployed = [];
  deployed.push(await deployFunction(rest, chatConfig, chatDirectory, {
    CHAT_DATABASE_ID: databaseId, CHAT_BUCKET_ID: env.OCTOPUS_BUCKET_ID || 'post-images',
  }, { progress }));
  deployed.push(await deployFunction(rest, feedConfig, feedDirectory, {
    OCTOPUS_DATABASE_ID: databaseId, OCTOPUS_FEED_COLLECTION_ID: repo.config.feedId,
    OCTOPUS_FEED_CONTROL_COLLECTION_ID: repo.config.controlId,
  }, { progress }));
  for (const result of deployed) await waitForIdle(rest, result.id);
  // Cleanup happens only after both replacement deployments are ready and active,
  // while new executions are still disabled, with a second verified copy pass.
  if (dropLegacy) {
    progress('Removing verified legacy collections.');
    await migrateShared(rest, { databaseId, migrateLegacy: true, dropLegacy: true, quiesced: true });
  }
  await enableFunctions(rest, deployed);
  return { deployed: true, projectId, databaseId, shared, processed, functions: deployed.map(({ id, deploymentId }) => ({ id, deploymentId })) };
}

if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try { console.log(JSON.stringify(await deployAll({ dropLegacy: process.argv.includes('--drop-legacy') }))); }
  catch (error) { console.error(error.message); process.exitCode = 1; }
}
