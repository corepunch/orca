import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtemp, mkdir, writeFile, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { execFile } from 'node:child_process';
import { promisify } from 'node:util';
import { configureFunction, deployFunction, sourceArchive, waitForIdle, enableFunctions } from '../src/deploy.js';

const config = { $id: 'feed', name: 'Feed', runtime: 'node-22', enabled: true, execute: [],
  events: ['databases.*.collections.posts.documents.*.create'], schedule: '', interval: 60, timeout: 600,
  entrypoint: 'src/main.js', commands: 'npm install', scopes: ['documents.read'] };

function service(statuses = ['ready']) {
  const calls = [], variables = [{ $id: 'private', key: 'OTP_KEY', value: '', secret: true }];
  let active;
  return { calls, variables, async request(method, path, options = {}) {
    calls.push({ method, path, body: options.body });
    if (method === 'GET' && path === '/functions/feed') return { runtime: 'node-22', deploymentId: active };
    if (method === 'GET' && path.endsWith('/variables')) return { variables };
    if (method === 'POST' && path.endsWith('/deployments')) return { $id: 'new-build', status: statuses[0] };
    if (method === 'GET' && path.includes('/deployments/')) return { $id: 'new-build', status: statuses.shift() || 'ready' };
    if (method === 'PATCH' && path.endsWith('/deployment')) active = options.body.deploymentId;
    return {};
  } };
}

test('deployment preserves secret variables and keeps traffic paused until ready activation', async () => {
  const rest = service(['building', 'ready']);
  const deployed = await deployFunction(rest, config, '.', { DATABASE: 'shared' }, {
    archive: async () => Buffer.from('archive'), wait: async () => {}, progress() {},
  });
  assert.equal(deployed.deploymentId, 'new-build');
  assert.equal(deployed.settings.enabled, true);
  const updates = rest.calls.filter(call => call.method === 'PUT' && call.path === '/functions/feed');
  assert.equal(updates.length, 1);
  assert.equal(updates[0].body.enabled, false);
  assert.deepEqual(updates[0].body.events, []);
  assert.equal(updates[0].body.schedule, '');
  assert.equal(updates[0].body.interval, 0);
  assert.deepEqual(deployed.settings.events, config.events);
  assert.equal(deployed.settings.schedule, '');
  assert.equal(deployed.settings.interval, 60);
  assert(!rest.calls.some(call => call.path.endsWith('/variables/private') && call.method !== 'GET'));
  const upload = rest.calls.find(call => call.method === 'POST' && call.path.endsWith('/deployments'));
  assert.equal(upload.body.get('activate'), 'false');
  assert.equal(upload.body.get('entrypoint'), 'src/main.js');
  assert(rest.calls.findIndex(call => call.method === 'PATCH') > rest.calls.findIndex(call => call.path.includes('/deployments/')));
  await enableFunctions(rest, [deployed]);
  const activation = rest.calls.filter(call => call.method === 'PUT' && call.path === '/functions/feed').at(-1);
  assert.equal(activation.body.enabled, true);
  assert.equal(activation.body.schedule, '');
  assert.equal(activation.body.interval, 60);
});

test('failed builds never activate or reenable the function', async () => {
  const rest = service(['failed']);
  await assert.rejects(deployFunction(rest, config, '.', {}, {
    archive: async () => Buffer.from('archive'), progress() {},
  }), /failed/);
  assert(!rest.calls.some(call => call.method === 'PATCH'));
  assert(rest.calls.filter(call => call.method === 'PUT').every(call => call.body.enabled === false && call.body.interval === 0));
});

test('configuration refuses to overwrite a saved secret and waits for active executions to drain', async () => {
  const rest = service();
  await assert.rejects(configureFunction(rest, config, { OTP_KEY: 'replacement' }), /secret variable/);
  let checks = 0, pauses = 0;
  await waitForIdle({ async request() { return { executions: ++checks === 1 ? [{ $id: 'busy' }] : [] }; } }, 'feed', {
    wait: async () => { pauses++; },
  });
  assert.equal(checks, 2);
  assert.equal(pauses, 1);
});

test('source archive excludes credential files, fixtures and repository metadata', async () => {
  const directory = await mkdtemp(join(tmpdir(), 'octopus-archive-test-'));
  try {
    await mkdir(join(directory, 'src'));
    for (const [path, value] of [['package.json', '{}'], ['src/main.js', 'export default () => {}'],
      ['.env', 'API_KEY=private'], ['fixture.json', 'private']]) await writeFile(join(directory, path), value);
    const archive = await sourceArchive(directory);
    const file = join(directory, 'archive.tar.gz');
    await writeFile(file, archive);
    const { stdout } = await promisify(execFile)('tar', ['-tzf', file]);
    assert(stdout.includes('package.json') && stdout.includes('src/main.js'));
    assert(!stdout.includes('.env') && !stdout.includes('fixture.json'));
  } finally { await rm(directory, { recursive: true, force: true }); }
});

test('partial traffic activation pauses both Functions again', async () => {
  const enabled = { chat: false, feed: false };
  const calls = [], intervals = { chat: 0, feed: 60 };
  const rest = { async request(method, path, { body }) {
    calls.push(body);
    const id = path.split('/').at(-1);
    if (id === 'feed' && body.enabled) throw new Error('Activation unavailable');
    enabled[id] = body.enabled;
    intervals[id] = body.interval;
  } };
  await assert.rejects(enableFunctions(rest, Object.keys(enabled).map(id => ({ id, settings: { enabled: true, schedule: '', interval: id === 'feed' ? 60 : 0 } }))), /Activation unavailable/);
  assert.deepEqual(enabled, { chat: false, feed: false });
  assert.deepEqual(intervals, { chat: 0, feed: 0 });
  assert(calls.filter(body => !body.enabled).every(body => body.schedule === '' && body.interval === 0 && body.events.length === 0 && body.execute.length === 0));
});


test('cron deployment explicitly disables interval scheduling', async () => {
  const rest = service();
  const deployed = await deployFunction(rest, { ...config, schedule: '*/5 * * * *', interval: 0 }, '.', {}, {
    archive: async () => Buffer.from('archive'), progress() {},
  });
  const staging = rest.calls.find(call => call.method === 'PUT' && call.path === '/functions/feed');
  assert.equal(staging.body.interval, 0);
  assert.equal(staging.body.schedule, '');
  assert.equal(deployed.settings.interval, 0);
  assert.equal(deployed.settings.schedule, '*/5 * * * *');
});
