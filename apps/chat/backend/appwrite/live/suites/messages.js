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
