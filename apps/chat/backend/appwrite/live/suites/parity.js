// Telegram feature parity, excluding video and calls. Each case is a TODO probe:
// it states the expected behavior and the proposed wire contract, runs against
// the live backend, and is reported as TODO (not a failure) until implemented.
// When a feature ships, adjust its probe to the real contract and drop `todo`
// so it becomes a regular regression check.
import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { world } from '../world.js';

const todo = 'Telegram parity: not implemented yet';
// One-line TODO reasons keep the report readable; stacks add nothing here.
const probe = (name, fn) => it(name, { todo }, async () => {
  try { await fn(); } catch (error) {
    const reason = new Error(error.message.replace(/\s+/g, ' ').trim());
    reason.stack = reason.message; throw reason;
  }
});

describe('Telegram parity (TODO probes; not counted as failures)', () => {
  let w, group, message;
  before(async () => {
    w = await world();
    group = w.track(await w.alice.createRoom(w.name('parity'), [w.bob.id, w.carol.id]));
    message = await w.alice.sendMessage(group.id, `parity base ${w.run}`);
  });

  describe('messages', () => {
    probe('edit own message; others see the new text and an edited mark', async () => {
      await w.alice.rpc('edit_chat_message', { target_message: message.id, message_body: 'edited' });
      const seen = (await w.bob.listMessages(group.id)).find(row => row.id === message.id);
      assert.equal(seen.body, 'edited'); assert.ok(seen.edited_at);
    });
    probe('cannot edit someone else\'s message', async () => {
      const result = await w.bob.send('POST', '/rest/v1/rpc/edit_chat_message', { target_message: message.id, message_body: 'hijack' });
      assert.equal(result.status, 403);
    });
    probe('delete own message for everyone', async () => {
      const doomed = await w.alice.sendMessage(group.id, 'delete me');
      await w.alice.rpc('delete_chat_message', { target_message: doomed.id });
      assert.ok(!(await w.bob.listMessages(group.id)).some(row => row.id === doomed.id));
    });
    probe('admins delete other members\' messages in groups', async () => {
      const spam = await w.bob.sendMessage(group.id, 'spam');
      await w.alice.rpc('delete_chat_message', { target_message: spam.id });
      assert.ok(!(await w.carol.listMessages(group.id)).some(row => row.id === spam.id));
    });
    probe('reply to a message', async () => {
      const reply = (await w.bob.rpc('post_chat_message', { target_room: group.id, message_body: 'reply', attachment_ids: [], reply_to: message.id }))[0];
      assert.equal(reply.reply_to, message.id);
    });
    probe('forward a message to another chat, keeping the original author', async () => {
      const other = w.track(await w.bob.createRoom(w.name('forward target'), []));
      const forwarded = (await w.bob.rpc('forward_chat_message', { target_message: message.id, target_room: other.id }))[0];
      assert.equal(forwarded.forwarded_from, w.alice.id);
    });
    probe('emoji reactions with per-emoji counts', async () => {
      await w.bob.rpc('react_chat_message', { target_message: message.id, reaction: '👍' });
      const seen = (await w.alice.listMessages(group.id)).find(row => row.id === message.id);
      assert.equal(seen.reactions?.['👍'], 1);
    });
    probe('pin a message; members see the pinned message', async () => {
      await w.alice.rpc('pin_chat_message', { target_message: message.id });
      assert.equal((await w.bob.listRooms()).find(row => row.id === group.id).pinned_message_id, message.id);
    });
    probe('@mentions are recorded for the mentioned person', async () => {
      const mention = await w.alice.sendMessage(group.id, `@${w.bob.username} look`);
      assert.ok(mention.mentions?.includes(w.bob.id));
    });
    probe('formatted text entities (bold, italic, code, links) survive', async () => {
      const sent = (await w.alice.rpc('post_chat_message', { target_room: group.id, message_body: 'bold', attachment_ids: [],
        entities: [{ type: 'bold', offset: 0, length: 4 }] }))[0];
      assert.equal(sent.entities?.[0]?.type, 'bold');
    });
    probe('link previews for URLs', async () => {
      const sent = await w.alice.sendMessage(group.id, 'https://example.com');
      assert.ok(sent.link_preview);
    });
    probe('scheduled messages', async () => {
      await w.alice.rpc('post_chat_message', { target_room: group.id, message_body: 'later', attachment_ids: [],
        send_at: new Date(Date.now() + 3600000).toISOString() });
      assert.ok((await w.alice.call('GET', '/rest/v1/chat_scheduled?room_id=eq.' + group.id)).length > 0);
    });
    probe('search messages in a conversation', async () => {
      const found = await w.bob.rpc('search_chat_messages', { target_room: group.id, query: 'parity base' });
      assert.ok(found.some(row => row.id === message.id));
    });
    probe('global message search across my chats', async () => {
      const found = await w.bob.rpc('search_chat_messages', { query: `parity base ${w.run}` });
      assert.ok(found.some(row => row.id === message.id));
    });
    probe('load older history beyond the latest 100 messages', async () => {
      const page = await w.bob.call('GET', `/rest/v1/chat_messages?room_id=eq.${group.id}&before=${message.id}&limit=50`);
      assert.ok(Array.isArray(page));
      assert.ok(!page.some(row => row.id === message.id), 'the cursor message itself is excluded');
    });
  });

  describe('read state and presence', () => {
    probe('unread counters per conversation', async () => {
      await w.bob.sendMessage(group.id, 'unread for Carol');
      assert.ok((await w.carol.listRooms()).find(row => row.id === group.id).unread_count >= 1);
    });
    probe('mark a conversation read; the sender sees read receipts', async () => {
      await w.carol.rpc('mark_chat_read', { target_room: group.id, target_message: message.id });
      const seen = (await w.alice.listMessages(group.id)).find(row => row.id === message.id);
      assert.ok(seen.read_by?.includes(w.carol.id) || seen.read_count >= 1);
    });
    probe('typing indicator', async () => {
      await w.bob.rpc('set_chat_typing', { target_room: group.id });
      assert.ok((await w.alice.listRooms()).find(row => row.id === group.id).typing?.includes(w.bob.id));
    });
    probe('last seen / online status, with privacy setting', async () => {
      const [bob] = await w.alice.findPeople(w.bob.username);
      assert.ok('last_seen_at' in bob);
    });
  });

  describe('inbox', () => {
    probe('inbox rows carry a last-message preview', async () => {
      assert.ok((await w.alice.listRooms()).find(row => row.id === group.id).last_message);
    });
    probe('inbox is ordered by latest activity, not creation time', async () => {
      const older = w.track(await w.alice.createRoom(w.name('older'), []));
      w.track(await w.alice.createRoom(w.name('newer'), []));
      await w.alice.sendMessage(older.id, 'bump');
      assert.equal((await w.alice.listRooms())[0].id, older.id);
    });
    probe('pin, archive and mute conversations', async () => {
      await w.alice.rpc('update_chat_settings', { target_room: group.id, pinned: true, archived: false, muted_until: null });
      assert.equal((await w.alice.listRooms()).find(row => row.id === group.id).pinned, true);
    });
    probe('Saved Messages: a private chat with yourself', async () => {
      const saved = await w.alice.createRoom('Saved Messages', [w.alice.id], { kind: 'direct' });
      assert.equal(saved.kind, 'direct');
    });
    probe('clear history / delete a chat for myself', async () => {
      const room = w.track(await w.alice.createRoom(w.name('to delete'), []));
      await w.alice.rpc('delete_chat_room', { target_room: room.id });
      assert.ok(!(await w.alice.listRooms()).some(row => row.id === room.id));
    });
  });

  describe('groups and channels', () => {
    probe('a member can leave a group', async () => {
      await w.carol.rpc('leave_chat_room', { target_room: group.id });
      assert.ok(!(await w.carol.listRooms()).some(row => row.id === group.id));
    });
    probe('rename a group and set its description', async () => {
      await w.alice.rpc('update_chat_room', { target_room: group.id, room_name: 'Renamed', description: 'About' });
      const seen = (await w.bob.listRooms()).find(row => row.id === group.id);
      assert.equal(seen.name, 'Renamed'); assert.equal(seen.description, 'About');
    });
    probe('group and channel photos', async () => {
      assert.ok('photo_id' in (await w.alice.listRooms()).find(row => row.id === group.id));
    });
    probe('transfer ownership', async () => {
      const room = w.track(await w.alice.createRoom(w.name('transfer'), [w.bob.id]));
      await w.alice.setMemberRole(room.id, w.bob.id, 'owner');
      assert.equal((await w.bob.listRooms()).find(row => row.id === room.id).role, 'owner');
    });
    probe('add a member directly (without an invitation link)', async () => {
      const room = w.track(await w.alice.createRoom(w.name('add member'), []));
      await w.alice.rpc('add_chat_member', { target_room: room.id, target_user: w.bob.id });
      assert.ok((await w.bob.listRooms()).some(row => row.id === room.id));
    });
    probe('granular admin rights (post, delete, ban, invite, pin)', async () => {
      const room = w.track(await w.alice.createRoom(w.name('rights'), [w.bob.id]));
      await w.alice.rpc('set_chat_member_role', { target_room: room.id, target_user: w.bob.id, member_role: 'admin', rights: { delete_messages: false } });
      assert.equal((await w.alice.listMembers(room.id)).find(row => row.user_id === w.bob.id).rights?.delete_messages, false);
    });
    probe('ban a member so invitations cannot bring them back', async () => {
      const room = w.track(await w.alice.createRoom(w.name('ban'), [w.dave.id]));
      await w.alice.rpc('ban_chat_member', { target_room: room.id, target_user: w.dave.id });
      const code = (await w.alice.createInvite(room.id)).code;
      assert.equal((await w.dave.send('POST', '/rest/v1/rpc/join_chat_invite', { invite_code: code })).status, 403);
    });
    probe('slow mode', async () => {
      await w.alice.rpc('update_chat_room', { target_room: group.id, slow_mode_seconds: 30 });
      await w.bob.sendMessage(group.id, 'one');
      assert.equal((await w.bob.send('POST', '/rest/v1/rpc/post_chat_message', { target_room: group.id, message_body: 'two', attachment_ids: [] })).status, 429);
    });
    probe('polls', async () => {
      const poll = (await w.alice.rpc('post_chat_poll', { target_room: group.id, question: 'Lunch?', options: ['yes', 'no'] }))[0];
      assert.equal(poll.poll?.options?.length, 2);
    });
    probe('channel view counters', async () => {
      const channel = w.track(await w.alice.createRoom(w.name('views'), [w.bob.id], { kind: 'channel' }));
      const post = await w.alice.sendMessage(channel.id, 'counted');
      await w.bob.listMessages(channel.id);
      assert.ok((await w.alice.listMessages(channel.id)).find(row => row.id === post.id).views >= 1);
    });
    probe('channel discussion group (comments)', async () => {
      const channel = w.track(await w.alice.createRoom(w.name('comments'), [], { kind: 'channel' }));
      assert.ok((await w.alice.rpc('link_chat_discussion', { target_room: channel.id, discussion_room: group.id })));
    });
    probe('public @username links resolve to a person or public channel', async () => {
      const resolved = await w.dave.rpc('resolve_chat_username', { username: w.alice.username });
      assert.equal(resolved.id, w.alice.id);
    });
  });

  describe('people and privacy', () => {
    probe('search people by display name or username prefix', async () => {
      const found = await w.alice.findPeople(w.bob.username.slice(0, -2));
      assert.ok(found.some(person => person.id === w.bob.id));
    });
    probe('profile bio', async () => {
      assert.equal((await w.carol.updateProfile({ bio: 'hello' })).bio, 'hello');
    });
    probe('upload my own profile photo from Chat', async () => {
      await w.carol.rpc('set_chat_avatar', { attachment_id: 'pending' });
      assert.ok((await w.carol.getAvatars([w.carol.id]))[w.carol.id]);
    });
    probe('block a user: they can no longer message me', async () => {
      const room = await w.dave.createRoom('Bob', [w.bob.id], { kind: 'direct' });
      await w.bob.rpc('block_chat_user', { target_user: w.dave.id });
      try {
        assert.equal((await w.dave.send('POST', '/rest/v1/rpc/post_chat_message', { target_room: room.id, message_body: 'hi', attachment_ids: [] })).status, 403);
      } finally { await w.bob.rpc('unblock_chat_user', { target_user: w.dave.id }).catch(() => {}); }
    });
    probe('report spam', async () => {
      assert.ok(await w.bob.rpc('report_chat_message', { target_message: message.id, reason: 'spam' }));
    });
    probe('contacts can be added and named without sharing a chat', async () => {
      await w.dave.rpc('add_chat_contact', { target_user: w.alice.id, display_name: 'Al' });
      assert.ok((await w.dave.listContacts()).some(row => row.id === w.alice.id && row.display_name === 'Al'));
    });
    probe('terminate my other sessions', async () => {
      const result = await w.alice.send('POST', '/rest/v1/rpc/terminate_other_sessions', {});
      assert.equal(result.status, 200);
    });
  });

  describe('media beyond photos (no video)', () => {
    probe('files / documents', async () => {
      const reserved = await w.alice.rpc('reserve_chat_file', { target_room: group.id, file_name: 'notes.pdf', mime_type: 'application/pdf', file_bytes: 1024 });
      assert.ok(reserved.id);
    });
    probe('voice messages', async () => {
      const reserved = await w.alice.rpc('reserve_chat_voice', { target_room: group.id, duration_seconds: 3, file_bytes: 2048 });
      assert.ok(reserved.id);
    });
    probe('stickers and GIFs', async () => {
      assert.ok(Array.isArray(await w.alice.rpc('list_chat_stickers', {})));
    });
    probe('PNG / non-JPEG photos keep their format', async () => {
      const result = await w.alice.send('POST', '/rest/v1/rpc/reserve_chat_photo', { target_room: group.id, mime_type: 'image/png',
        photo_width: 1, photo_height: 1, photo_bytes: 67, thumbnail_width: 1, thumbnail_height: 1, thumbnail_bytes: 67 });
      assert.equal(result.body?.mime_type, 'image/png');
    });
  });

  describe('delivery', () => {
    probe('realtime updates instead of polling', async () => {
      assert.equal((await w.alice.send('GET', '/rest/v1/realtime', undefined)).status, 200);
    });
    probe('push notifications: register a device token', async () => {
      await w.alice.rpc('register_chat_device', { platform: 'ios', token: 'live-test-' + w.run });
    });
  });
});
