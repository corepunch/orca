import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { ids, rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('direct chats', () => {
  let w, room;
  before(async () => {
    w = await world();
    room = await w.alice.createRoom('Bob', [w.bob.id], { kind: 'direct' });
  });

  it('a direct chat is private, postable and lists both people', async () => {
    assert.equal(room.kind, 'direct');
    assert.equal(room.visibility, 'private');
    assert.equal(room.can_post, true);
    assert.ok(['owner', 'member'].includes(room.role));
    assert.equal(room.direct_key, undefined, 'the internal pair key is not exposed');
    assert.deepEqual(ids(await w.alice.listMembers(room.id)), [w.alice.id, w.bob.id].sort());
  });

  it('is canonical for the pair in either order and under any name', async () => {
    assert.equal((await w.alice.createRoom('Another title', [w.bob.id], { kind: 'direct' })).id, room.id);
    assert.equal((await w.bob.createRoom('Alice', [w.alice.id], { kind: 'direct' })).id, room.id);
  });

  it('appears exactly once in both inboxes', async () => {
    for (const client of [w.alice, w.bob]) {
      const rooms = await client.listRooms();
      assert.equal(rooms.filter(row => row.id === room.id).length, 1, client.label);
    }
  });

  it('both people can post and read each other in order', async () => {
    const first = await w.alice.sendMessage(room.id, `ping ${w.run}`);
    const second = await w.bob.sendMessage(room.id, `pong ${w.run}`);
    for (const client of [w.alice, w.bob]) {
      const timeline = await client.listMessages(room.id, { limit: 10 });
      const a = timeline.findIndex(message => message.id === first.id), b = timeline.findIndex(message => message.id === second.id);
      assert.ok(a >= 0 && b > a, `${client.label} sees ping before pong`);
      assert.equal(timeline[b].sender_id, w.bob.id);
    }
  });

  it('the other person becomes a contact without private identifiers', async () => {
    const contacts = await w.alice.listContacts();
    const bob = contacts.find(contact => contact.id === w.bob.id);
    assert.ok(bob, 'Bob is a contact');
    assert.deepEqual(Object.keys(bob).sort(), ['display_name', 'id', 'username']);
    assert.ok(!contacts.some(contact => contact.id === w.alice.id), 'contacts exclude yourself');
  });

  it('rejects a direct chat with yourself, several people, nobody, or public visibility', async () => {
    await rejects(w.alice.createRoom('me', [w.alice.id], { kind: 'direct' }), 400, 'validation');
    await rejects(w.alice.createRoom('two', [w.bob.id, w.carol.id], { kind: 'direct' }), 400, 'validation');
    await rejects(w.alice.createRoom('none', [], { kind: 'direct' }), 400, 'validation');
    await rejects(w.alice.createRoom('public', [w.bob.id], { kind: 'direct', visibility: 'public' }), 400, 'validation');
  });

  it('rejects a direct chat with an account that does not exist', async () => {
    await rejects(w.alice.createRoom('ghost', ['nonexistent' + w.run], { kind: 'direct' }), [400, 404]);
  });

  it('membership cannot be administered in a direct chat', async () => {
    await rejects(w.alice.removeMember(room.id, w.bob.id), 403);
    await rejects(w.alice.setMemberRole(room.id, w.bob.id, 'admin'), 403);
    await rejects(w.alice.createInvite(room.id), 403);
  });

  it('outsiders cannot read, post to or inspect the direct chat', async () => {
    await rejects(w.carol.listMessages(room.id), 403, 'forbidden');
    await rejects(w.carol.sendMessage(room.id, 'intrusion'), 403, 'forbidden');
    await rejects(w.carol.listMembers(room.id), 403, 'forbidden');
    assert.ok(!(await w.carol.listRooms()).some(row => row.id === room.id));
  });
});
