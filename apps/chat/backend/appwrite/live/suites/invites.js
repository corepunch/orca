import { before, describe, it } from 'node:test';
import assert from 'node:assert/strict';
import { ids, rejects } from '../fixtures.js';
import { world } from '../world.js';

describe('invitations', () => {
  let w, group, invite, history;
  before(async () => {
    w = await world();
    group = w.track(await w.alice.createRoom(w.name('invite group'), [w.bob.id]));
    history = await w.alice.sendMessage(group.id, `before Dave joined ${w.run}`);
    invite = await w.alice.createInvite(group.id);
  });

  it('an invitation has a long random code, ten uses and a 24-hour expiry', async () => {
    assert.match(invite.code, /^[a-f0-9]{48}$/);
    assert.equal(invite.room_id, group.id);
    assert.equal(invite.max_uses, 10);
    assert.equal(invite.uses, 0);
    assert.equal(invite.revoked, false);
    const hours = (Date.parse(invite.expires_at) - Date.now()) / 3600000;
    assert.ok(hours > 23 && hours <= 24.1, `expires in ${hours.toFixed(2)} h`);
  });

  it('joining adds a postable member who sees earlier history', async () => {
    const joined = await w.dave.joinInvite(invite.code);
    assert.equal(joined.id, group.id);
    assert.equal(joined.role, 'member');
    assert.equal(joined.can_post, true);
    assert.ok((await w.dave.listRooms()).some(row => row.id === group.id));
    assert.ok((await w.dave.listMessages(group.id)).some(message => message.id === history.id));
    await w.dave.sendMessage(group.id, `Dave joined ${w.run}`);
    assert.ok(ids(await w.alice.listMembers(group.id)).includes(w.dave.id));
  });

  it('joining again is harmless and keeps the role', async () => {
    assert.equal((await w.dave.joinInvite(invite.code)).role, 'member');
    assert.equal((await w.bob.joinInvite(invite.code)).role, 'member', 'an existing member can follow the link');
  });

  it('ordinary members cannot create or revoke invitations; admins can', async () => {
    await rejects(w.bob.createInvite(group.id), 403, 'forbidden');
    await rejects(w.bob.revokeInvite(group.id, invite.code), 403, 'forbidden');
    await w.alice.setMemberRole(group.id, w.bob.id, 'admin');
    const fromAdmin = await w.bob.createInvite(group.id);
    assert.equal(await w.bob.revokeInvite(group.id, fromAdmin.code), true);
    await w.alice.setMemberRole(group.id, w.bob.id, 'member');
  });

  it('a revoked invitation stops working; revoking unknown codes returns false', async () => {
    const next = await w.alice.createInvite(group.id);
    assert.equal(await w.alice.revokeInvite(group.id, next.code), true);
    await rejects(w.carol.joinInvite(next.code), 404, 'invalid_invite');
    assert.equal(await w.alice.revokeInvite(group.id, 'f'.repeat(48)), false);
    assert.ok(!(await w.carol.listRooms()).some(row => row.id === group.id));
  });

  it('an invitation only revokes within its own conversation', async () => {
    const other = w.track(await w.alice.createRoom(w.name('other invite room'), []));
    const code = (await w.alice.createInvite(other.id)).code;
    assert.equal(await w.alice.revokeInvite(group.id, code), false);
    assert.equal((await w.carol.joinInvite(code)).id, other.id);
  });

  it('malformed or unknown codes are rejected uniformly', async () => {
    for (const code of ['', 'short', 'g'.repeat(48), '0'.repeat(48), invite.code.toUpperCase()]) {
      await rejects(w.carol.joinInvite(code), 404, 'invalid_invite');
    }
  });

  it('a removed member can come back with a valid invitation', async () => {
    await w.alice.removeMember(group.id, w.dave.id);
    await rejects(w.dave.listMessages(group.id), 403);
    assert.equal((await w.dave.joinInvite(invite.code)).role, 'member');
    assert.ok((await w.dave.listMessages(group.id)).length > 0);
  });

  it('a channel invitation makes a read-only subscriber', async () => {
    const channel = w.track(await w.alice.createRoom(w.name('invite channel'), [], { kind: 'channel' }));
    const code = (await w.alice.createInvite(channel.id)).code;
    const joined = await w.carol.joinInvite(code);
    assert.equal(joined.role, 'subscriber');
    assert.equal(joined.can_post, false);
    await rejects(w.carol.sendMessage(channel.id, 'subscriber'), 403, 'forbidden');
  });
});
