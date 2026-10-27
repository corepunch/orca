import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { byId, ids, rejects } from '../fixtures.js';
import { world } from '../world.js';

// One public channel persists between runs so manual runs do not flood public
// channel search with throwaway channels.
async function publicChannel(w) {
  const name = `${w.cfg.prefix} public channel`;
  const owned = (await w.alice.listRooms()).find(room => room.name === name && room.kind === 'channel'
    && room.visibility === 'public' && room.role === 'owner');
  return owned || w.alice.createRoom(name, [], { kind: 'channel', visibility: 'public' });
}

describe('channels: broadcast posting, subscribers and public discovery', () => {
  let w, channel;
  before(async () => {
    w = await world();
    channel = w.track(await w.alice.createRoom(w.name('private channel'), [w.bob.id, w.carol.id], { kind: 'channel' }));
  });

  it('subscribers join as read-only; the owner can post', async () => {
    assert.equal(channel.kind, 'channel');
    assert.equal(channel.visibility, 'private');
    assert.equal(channel.role, 'owner');
    assert.equal(channel.can_post, true);
    const seen = (await w.bob.listRooms()).find(row => row.id === channel.id);
    assert.equal(seen?.role, 'subscriber');
    assert.equal(seen?.can_post, false);
  });

  it('posts show the channel as sender; subscribers read but cannot post', async () => {
    const post = await w.alice.sendMessage(channel.id, `announcement ${w.run}`);
    assert.equal(post.sender_name, channel.name);
    const read = (await w.bob.listMessages(channel.id)).find(message => message.id === post.id);
    assert.equal(read?.body, `announcement ${w.run}`);
    assert.equal(read?.sender_name, channel.name);
    await rejects(w.bob.sendMessage(channel.id, 'can I post?'), 403, 'forbidden');
  });

  it('subscribers see only themselves and staff; staff see everyone', async () => {
    assert.deepEqual(ids(await w.bob.listMembers(channel.id)), [w.alice.id, w.bob.id].sort());
    assert.deepEqual(ids(await w.alice.listMembers(channel.id)), [w.alice.id, w.bob.id, w.carol.id].sort());
  });

  it('promoting a subscriber to admin grants posting; demoting revokes it', async () => {
    await w.alice.setMemberRole(channel.id, w.bob.id, 'admin');
    assert.equal((await w.bob.listRooms()).find(row => row.id === channel.id)?.can_post, true);
    assert.deepEqual(ids(await w.bob.listMembers(channel.id)), [w.alice.id, w.bob.id, w.carol.id].sort(), 'admins see all subscribers');
    const post = await w.bob.sendMessage(channel.id, `admin post ${w.run}`);
    assert.equal(post.sender_name, channel.name, 'the channel identity hides the author');
    await w.alice.setMemberRole(channel.id, w.bob.id, 'subscriber');
    assert.equal(byId(await w.alice.listMembers(channel.id), w.bob.id).role, 'subscriber');
    await rejects(w.bob.sendMessage(channel.id, 'not anymore'), 403, 'forbidden');
  });

  it('only channel roles are valid in a channel', async () => {
    await rejects(w.alice.setMemberRole(channel.id, w.bob.id, 'member'), 403);
  });

  it('an empty channel can be created', async () => {
    const empty = w.track(await w.alice.createRoom(w.name('empty channel'), [], { kind: 'channel' }));
    assert.deepEqual(ids(await w.alice.listMembers(empty.id)), [w.alice.id]);
  });

  it('a public channel is found by a case-insensitive name fragment and joined as subscriber', async () => {
    const pub = await publicChannel(w);
    const post = await w.alice.sendMessage(pub.id, `public news ${w.run}`);
    const found = await w.dave.findChannels(w.cfg.prefix.toUpperCase() + ' PUBLIC');
    const match = found.find(row => row.id === pub.id);
    assert.ok(match, `Dave finds "${pub.name}"`);
    assert.deepEqual(Object.keys(match).sort(), ['created_at', 'id', 'kind', 'name', 'visibility']);
    const joined = await w.dave.joinPublicChannel(pub.id);
    assert.equal(joined.role, 'subscriber');
    assert.equal(joined.can_post, false);
    assert.equal((await w.dave.joinPublicChannel(pub.id)).role, 'subscriber', 'joining twice is harmless');
    assert.ok((await w.dave.listMessages(pub.id)).some(message => message.id === post.id));
    await rejects(w.dave.sendMessage(pub.id, 'subscriber post'), 403, 'forbidden');
    await w.alice.removeMember(pub.id, w.dave.id);
  });

  it('private channels and groups are not discoverable or joinable publicly', async () => {
    assert.ok(!(await w.dave.findChannels(channel.name)).some(row => row.id === channel.id));
    await rejects(w.dave.joinPublicChannel(channel.id), 404, 'not_found');
    const group = w.track(await w.alice.createRoom(w.name('group not channel'), []));
    await rejects(w.dave.joinPublicChannel(group.id), 404, 'not_found');
  });

  it('channel search needs two characters and validates length', async () => {
    assert.deepEqual(await w.dave.findChannels('o'), []);
    assert.deepEqual(await w.dave.findChannels('  '), []);
    await rejects(w.dave.findChannels('x'.repeat(81)), 400, 'validation');
  });
});
