import { after, before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { normalize, world } from '../world.js';

const PUBLIC_FIELDS = ['display_name', 'id', 'username'];

describe('discovery: finding a friend and starting to talk', () => {
  let w;
  before(async () => { w = await world(); });
  after(async () => { await normalize(w.bob); await normalize(w.dave); });

  it('finds a friend by exact username and returns only public fields', async () => {
    const found = await w.alice.findPeople(w.bob.username);
    assert.equal(found.length, 1, `expected @${w.bob.username}, got ${JSON.stringify(found)}`);
    assert.equal(found[0].id, w.bob.id);
    assert.equal(found[0].username, w.bob.username);
    assert.deepEqual(Object.keys(found[0]).sort(), PUBLIC_FIELDS);
  });

  it('accepts @username, uppercase and surrounding spaces like the search box does', async () => {
    for (const query of ['@' + w.bob.username, w.bob.username.toUpperCase(), `  ${w.bob.username}  `, '@' + w.bob.username.toUpperCase()]) {
      const found = await w.alice.findPeople(query);
      assert.deepEqual(found.map(person => person.id), [w.bob.id], `query ${JSON.stringify(query)}`);
    }
  });

  it('the server itself normalizes case and @ for older clients', async () => {
    const found = await w.alice.rpc('find_chat_people', { query: ' @' + w.bob.username.toUpperCase() + ' ' });
    assert.deepEqual(found.map(person => person.id), [w.bob.id]);
  });

  it('every test account can find every other one', async () => {
    for (const seeker of [w.bob, w.carol, w.dave]) {
      for (const target of [w.alice, w.bob, w.carol, w.dave]) {
        if (target === seeker) continue;
        const found = await seeker.findPeople('@' + target.username);
        assert.deepEqual(found.map(person => person.id), [target.id], `${seeker.label} → @${target.username}`);
      }
    }
  });

  it('searching for yourself returns nothing', async () => {
    assert.deepEqual(await w.alice.findPeople(w.alice.username), []);
  });

  it('an unknown username returns an empty list, not an error', async () => {
    assert.deepEqual(await w.alice.findPeople('zz_nobody_' + w.run), []);
  });

  it('an unverified or undiscoverable email is indistinguishable from an unknown one', async () => {
    assert.deepEqual(await w.alice.findPeople(w.bob.email), []);
    assert.deepEqual(await w.alice.findPeople(`nobody.${w.run}@${w.cfg.domain}`), []);
  });

  it('a new username is discoverable immediately and the old one stops matching', async () => {
    const renamed = ('bob_' + w.run).slice(0, 32);
    await w.bob.updateProfile({ username: renamed });
    assert.deepEqual((await w.alice.findPeople(renamed)).map(person => person.id), [w.bob.id]);
    assert.deepEqual(await w.alice.findPeople(w.bob.username), []);
    await w.bob.updateProfile({ username: w.bob.username });
    assert.deepEqual((await w.alice.findPeople(w.bob.username)).map(person => person.id), [w.bob.id]);
  });

  it('clearing a username hides the person from username search', async () => {
    await w.dave.updateProfile({ username: '' });
    assert.deepEqual(await w.alice.findPeople(w.dave.username), []);
    await w.dave.updateProfile({ username: w.dave.username });
  });

  it('end to end: find a friend, open a direct chat, exchange messages both ways', async () => {
    const [friend] = await w.carol.findPeople('@' + w.dave.username);
    assert.equal(friend?.id, w.dave.id, 'Carol finds Dave by username');
    const room = await w.carol.createRoom(friend.display_name, [friend.id], { kind: 'direct' });
    assert.equal(room.kind, 'direct');
    assert.equal(room.can_post, true);

    const hello = `hi Dave, it's Carol (${w.run})`;
    const sent = await w.carol.sendMessage(room.id, hello);
    assert.equal(sent.sender_id, w.carol.id);

    const inbox = await w.dave.listRooms();
    const seen = inbox.find(row => row.id === room.id);
    assert.ok(seen, 'the conversation appears in Dave\'s inbox');
    assert.equal(seen.kind, 'direct');
    assert.equal(seen.can_post, true);
    const received = await w.dave.listMessages(room.id);
    assert.ok(received.some(message => message.id === sent.id && message.body === hello), 'Dave reads Carol\'s message');

    const reply = await w.dave.sendMessage(room.id, `hey Carol (${w.run})`);
    const timeline = await w.carol.listMessages(room.id);
    assert.equal(timeline.at(-1).id, reply.id, 'Carol sees Dave\'s reply last');
    assert.ok(timeline.findIndex(message => message.id === sent.id) < timeline.length - 1);

    const contacts = await w.dave.listContacts();
    assert.ok(contacts.some(contact => contact.id === w.carol.id), 'Carol becomes Dave\'s contact');
  });

  it('a direct chat names the other person with peer_id (profile photo in the inbox)', async () => {
    const room = await w.carol.createRoom('Dave', [w.dave.id], { kind: 'direct' });
    assert.equal(room.peer_id, w.dave.id);
    const fromDave = (await w.dave.listRooms()).find(row => row.id === room.id);
    assert.equal(fromDave?.peer_id, w.carol.id);
  });

  it('a verified test account is discoverable by email only while opted in', async t => {
    // Provision this fixture through the Appwrite plugin, outside the test. All
    // assertions and opt-in changes below use Dave's ordinary Account session.
    const account = await w.dave.account('GET', '/account');
    if (!account.emailVerification) {
      t.skip('Mark the dedicated Dave fixture email verified with the Appwrite plugin (not an OTP delivery test).');
      return;
    }
    try {
      assert.deepEqual(await w.alice.findPeople(w.dave.email), [], 'verified but not opted in');
      const profile = await w.dave.updateProfile({ discoverable_email: true });
      assert.equal(profile.discoverable_email, true);
      assert.equal(profile.email, w.dave.email.toLowerCase());
      assert.deepEqual((await w.alice.findPeople(w.dave.email.toUpperCase())).map(person => person.id), [w.dave.id]);
      const [person] = await w.alice.findPeople(w.dave.email);
      assert.deepEqual(Object.keys(person).sort(), PUBLIC_FIELDS, 'the email itself is not returned');
      await w.dave.updateProfile({ discoverable_email: false });
      assert.deepEqual(await w.alice.findPeople(w.dave.email), []);
    } finally {
      await w.dave.updateProfile({ discoverable_email: false }).catch(() => {});
      await w.dave.getProfile();
    }
  });
});
