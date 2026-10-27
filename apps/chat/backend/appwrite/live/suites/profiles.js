import { after, before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { rejects } from '../fixtures.js';
import { normalize, world } from '../world.js';

describe('profiles: names, usernames and private identifiers', () => {
  let w;
  before(async () => { w = await world(); });
  after(async () => { await normalize(w.carol); });

  it('the self profile has the documented shape', async () => {
    const profile = await w.carol.getProfile();
    for (const field of ['id', 'display_name', 'username', 'discoverable_email', 'discoverable_phone']) assert.ok(field in profile, field);
    assert.equal(profile.id, w.carol.id);
    assert.equal(profile.discoverable_email, false);
    assert.equal(profile.discoverable_phone, false);
  });

  it('an unverified login email is not exposed, even to its owner', async () => {
    const account = await w.carol.account('GET', '/account');
    const profile = await w.carol.getProfile();
    if (!account.emailVerification) assert.equal(profile.email ?? null, null);
    else assert.equal(profile.email, account.email.toLowerCase());
  });

  it('display names round-trip Unicode and are trimmed', async () => {
    const name = '  Кэрол 🐙 کارول  ';
    const updated = await w.carol.updateProfile({ display_name: name });
    assert.equal(updated.display_name, name.trim());
    assert.equal((await w.carol.getProfile()).display_name, name.trim());
  });

  it('a renamed person appears renamed to others', async () => {
    const room = w.track(await w.alice.createRoom(w.name('rename visibility'), [w.carol.id]));
    await w.carol.updateProfile({ display_name: 'Carol Renamed ' + w.run });
    const members = await w.alice.listMembers(room.id);
    assert.equal(members.find(member => member.user_id === w.carol.id).display_name, 'Carol Renamed ' + w.run);
    const contacts = await w.alice.listContacts();
    assert.equal(contacts.find(contact => contact.id === w.carol.id)?.display_name, 'Carol Renamed ' + w.run);
  });

  it('display name accepts 80 characters and rejects empty or 81', async () => {
    assert.equal((await w.carol.updateProfile({ display_name: 'ё'.repeat(80) })).display_name, 'ё'.repeat(80));
    await rejects(w.carol.updateProfile({ display_name: 'ё'.repeat(81) }), 400, 'validation');
    await rejects(w.carol.updateProfile({ display_name: '   ' }), 400, 'validation');
  });

  it('usernames validate format', async () => {
    for (const bad of ['Upper_case', 'ab', '1starts_with_digit', 'has-dash', 'has space', 'a'.repeat(33), '_underscore']) {
      await rejects(w.carol.updateProfile({ username: bad }), 400, 'validation');
    }
    const longest = ('c' + w.run + '_'.repeat(32)).slice(0, 32);
    assert.equal((await w.carol.updateProfile({ username: longest })).username, longest);
  });

  it('usernames are unique across accounts', async () => {
    await rejects(w.carol.updateProfile({ username: w.alice.username }), 409);
    assert.equal((await w.alice.getProfile()).username, w.alice.username);
  });

  it('a username can be cleared and set again', async () => {
    assert.equal((await w.carol.updateProfile({ username: '' })).username, null);
    assert.equal((await w.carol.updateProfile({ username: w.carol.username })).username, w.carol.username);
  });

  it('a released username can be claimed by someone else', async () => {
    const shared = ('rel_' + w.run).slice(0, 32);
    await w.carol.updateProfile({ username: shared });
    await w.carol.updateProfile({ username: '' });
    assert.equal((await w.dave.updateProfile({ username: shared })).username, shared);
    await normalize(w.dave);
  });

  it('profiles cannot edit login identifiers or unknown fields', async () => {
    await rejects(w.carol.updateProfile({ email: 'someone@example.com' }), 400, 'validation');
    await rejects(w.carol.updateProfile({ phone: '+15555550123' }), 400, 'validation');
    await rejects(w.carol.updateProfile({ id: w.alice.id }), 400, 'validation');
  });

  it('discovery by email or phone needs a verified identifier', async () => {
    const account = await w.carol.account('GET', '/account');
    if (!account.emailVerification) await rejects(w.carol.updateProfile({ discoverable_email: true }), 400, 'validation');
    if (!account.phoneVerification) await rejects(w.carol.updateProfile({ discoverable_phone: true }), 400, 'validation');
    await rejects(w.carol.updateProfile({ discoverable_email: 'yes' }), 400, 'validation');
  });

  it('identity linking reports missing delivery configuration or rate limits clearly', async () => {
    // Without Messaging providers this must be a clear configuration error, never a silent success.
    try {
      const started = await w.carol.requestIdentityLink(`${w.cfg.prefix}.carol.link.${w.run}@${w.cfg.domain}`);
      assert.equal(started.delivery, 'email');
    } catch (error) {
      assert.ok([503, 429].includes(error.status), error.message);
      assert.ok(['configuration', 'rate_limit'].includes(error.code), error.message);
    }
  });

  it('public profile photos resolve anonymously for chat members', async () => {
    const avatars = await w.alice.getAvatars([w.alice.id, w.bob.id]);
    assert.deepEqual(Object.keys(avatars).sort(), [w.alice.id, w.bob.id].sort());
    for (const value of Object.values(avatars)) assert.ok(value === false || typeof value === 'string');
  });
});
