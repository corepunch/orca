import { readFile, mkdtemp, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { AppwriteRest } from '../../../chat/backend/appwrite/src/rest.js';

const run = promisify(execFile);
const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
const segment = encodeURIComponent;
const fields = ['name', 'execute', 'events', 'schedule', 'interval', 'timeout', 'enabled', 'logging', 'entrypoint', 'commands', 'scopes'];
// enabled=false only blocks ordinary callers; event and scheduled executions
// must be disconnected separately while a migration changes their storage.
export const paused = config => ({ ...config, enabled: false, execute: [], events: [], schedule: '', interval: 0 });

export async function sourceArchive(directory) {
  const temporary = await mkdtemp(join(tmpdir(), 'octopus-deploy-'));
  try {
    const archive = join(temporary, 'source.tar.gz');
    // Explicit allowlist keeps credentials, tests, local configuration and Git out.
    await run('tar', ['-czf', archive, '-C', directory, 'package.json', 'src']);
    return await readFile(archive);
  } finally { await rm(temporary, { recursive: true, force: true }); }
}

export async function configureFunction(rest, config, variables = {}) {
  const path = `/functions/${segment(config.$id)}`;
  let existing;
  try { existing = await rest.request('GET', path); }
  catch (error) { if (error.status !== 404) throw error; }
  const settings = Object.fromEntries(fields.filter(key => config[key] !== undefined).map(key => [key, config[key]]));
  if (existing && existing.runtime !== config.runtime) throw new Error(`Unexpected runtime for ${config.$id}.`);
  if (existing) await rest.request('PUT', path, { body: settings });
  else await rest.request('POST', '/functions', { body: { functionId: config.$id, runtime: config.runtime, ...settings } });
  const saved = await rest.request('GET', path + '/variables');
  for (const [key, value] of Object.entries(variables)) {
    const found = saved.variables.find(variable => variable.key === key);
    if (found && found.secret) throw new Error(`Refusing to replace secret variable ${key}.`);
    if (found && found.value === value) continue;
    const body = { key, value, secret: false };
    if (found) await rest.request('PUT', `${path}/variables/${segment(found.$id)}`, { body });
    else await rest.request('POST', path + '/variables', { body: { variableId: 'unique()', ...body } });
  }
  return settings;
}

export async function deployFunction(rest, config, directory, variables, {
  archive = sourceArchive, wait = pause, progress = console.log,
} = {}) {
  const staging = await configureFunction(rest, paused(config), variables);
  const settings = { ...staging, execute: config.execute || [], events: config.events || [], schedule: config.schedule || '', interval: config.interval || 0 };
  const path = `/functions/${segment(config.$id)}`;
  const multipart = new FormData();
  multipart.set('code', new Blob([await archive(directory)], { type: 'application/gzip' }), 'source.tar.gz');
  multipart.set('activate', 'false');
  multipart.set('entrypoint', config.entrypoint);
  multipart.set('commands', config.commands);
  const created = await rest.request('POST', path + '/deployments', { body: multipart });
  let deployment = created, previous;
  for (let attempt = 0; attempt < 180; attempt++) {
    if (deployment.status !== previous) progress(`${config.$id}: ${deployment.status}`);
    previous = deployment.status;
    if (deployment.status === 'ready') break;
    if (['failed', 'canceled'].includes(deployment.status)) throw new Error(`Deployment ${created.$id} failed; inspect Appwrite build logs.`);
    await wait(3000);
    deployment = await rest.request('GET', `${path}/deployments/${segment(created.$id)}`);
  }
  if (deployment.status !== 'ready') throw new Error(`Deployment ${created.$id} is still building; function remains disabled.`);
  await rest.request('PATCH', path + '/deployment', { body: { deploymentId: created.$id } });
  const active = await rest.request('GET', path);
  if (active.deploymentId !== created.$id) throw new Error(`Deployment activation failed for ${config.$id}.`);
  return { id: config.$id, deploymentId: created.$id, settings: { ...settings, enabled: true } };
}

export async function waitForIdle(rest, functionId, { wait = pause } = {}) {
  const params = new URLSearchParams();
  params.append('queries[]', JSON.stringify({ method: 'equal', attribute: 'status', values: ['waiting', 'processing'] }));
  params.append('queries[]', JSON.stringify({ method: 'limit', values: [1] }));
  for (let attempt = 0; attempt < 120; attempt++) {
    const result = await rest.request('GET', `/functions/${segment(functionId)}/executions?${params}`);
    if (result.executions.length === 0) return;
    await wait(1000);
  }
  throw new Error('Active executions did not drain; retry after they finish.');
}

export async function enableFunctions(rest, deployed) {
  try {
    for (const result of deployed) await rest.request('PUT', `/functions/${segment(result.id)}`, { body: result.settings });
  } catch (error) {
    const failures = [];
    for (const result of deployed) {
      try { await rest.request('PUT', `/functions/${segment(result.id)}`, { body: paused(result.settings) }); }
      catch { failures.push(result.id); }
    }
    if (failures.length) throw new Error(`Activation failed and these Functions could not be paused: ${failures.join(', ')}. Check their enabled status in Console.`);
    throw error;
  }
}

// Deployment is explicit. Do not run this module as part of a build or test.
if (process.argv[1] && import.meta.url === pathToFileURL(process.argv[1]).href) {
  try {
    const directory = resolve(fileURLToPath(new URL('..', import.meta.url)));
    const manifest = JSON.parse(await readFile(join(directory, 'appwrite.config.json'), 'utf8'));
    const key = process.env.APPWRITE_API_KEY;
    if (!key) throw new Error('APPWRITE_API_KEY is required; never put it in the source archive.');
    const endpoint = process.env.APPWRITE_ENDPOINT || manifest.endpoint;
    const projectId = process.env.APPWRITE_PROJECT_ID || manifest.projectId;
    if (projectId !== manifest.projectId) throw new Error('Deployment project differs from the manifest.');
    const rest = new AppwriteRest({ endpoint, projectId, apiKey: key });
    const result = await deployFunction(rest, manifest.functions[0], directory, {
      OCTOPUS_DATABASE_ID: process.env.OCTOPUS_DATABASE_ID || 'octopus-db',
      OCTOPUS_FEED_COLLECTION_ID: process.env.OCTOPUS_FEED_COLLECTION_ID || 'post-feed',
    });
    await rest.request('PUT', `/functions/${segment(result.id)}`, { body: result.settings });
    console.log(JSON.stringify({ deployed: true, projectId, functionId: result.id, deploymentId: result.deploymentId }));
  } catch (error) { console.error(error.message); process.exitCode = 1; }
}
