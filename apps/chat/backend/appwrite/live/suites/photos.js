import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { key } from '../../src/rest.js';
import { jpeg, locatedJpeg, photo, rejects } from '../fixtures.js';
import { world } from '../world.js';

const PRIVATE_FIELDS = ['secret', 'uploads'];

describe('photos: private encrypted attachments', () => {
  let w, room, original, ready, message;
  before(async () => {
    w = await world();
    room = w.track(await w.alice.createRoom(w.name('photos'), [w.bob.id]));
    original = photo(48, 32, 12, 8);
    ready = await w.alice.uploadPhoto(room.id, original);
    message = await w.alice.sendMessage(room.id, 'look at this', [ready.id]);
  });

  it('upload walks pending → ready and never returns the key', async () => {
    assert.equal(ready.status, 'ready');
    assert.equal(ready.room_id, room.id);
    assert.equal(ready.uploader_id, w.alice.id);
    assert.equal(ready.width, 48); assert.equal(ready.height, 32);
    assert.equal(ready.byte_count, original.jpeg.length);
    for (const field of PRIVATE_FIELDS) assert.equal(ready[field], undefined, field);
  });

  it('the message embeds the attached photo', async () => {
    assert.equal(message.attachments.length, 1);
    assert.equal(message.attachments[0].id, ready.id);
    assert.equal(message.attachments[0].status, 'attached');
    assert.equal(message.attachments[0].message_id, message.id);
    for (const field of PRIVATE_FIELDS) assert.equal(message.attachments[0][field], undefined, field);
  });

  it('a member downloads the exact photo and thumbnail bytes', async () => {
    const listed = (await w.bob.listMessages(room.id)).find(row => row.id === message.id);
    assert.equal(listed.attachments[0].id, ready.id);
    assert.deepEqual(await w.bob.getPhoto(ready.id), original.jpeg);
    assert.deepEqual(await w.bob.getPhoto(ready.id, true), original.thumbnail);
  });

  it('the public bucket only holds ciphertext', async () => {
    const fileId = key('photo:' + ready.object_key);
    const response = await w.alice.api.call('GET', `/storage/buckets/${w.cfg.bucketId}/files/${fileId}/view`, { binary: true });
    if (response.status === 200) {
      assert.ok(Buffer.isBuffer(response.body));
      assert.notDeepEqual(response.body, original.jpeg);
      assert.ok(!(response.body[0] === 0xff && response.body[1] === 0xd8), 'stored bytes are not a readable JPEG');
    } else assert.ok([401, 403, 404].includes(response.status), `unexpected ${response.status}`);
  });

  it('photo-only messages and four photos per message are allowed', async () => {
    const photos = [];
    for (let index = 0; index < 4; index++) photos.push(await w.bob.uploadPhoto(room.id, photo(16 + index, 16)));
    const sent = await w.bob.sendMessage(room.id, '', photos.map(row => row.id));
    assert.equal(sent.body, '');
    assert.deepEqual(sent.attachments.map(row => row.id), photos.map(row => row.id));
    assert.deepEqual(await w.alice.getPhoto(photos[3].id), (await w.bob.getPhoto(photos[3].id)));
  });

  it('five photos are rejected', async () => {
    await rejects(w.alice.sendMessage(room.id, 'too many', ['a1', 'a2', 'a3', 'a4', 'a5']), 400, 'validation');
  });

  it('an unpublished photo is private to its uploader', async () => {
    const pending = await w.alice.uploadPhoto(room.id, photo());
    await rejects(w.bob.getPhoto(pending.id), 403);
    await rejects(w.bob.readObject(pending.object_key), 403);
    assert.ok((await w.alice.getPhoto(pending.id)).length > 0, 'the uploader can preview it');
    await w.alice.discardPhoto(pending.id);
  });

  it('nobody can attach a photo they did not upload or reuse a published one', async () => {
    const mine = await w.alice.uploadPhoto(room.id, photo());
    await rejects(w.bob.sendMessage(room.id, 'stolen', [mine.id]), 403, 'invalid_attachment');
    await rejects(w.alice.sendMessage(room.id, 'again', [ready.id]), 403, 'invalid_attachment');
    await w.alice.discardPhoto(mine.id);
  });

  it('a photo from another conversation cannot be attached', async () => {
    const other = w.track(await w.alice.createRoom(w.name('other photos'), [w.bob.id]));
    const elsewhere = await w.alice.uploadPhoto(other.id, photo());
    await rejects(w.alice.sendMessage(room.id, 'wrong room', [elsewhere.id]), 403, 'invalid_attachment');
    await w.alice.discardPhoto(elsewhere.id);
  });

  it('discarding removes an unpublished photo; published photos cannot be discarded', async () => {
    const pending = await w.alice.uploadPhoto(room.id, photo());
    const discarded = await w.alice.discardPhoto(pending.id);
    assert.equal(discarded.status, 'discarded');
    await rejects(w.alice.getPhoto(pending.id), 404);
    await rejects(w.alice.sendMessage(room.id, 'discarded', [pending.id]), 403, 'invalid_attachment');
    await rejects(w.alice.rpc('discard_chat_photo', { attachment_id: ready.id }), 403);
    await rejects(w.bob.rpc('discard_chat_photo', { attachment_id: ready.id }), 404);
  });

  it('uploaded objects are immutable and must match their reservation', async () => {
    const sample = photo();
    const reserved = await w.alice.reservePhoto(room.id, sample);
    assert.equal(reserved.status, 'pending');
    for (const field of PRIVATE_FIELDS) assert.equal(reserved[field], undefined, field);
    await rejects(w.alice.finalizePhoto(reserved.id), 400, 'invalid_photo');
    await rejects(w.alice.putObject(reserved.object_key, jpeg(sample.width + 1, sample.height)), 400, 'invalid_photo');
    await w.alice.putObject(reserved.object_key, sample.jpeg);
    await rejects(w.alice.putObject(reserved.object_key, sample.jpeg), 409, 'immutable_photo');
    await rejects(w.bob.putObject(reserved.thumbnail_key, sample.thumbnail), 403);
    await w.alice.putObject(reserved.thumbnail_key, sample.thumbnail);
    assert.equal((await w.alice.finalizePhoto(reserved.id)).status, 'ready');
    await w.alice.discardPhoto(reserved.id);
  });

  it('rejects non-JPEG data and photos carrying GPS location', async () => {
    for (const bytes of [Buffer.from('definitely not a jpeg, just some text bytes'), locatedJpeg(40, 30)]) {
      const sample = { ...photo(), jpeg: bytes };
      const reserved = await w.alice.reservePhoto(room.id, sample);
      await rejects(w.alice.putObject(reserved.object_key, bytes), 400, 'invalid_photo');
      await w.alice.discardPhoto(reserved.id);
    }
  });

  it('rejects reservations over the size and dimension limits', async () => {
    for (const [width, height, thumbnailWidth, thumbnailHeight] of [[1601, 10, 8, 8], [10, 10, 257, 8], [0, 10, 8, 8]]) {
      await rejects(w.alice.reservePhoto(room.id, { ...photo(), width, height, thumbnailWidth, thumbnailHeight }), 400, 'validation');
    }
    await rejects(w.alice.reservePhoto(room.id, { ...photo(), jpeg: Buffer.alloc(1000001) }), 400, 'validation');
  });

  it('channel subscribers cannot upload; outsiders cannot read', async () => {
    const channel = w.track(await w.alice.createRoom(w.name('photo channel'), [w.bob.id], { kind: 'channel' }));
    await rejects(w.bob.reservePhoto(channel.id, photo()), 403, 'forbidden');
    await rejects(w.carol.getPhoto(ready.id), 403);
    await rejects(w.carol.readObject(ready.object_key), 403);
  });

  it('removing a member revokes photo access immediately', async () => {
    const shared = w.track(await w.alice.createRoom(w.name('photo revocation'), [w.carol.id]));
    const sent = await w.alice.uploadPhoto(shared.id, photo());
    await w.alice.sendMessage(shared.id, '', [sent.id]);
    assert.ok((await w.carol.getPhoto(sent.id, true)).length > 0);
    await w.alice.removeMember(shared.id, w.carol.id);
    await rejects(w.carol.readObject(sent.thumbnail_key), 403);
  });

  it('a caption can be removed by editing, keeping the photo', async () => {
    const sent = await w.alice.uploadPhoto(room.id, photo());
    const captioned = await w.alice.sendMessage(room.id, 'caption', [sent.id]);
    const edited = await w.alice.editMessage(captioned.id, '');
    assert.equal(edited.body, '');
    assert.deepEqual(edited.attachments.map(row => row.id), [sent.id]);
    for (const field of PRIVATE_FIELDS) assert.equal(edited.attachments[0][field], undefined, field);
  });

  it('deleting a message deletes its photos for everyone', async () => {
    const sent = await w.alice.uploadPhoto(room.id, photo());
    const message = await w.alice.sendMessage(room.id, 'short-lived photo', [sent.id]);
    assert.ok((await w.bob.getPhoto(sent.id)).length > 0);
    assert.equal(await w.alice.deleteMessage(message.id), true);
    await rejects(w.bob.getPhoto(sent.id), 404);
    await rejects(w.alice.readObject(sent.object_key), 404);
    const fileId = key('photo:' + sent.object_key);
    const stored = await w.alice.api.call('GET', `/storage/buckets/${w.cfg.bucketId}/files/${fileId}/view`, { binary: true });
    assert.notEqual(stored.status, 200, 'the encrypted object is removed from the bucket');
  });
});
