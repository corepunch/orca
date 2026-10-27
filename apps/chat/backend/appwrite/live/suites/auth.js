import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { ChatClient } from '../client.js';
import { rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('auth: native Appwrite Account and Function identity', () => {
  let w;
  before(async () => { w = await world(); });

  it('restores every saved session and identifies the account', async () => {
    for (const client of Object.values(w.users)) {
      const account = await client.account('GET', '/account');
      assert.equal(account.$id, client.id);
      assert.equal(account.email, client.email);
    }
  });

  it('the Function resolves the caller from the user JWT', async () => {
    const profile = await w.alice.getProfile();
    assert.equal(profile.id, w.alice.id);
    assert.equal(profile.username, w.alice.username);
  });

  it('a freshly issued JWT works and replaces the previous one', async () => {
    const previous = w.bob.jwt;
    await w.bob.refresh();
    assert.notEqual(w.bob.jwt, previous);
    assert.equal((await w.bob.getProfile()).id, w.bob.id);
  });

  it('rejects a wrong password without creating a session', async () => {
    const stranger = new ChatClient(w.cfg, 'wrong-password');
    await rejects(stranger.signIn(w.dave.email, w.dave.password + '-wrong'), 401);
    assert.equal(stranger.cookie, null);
  });

  it('lists active sessions, including the current one', async () => {
    const sessions = await w.alice.account('GET', '/account/sessions');
    assert.ok(sessions.total >= 1);
    assert.ok(sessions.sessions.some(session => session.current), 'the current session is listed');
  });

  it('signing out revokes that session only', async () => {
    const second = new ChatClient(w.cfg, 'carol-second-device');
    await second.signIn(w.carol.email, w.carol.password);
    assert.equal((await second.getProfile()).id, w.carol.id);
    const cookie = second.cookie;
    await second.signOut();
    const revoked = new ChatClient(w.cfg, 'revoked');
    assert.equal(await revoked.restore(cookie), false, 'a signed-out cookie no longer authenticates');
    assert.equal((await w.carol.getProfile()).id, w.carol.id, 'the other device stays signed in');
  });

  it('anonymous executions are refused by Appwrite or by the Function', async () => {
    const { outer, result } = await w.alice.execute({ method: 'POST', path: '/rest/v1/rpc/get_chat_profile', body: {} }, { jwt: null });
    assert.ok(outer.status === 401 || result?.status === 401, `expected 401, got outer ${outer.status} / inner ${result?.status}`);
  });

  it('a forged JWT is refused', async () => {
    const { outer, result } = await w.alice.execute({ method: 'POST', path: '/rest/v1/rpc/get_chat_profile', body: {} }, { jwt: 'not.a.jwt' });
    assert.ok(outer.status === 401 || result?.status === 401, `expected 401, got outer ${outer.status} / inner ${result?.status}`);
  });

  it('request JSON cannot choose the caller', async () => {
    const result = await w.alice.send('POST', '/rest/v1/rpc/get_chat_profile', { user_id: w.bob.id, jwt: w.bob.jwt, session: w.bob.jwt });
    assert.equal(result.status, 200);
    assert.equal(result.body.id, w.alice.id);
  });

  it('sign-in OTP verification is not proxied by the Function', async () => {
    await rejects(w.alice.call('POST', '/auth/v1/verify', { type: 'email', email: w.alice.email, token: '123456' }), 404, 'not_found');
  });
});
