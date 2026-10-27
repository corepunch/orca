import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { byId, ids, rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('groups: creation, posting and roles', () => {
  let w, group;
  before(async () => {
    w = await world();
    group = w.track(await w.alice.createRoom(w.name('group'), [w.bob.id, w.carol.id, w.bob.id, w.alice.id]));
  });

  it('the creator owns the group; duplicates and self are ignored', async () => {
    assert.equal(group.kind, 'group');
    assert.equal(group.visibility, 'private');
    assert.equal(group.role, 'owner');
    assert.equal(group.can_post, true);
    assert.equal(group.peer_id, undefined);
    const members = await w.alice.listMembers(group.id);
    assert.deepEqual(ids(members), [w.alice.id, w.bob.id, w.carol.id].sort());
    assert.equal(byId(members, w.alice.id).role, 'owner');
    assert.equal(byId(members, w.bob.id).role, 'member');
    assert.equal(byId(members, w.bob.id).display_name, w.bob.displayName);
    assert.equal(byId(members, w.bob.id).username, w.bob.username);
  });

  it('members see the group in their inbox with their own role', async () => {
    const seen = (await w.bob.listRooms()).find(row => row.id === group.id);
    assert.equal(seen?.role, 'member');
    assert.equal(seen?.can_post, true);
    assert.equal(seen?.name, group.name);
  });

  it('every member can post and everyone reads the same timeline', async () => {
    const sent = [];
    for (const client of [w.alice, w.bob, w.carol]) sent.push(await client.sendMessage(group.id, `${client.label} says hi ${w.run}`));
    for (const client of [w.alice, w.bob, w.carol]) {
      const timeline = (await client.listMessages(group.id)).map(message => message.id);
      assert.deepEqual(timeline.slice(-3), sent.map(message => message.id), client.label);
    }
  });

  it('a group may start empty and members can be invited later', async () => {
    const empty = w.track(await w.alice.createRoom(w.name('empty group'), []));
    assert.deepEqual(ids(await w.alice.listMembers(empty.id)), [w.alice.id]);
    assert.equal((await w.alice.sendMessage(empty.id, 'note to self')).sender_id, w.alice.id);
  });

  it('names accept 80 Unicode characters and reject empty, blank or 81', async () => {
    const long = w.track(await w.alice.createRoom('я'.repeat(80), []));
    assert.equal(long.name, 'я'.repeat(80));
    await rejects(w.alice.createRoom('', []), 400, 'validation');
    await rejects(w.alice.createRoom('   ', []), 400, 'validation');
    await rejects(w.alice.createRoom('я'.repeat(81), []), 400, 'validation');
  });

  it('rejects invalid kinds, public groups, more than 50 members and malformed IDs', async () => {
    await rejects(w.alice.createRoom(w.name('bad kind'), [], { kind: 'supergroup' }), 400, 'validation');
    await rejects(w.alice.createRoom(w.name('public group'), [], { kind: 'group', visibility: 'public' }), 400, 'validation');
    await rejects(w.alice.createRoom(w.name('crowd'), Array.from({ length: 51 }, (_, index) => 'u' + index)), 400, 'validation');
    await rejects(w.alice.createRoom(w.name('bad id'), ['../etc/passwd']), 400, 'validation');
  });

  it('rejects members that are not real accounts and leaves nothing behind', async () => {
    const before = (await w.alice.listRooms()).length;
    await rejects(w.alice.createRoom(w.name('ghost member'), [w.bob.id, 'ghost' + w.run]), [400, 404]);
    assert.equal((await w.alice.listRooms()).length, before);
  });

  it('only the owner assigns roles; an admin can remove ordinary members', async () => {
    const room = w.track(await w.alice.createRoom(w.name('roles'), [w.bob.id, w.carol.id, w.dave.id]));
    await rejects(w.bob.setMemberRole(room.id, w.carol.id, 'admin'), 403, 'forbidden');
    assert.equal(await w.alice.setMemberRole(room.id, w.bob.id, 'admin'), true);
    assert.equal(byId(await w.alice.listMembers(room.id), w.bob.id).role, 'admin');
    await rejects(w.bob.setMemberRole(room.id, w.carol.id, 'admin'), 403, 'forbidden');

    assert.equal(await w.bob.removeMember(room.id, w.dave.id), true);
    assert.ok(!ids(await w.alice.listMembers(room.id)).includes(w.dave.id));
    await rejects(w.dave.listMessages(room.id), 403, 'forbidden');
    await rejects(w.dave.sendMessage(room.id, 'still here?'), 403, 'forbidden');
    assert.ok(!(await w.dave.listRooms()).some(row => row.id === room.id), 'the room left Dave\'s inbox');
  });

  it('staff and the owner are protected', async () => {
    const room = w.track(await w.alice.createRoom(w.name('protected'), [w.bob.id, w.carol.id]));
    await w.alice.setMemberRole(room.id, w.bob.id, 'admin');
    await w.alice.setMemberRole(room.id, w.carol.id, 'admin');
    await rejects(w.bob.removeMember(room.id, w.carol.id), 403, 'forbidden');
    await rejects(w.bob.removeMember(room.id, w.alice.id), 403, 'forbidden');
    await rejects(w.alice.removeMember(room.id, w.alice.id), 403, 'forbidden');
    await rejects(w.alice.setMemberRole(room.id, w.alice.id, 'member'), 403, 'forbidden');
    assert.equal(await w.alice.removeMember(room.id, w.carol.id), true, 'the owner removes an admin');
  });

  it('ordinary members cannot remove anyone or change roles', async () => {
    const room = w.track(await w.alice.createRoom(w.name('members'), [w.bob.id, w.carol.id]));
    await rejects(w.bob.removeMember(room.id, w.carol.id), 403, 'forbidden');
    await rejects(w.bob.setMemberRole(room.id, w.bob.id, 'admin'), 403, 'forbidden');
  });

  it('only group roles are valid in a group', async () => {
    const room = w.track(await w.alice.createRoom(w.name('valid roles'), [w.bob.id]));
    await rejects(w.alice.setMemberRole(room.id, w.bob.id, 'subscriber'), 403);
    await rejects(w.alice.setMemberRole(room.id, w.bob.id, 'owner'), 403);
    await rejects(w.alice.setMemberRole(room.id, w.bob.id, 'superadmin'), 403);
    assert.equal(await w.alice.setMemberRole(room.id, w.bob.id, 'admin'), true);
    assert.equal(await w.alice.setMemberRole(room.id, w.bob.id, 'member'), true);
    assert.equal(byId(await w.alice.listMembers(room.id), w.bob.id).role, 'member');
  });

  it('removing someone who is not a member is reported as not found', async () => {
    const room = w.track(await w.alice.createRoom(w.name('not found'), [w.bob.id]));
    await rejects(w.alice.removeMember(room.id, w.dave.id), 404, 'not_found');
  });

  it('an outsider cannot see or touch the group', async () => {
    await rejects(w.dave.listMembers(group.id), 403, 'forbidden');
    await rejects(w.dave.listMessages(group.id), 403, 'forbidden');
    await rejects(w.dave.sendMessage(group.id, 'hello?'), 403, 'forbidden');
    await rejects(w.dave.createInvite(group.id), 403, 'forbidden');
  });
});
