import { reject } from './rest.js';
import { decodeUpload } from './photos.js';

// Explicit method/path handlers follow Lapis's application/route boundary. Auth
// middleware supplies the caller; handlers select domain operations, never raw DB paths.
const handlers = new Map();
const route = (method, path, handler) => handlers.set(method + ' ' + path, handler);
const rpc = (name, handler) => route('POST', '/rest/v1/rpc/' + name, handler);
const equalValue = (url, field) => (url.searchParams.get(field) || '').replace(/^eq\./, '');

route('PUT', '/auth/v1/user', (s, c) => s.auth.requestLink(c.user, c.body));
route('POST', '/auth/v1/verify', (s, c) => {
  if (!['email_change', 'phone_change'].includes(c.body.type)) reject(404, 'not_found', 'Use Appwrite Account directly for sign-in verification.');
  return s.auth.verifyLink(c.user, c.body);
});
route('GET', '/rest/v1/chat_rooms', (s, c) => s.listRooms(c.user));
route('GET', '/rest/v1/chat_profiles', (s, c) => s.listContacts(c.user));
route('GET', '/rest/v1/chat_messages', (s, c) => s.listMessages(c.user, equalValue(c.url, 'room_id'), c.url.searchParams.get('limit') ?? undefined));
route('GET', '/rest/v1/chat_attachments', async (s, c) => {
  const { row, committed } = await s.photoAccess(c.user, equalValue(c.url, 'id'));
  return [s.photoView(row, committed ? row.message_id : null)];
});
route('POST', '/functions/v1/finalize-chat-photo', (s, c) => s.finalizePhoto(c.user, c.body.attachment_id));
route('DELETE', '/storage/v1/object/chat-photos', (s, c) => s.removeObjects(c.user, c.body.prefixes));

rpc('get_chat_profile', (s, c) => c.profile);
rpc('update_chat_profile', (s, c) => s.updateProfile(c.user, c.body.values));
rpc('find_chat_people', (s, c) => s.findPeople(c.user, c.body.query));
rpc('create_chat_room', (s, c) => s.createRoom(c.user, c.body));
rpc('list_chat_members', (s, c) => s.listMembers(c.user, c.body.target_room));
rpc('set_chat_member_role', (s, c) => s.setRole(c.user, c.body));
rpc('remove_chat_member', (s, c) => s.setRole(c.user, c.body, true));
rpc('create_chat_invite', (s, c) => s.createInvite(c.user, c.body.target_room));
rpc('join_chat_invite', (s, c) => s.joinInvite(c.user, c.body.invite_code));
rpc('revoke_chat_invite', (s, c) => s.revokeInvite(c.user, c.body.target_room, c.body.invite_code));
rpc('find_chat_channels', (s, c) => s.findChannels(c.body.query));
rpc('join_public_chat_channel', (s, c) => s.joinPublic(c.user, c.body.target_room));
rpc('post_chat_message', (s, c) => s.postMessage(c.user, c.body));
rpc('edit_chat_message', (s, c) => s.editMessage(c.user, c.body));
rpc('delete_chat_message', (s, c) => s.deleteMessage(c.user, c.body));
rpc('reserve_chat_photo', (s, c) => s.reservePhoto(c.user, c.body));
rpc('discard_chat_photo', (s, c) => s.discardPhoto(c.user, c.body.attachment_id));

const objectRoutes = [
  { method: 'POST', prefix: '/storage/v1/object/chat-photos/',
    handler: (s, c) => s.uploadPhoto(c.user, c.objectKey, decodeUpload(c.body, c.encoding)) },
  { method: 'GET', prefix: '/storage/v1/object/authenticated/chat-photos/',
    handler: (s, c) => s.readPhoto(c.user, c.objectKey) },
];

export function resolveRoute(method, url) {
  const handler = handlers.get(method + ' ' + url.pathname);
  if (handler) return { handler };
  for (const candidate of objectRoutes) if (method === candidate.method && url.pathname.startsWith(candidate.prefix)) {
    let objectKey;
    try { objectKey = decodeURIComponent(url.pathname.slice(candidate.prefix.length)); }
    catch { reject(400, 'validation', 'Choose a valid photo object.'); }
    return { handler: candidate.handler, objectKey };
  }
  reject(404, 'not_found', 'Chat endpoint not found.');
}
