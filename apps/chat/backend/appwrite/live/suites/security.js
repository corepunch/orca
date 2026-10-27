import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('security: the Function is the only door', () => {
  let w;
  before(async () => { w = await world(); });

  it('Chat collections are closed to direct client access', async () => {
    for (const collection of ['chat_messages', 'chat_rooms', 'chat_profiles', 'chat_members', 'chat_attachments', 'chat_invites', 'chat_state']) {
      const response = await w.alice.api.call('GET', `/databases/${w.cfg.databaseId}/collections/${collection}/documents`,
        { headers: { Cookie: w.alice.cookie } });
      assert.ok([401, 403, 404].includes(response.status), `${collection} answered ${response.status}`);
    }
  });

  it('only the fixed route allowlist is reachable', async () => {
    for (const [method, path] of [
      ['GET', '/databases/octopus-db/collections/chat_messages/documents'],
      ['GET', '/rest/v1/chat_state'],
      ['POST', '/rest/v1/rpc/not_a_real_rpc'],
      ['GET', '/rest/v1/rpc/get_chat_profile'],
      ['GET', '/rest/v1/../v1/chat_state'],
      ['POST', '/users'],
    ]) {
      const result = await w.alice.send(method, path, {});
      assert.equal(result.status, 404, `${method} ${path} → ${result.status}`);
    }
  });

  it('malformed envelopes are rejected', async () => {
    for (const envelope of [{ method: 'PATCH', path: '/rest/v1/chat_rooms' }, { method: 'GET', path: 'rest/v1/chat_rooms' }, { method: 'GET' }]) {
      const { outer, result } = await w.alice.execute(envelope);
      assert.equal(outer.status, 201, 'Appwrite accepted the execution');
      assert.equal(result.status, 400, JSON.stringify(envelope));
    }
  });

  it('error bodies never include stack traces or server keys', async () => {
    const result = await w.alice.send('POST', '/rest/v1/rpc/create_chat_room', { room_name: 1, member_ids: 'x' });
    assert.equal(result.status, 400);
    assert.deepEqual(Object.keys(result.body).sort(), ['code', 'message']);
    assert.ok(!/at .*\.js:\d+|x-appwrite-key|standard_/i.test(JSON.stringify(result.body)));
  });

  it('identifiers in routes are validated before lookups', async () => {
    await rejects(w.alice.listMembers('../rooms'), 400, 'validation');
    await rejects(w.alice.removeMember('x'.repeat(37), w.bob.id), 400, 'validation');
    await rejects(w.alice.call('GET', '/storage/v1/object/authenticated/chat-photos/%E0%A4%A'), 400, 'validation');
  });

  it('discovery is rate limited per account (runs last; Dave waits a minute afterwards)', async () => {
    let limited = false;
    for (let attempt = 0; attempt < 45 && !limited; attempt++) {
      const result = await w.dave.send('POST', '/rest/v1/rpc/find_chat_people', { query: 'zz_nobody_' + attempt });
      if (result.status === 429) { limited = true; assert.equal(result.body.code, 'rate_limit'); }
      else assert.equal(result.status, 200);
    }
    assert.ok(limited, 'no 429 after 45 lookups (20 per clock minute are allowed)');
  });
});
