import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('messages: content, ordering and limits', () => {
  let w, room;
  before(async () => {
    w = await world();
    room = w.track(await w.alice.createRoom(w.name('messages'), [w.bob.id]));
  });

  it('a sent message has the documented shape', async () => {
    const message = await w.alice.sendMessage(room.id, 'shape check');
    for (const field of ['id', 'room_id', 'sender_id', 'body', 'created_at', 'attachments']) assert.ok(field in message, field);
    assert.equal(message.room_id, room.id);
    assert.equal(message.sender_id, w.alice.id);
    assert.deepEqual(message.attachments, []);
    assert.ok(!Number.isNaN(Date.parse(message.created_at)));
    assert.equal(message.attachment_ids, undefined, 'internal fields stay private');
  });

  it('text round-trips exactly: Unicode, emoji, RTL, newlines, NUL and markup', async () => {
    const samples = [
      'Привет, мир! 你好 🌍🐙👨‍👩‍👧‍👦',
      'שלום عليكم ‮reversed‬',
      'line one\nline two\r\n\ttabbed',
      'nul\u0000inside',
      '"quotes" \\backslash\\ </script><b>bold</b> {"json":true} %00 %s',
      '  leading and trailing spaces  ',
    ];
    const sent = [];
    for (const body of samples) sent.push(await w.alice.sendMessage(room.id, body));
    sent.forEach((message, index) => assert.equal(message.body, samples[index]));
    const timeline = await w.bob.listMessages(room.id, { limit: samples.length });
    assert.deepEqual(timeline.map(message => message.body), samples);
  });

  it('accepts exactly 4000 UTF-8 bytes and rejects 4001', async () => {
    const exact = 'я'.repeat(2000);
    assert.equal(Buffer.byteLength(exact), 4000);
    assert.equal((await w.alice.sendMessage(room.id, exact)).body, exact);
    await rejects(w.alice.sendMessage(room.id, exact + 'x'), 400, 'validation');
  });

  it('rejects empty and whitespace-only text without photos', async () => {
    await rejects(w.alice.sendMessage(room.id, ''), 400, 'validation');
    await rejects(w.alice.sendMessage(room.id, ' \n\t '), 400, 'validation');
  });

  it('the sender is always the caller', async () => {
    const result = await w.alice.rpc('post_chat_message', { target_room: room.id, message_body: 'who sent me?', attachment_ids: [], sender_id: w.bob.id });
    assert.equal(result[0].sender_id, w.alice.id);
  });

  it('a burst arrives complete and in order for every reader', async () => {
    const sent = [];
    for (let index = 0; index < 6; index++) sent.push(await (index % 2 ? w.bob : w.alice).sendMessage(room.id, `burst ${index}`));
    assert.equal(new Set(sent.map(message => message.id)).size, sent.length, 'IDs are unique');
    for (const client of [w.alice, w.bob]) {
      assert.deepEqual((await client.listMessages(room.id, { limit: 6 })).map(message => message.body), sent.map(message => message.body));
    }
    const times = sent.map(message => Date.parse(message.created_at));
    assert.deepEqual([...times].sort((a, b) => a - b), times, 'created_at does not go backwards');
  });

  it('the limit returns the newest messages in chronological order', async () => {
    const last = await w.bob.sendMessage(room.id, `newest ${w.run}`);
    const two = await w.alice.listMessages(room.id, { limit: 2 });
    assert.equal(two.length, 2);
    assert.equal(two[1].id, last.id);
    assert.equal((await w.alice.listMessages(room.id, { limit: 1 }))[0].id, last.id);
  });

  it('the history window defaults to and is capped at 100', async () => {
    await rejects(w.alice.listMessages(room.id, { limit: 0 }), 400, 'validation');
    await rejects(w.alice.listMessages(room.id, { limit: 101 }), 400, 'validation');
    await rejects(w.alice.listMessages(room.id, { limit: 'many' }), 400, 'validation');
    assert.ok((await w.alice.listMessages(room.id, { limit: 100 })).length <= 100);
  });

  it('conversations never leak each other\'s messages', async () => {
    const other = w.track(await w.alice.createRoom(w.name('other messages'), [w.bob.id]));
    const secret = await w.alice.sendMessage(other.id, `only in other ${w.run}`);
    assert.ok(!(await w.bob.listMessages(room.id)).some(message => message.id === secret.id));
    assert.deepEqual((await w.bob.listMessages(other.id)).map(message => message.id), [secret.id]);
  });

  it('unknown or malformed conversation IDs are refused', async () => {
    await rejects(w.alice.listMessages('missing' + w.run), 403, 'forbidden');
    await rejects(w.alice.sendMessage('missing' + w.run, 'hello'), 403, 'forbidden');
    await rejects(w.alice.call('GET', '/rest/v1/chat_messages?room_id=eq.' + encodeURIComponent('../../x')), 400, 'validation');
  });
});

describe('messages: editing and deleting', () => {
  let w, group;
  before(async () => {
    w = await world();
    group = w.track(await w.alice.createRoom(w.name('edit and delete'), [w.bob.id, w.carol.id]));
  });

  it('the author edits; readers see the new text with edited_at', async () => {
    const sent = await w.alice.sendMessage(group.id, `helo ${w.run}`);
    const edited = await w.alice.editMessage(sent.id, `hello ${w.run}`);
    assert.equal(edited.id, sent.id);
    assert.equal(edited.body, `hello ${w.run}`);
    assert.equal(edited.created_at, sent.created_at, 'editing keeps the position in the timeline');
    assert.ok(Date.parse(edited.edited_at) >= Date.parse(sent.created_at));
    const seen = (await w.bob.listMessages(group.id)).find(message => message.id === sent.id);
    assert.equal(seen.body, `hello ${w.run}`);
    assert.equal(seen.edited_at, edited.edited_at);
  });

  it('editing round-trips Unicode and enforces the text limits', async () => {
    const sent = await w.bob.sendMessage(group.id, 'draft');
    assert.equal((await w.bob.editMessage(sent.id, 'Привет 🐙\nnul\u0000')).body, 'Привет 🐙\nnul\u0000');
    assert.equal((await w.bob.editMessage(sent.id, 'я'.repeat(2000))).body.length, 2000);
    await rejects(w.bob.editMessage(sent.id, 'я'.repeat(2000) + 'x'), 400, 'validation');
    await rejects(w.bob.editMessage(sent.id, ' \n '), 400, 'validation');
  });

  it('nobody edits someone else\'s message, not even the owner', async () => {
    const sent = await w.bob.sendMessage(group.id, 'mine');
    await rejects(w.alice.editMessage(sent.id, 'owner rewrite'), 403, 'forbidden');
    await rejects(w.carol.editMessage(sent.id, 'member rewrite'), 403, 'forbidden');
    await rejects(w.dave.editMessage(sent.id, 'outsider rewrite'), 403, 'forbidden');
    assert.equal((await w.bob.listMessages(group.id)).find(message => message.id === sent.id).body, 'mine');
  });

  it('the author deletes for everyone; repeating reports not found', async () => {
    const sent = await w.carol.sendMessage(group.id, `oops ${w.run}`);
    assert.equal(await w.carol.deleteMessage(sent.id), true);
    for (const reader of [w.alice, w.bob, w.carol]) {
      assert.ok(!(await reader.listMessages(group.id)).some(message => message.id === sent.id), reader.label);
    }
    await rejects(w.carol.deleteMessage(sent.id), 404, 'not_found');
    await rejects(w.carol.editMessage(sent.id, 'gone'), 404, 'not_found');
  });

  it('members cannot delete each other; the owner and admins can', async () => {
    const first = await w.bob.sendMessage(group.id, 'moderate me');
    await rejects(w.carol.deleteMessage(first.id), 403, 'forbidden');
    await rejects(w.dave.deleteMessage(first.id), 403, 'forbidden');
    assert.equal(await w.alice.deleteMessage(first.id), true, 'the owner moderates');
    await w.alice.setMemberRole(group.id, w.carol.id, 'admin');
    const second = await w.bob.sendMessage(group.id, 'moderate me too');
    assert.equal(await w.carol.deleteMessage(second.id), true, 'an admin moderates');
    await w.alice.setMemberRole(group.id, w.carol.id, 'member');
  });

  it('a removed member can no longer edit or delete their messages', async () => {
    const room = w.track(await w.alice.createRoom(w.name('leaver'), [w.dave.id]));
    const sent = await w.dave.sendMessage(room.id, 'before removal');
    await w.alice.removeMember(room.id, w.dave.id);
    await rejects(w.dave.editMessage(sent.id, 'after removal'), 403, 'forbidden');
    await rejects(w.dave.deleteMessage(sent.id), 403, 'forbidden');
  });

  it('in a direct chat only the author deletes', async () => {
    const direct = await w.alice.createRoom('Bob', [w.bob.id], { kind: 'direct' });
    const sent = await w.bob.sendMessage(direct.id, `direct ${w.run}`);
    await rejects(w.alice.deleteMessage(sent.id), 403, 'forbidden');
    assert.equal(await w.bob.deleteMessage(sent.id), true);
  });

  it('channel subscribers cannot edit or delete posts; a demoted admin cannot edit theirs', async () => {
    const channel = w.track(await w.alice.createRoom(w.name('edit channel'), [w.bob.id], { kind: 'channel' }));
    const post = await w.alice.sendMessage(channel.id, 'announcement');
    await rejects(w.bob.editMessage(post.id, 'subscriber edit'), 403, 'forbidden');
    await rejects(w.bob.deleteMessage(post.id), 403, 'forbidden');
    const edited = await w.alice.editMessage(post.id, 'announcement, corrected');
    assert.equal(edited.sender_name, channel.name, 'edited posts keep the channel identity');
    await w.alice.setMemberRole(channel.id, w.bob.id, 'admin');
    const fromBob = await w.bob.sendMessage(channel.id, 'admin post');
    await w.alice.setMemberRole(channel.id, w.bob.id, 'subscriber');
    await rejects(w.bob.editMessage(fromBob.id, 'still mine?'), 403, 'forbidden');
  });

  it('malformed message IDs are rejected', async () => {
    await rejects(w.alice.editMessage('../x', 'x'), 400, 'validation');
    await rejects(w.alice.deleteMessage('x'.repeat(37)), 400, 'validation');
  });
});
