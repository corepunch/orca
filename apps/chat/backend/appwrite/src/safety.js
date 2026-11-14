import { eq, key, reject } from './rest.js';

export const RULES_VERSION = '2026-11-08';
const reasons = new Set(['spam', 'harassment', 'sexual_content', 'violence', 'hate', 'other']);
const id = value => {
  if (typeof value !== 'string' || !/^[A-Za-z0-9][A-Za-z0-9._-]{0,35}$/.test(value)) reject(400, 'validation', 'Choose a valid person or message.');
  return value;
};
const normalize = value => String(value).normalize('NFKC').toLowerCase().replace(/\p{Cf}/gu, '').replace(/[^\p{L}\p{N}]+/gu, ' ').trim();
const baseTerms = ['fuck', 'shit', 'cunt', 'nigger', 'faggot', 'heil hitler', 'kill yourself', 'i will kill you', 'child sexual abuse material'];
const blockId = (from, to) => key('block:' + from + ':' + to);
const pairLock = (from, to) => 'interaction:' + [from, to].sort().join(':');

const socialPath = (repo, collection, docId) =>
  `/databases/${encodeURIComponent(repo.databaseId)}/collections/${collection}/documents/${encodeURIComponent(docId)}`;
const socialImages = doc => [...new Set([...(Array.isArray(doc.imageIds) ? doc.imageIds : []), doc.imageId]
  .filter(value => typeof value === 'string' && value))].slice(0, 9);
// Octopus collections are stored as plain Appwrite documents, not service payloads.
async function socialDocument(rest, repo, collection, docId) {
  try { return await rest.request('GET', socialPath(repo, collection, docId)); }
  catch (error) { if (error.status === 404) return null; throw error; }
}

export class Safety {
  constructor(service, { blockedTerms = [], now = Date.now } = {}) {
    this.service = service; this.repo = service.repo; this.auth = service.auth; this.now = now;
    if (!Array.isArray(blockedTerms) || blockedTerms.length > 200 || blockedTerms.some(term => typeof term !== 'string' || !term.trim() || term.length > 128))
      reject(503, 'moderation_configuration', 'The content filter needs administrator configuration.');
    this.terms = [...baseTerms, ...blockedTerms].map(normalize);
  }
  validate(text) {
    const normalized = ' ' + normalize(text) + ' ';
    if (this.terms.some(term => normalized.includes(' ' + term + ' ')))
      reject(422, 'objectionable_content', 'This text violates the community rules. Edit it before posting.');
  }
  displayName(text) {
    try { this.validate(text); return text; }
    catch (error) { if (error.code === 'objectionable_content') return 'Member'; throw error; }
  }
  async blocked(a, b) {
    if (a === b) return false;
    const records = await Promise.all([this.repo.get('blocks', blockId(a, b)), this.repo.get('blocks', blockId(b, a))]);
    return records.some(Boolean);
  }
  async assertInteraction(a, b) {
    if (await this.blocked(a, b)) reject(403, 'interaction_unavailable', 'You cannot contact this person.');
  }
  async directAction(user, room, action) {
    if (room.kind !== 'direct') return action();
    const peer = (await this.repo.list('members', [eq('room_id', room.id)], 2)).find(member => member.user_id !== user.$id);
    if (!peer) reject(403, 'interaction_unavailable', 'This conversation is no longer available.');
    return this.repo.lock(pairLock(user.$id, peer.user_id), async () => {
      await this.assertInteraction(user.$id, peer.user_id); return action();
    });
  }
  async setBlock(user, body) {
    const target = id(body.target_user);
    if (target === user.$id || typeof body.blocked !== 'boolean') reject(400, 'validation', 'Choose another person and whether to block them.');
    return this.repo.lock(pairLock(user.$id, target), async () => {
      const recordId = blockId(user.$id, target);
      if (!body.blocked) { await this.repo.remove('blocks', recordId); return true; }
      if (await this.repo.get('blocks', recordId)) return true;
      await this.auth.rest.request('GET', '/users/' + encodeURIComponent(target));
      await this.repo.create('blocks', { blocker_id: user.$id, blocked_id: target,
        created_at: new Date(this.now()).toISOString() }, recordId);
      return true;
    });
  }
  async listBlocks(user) {
    const rows = await this.repo.list('blocks', [eq('blocker_id', user.$id)]), result = [];
    for (const row of rows) {
      const profile = await this.repo.get('profiles', row.blocked_id);
      result.push({ user_id: row.blocked_id, display_name: profile ? this.displayName(profile.display_name) : 'Deleted account', username: profile?.username || null });
    }
    return result;
  }
  async report(user, body) {
    const target = id(body.target_user), details = body.details || '';
    if (target === user.$id || !reasons.has(body.reason) || typeof details !== 'string' || Buffer.byteLength(details) > 2000)
      reject(400, 'validation', 'Choose a report reason and details of at most 2000 text bytes.');
    if ([body.target_message, body.target_post, body.target_comment].filter(Boolean).length > 1)
      reject(400, 'validation', 'Report one message, post or comment at a time.');
    let message, social;
    if (body.target_post || body.target_comment) {
      // Octopus posts and comments are public; evidence is copied so a later
      // edit or deletion cannot change what moderators review.
      const kind = body.target_post ? 'post' : 'comment';
      const doc = await socialDocument(this.auth.rest, this.repo, kind + 's', id(body.target_post || body.target_comment));
      if (!doc || doc.authorId !== target) reject(404, 'not_found', 'The reported ' + kind + ' was not found.');
      social = kind === 'post'
        ? { post_id: doc.$id, evidence: { kind, title: doc.title || '', body: doc.content || '', quote_source: doc.quoteSource || '',
            user_text: doc.userText || '', link_url: doc.linkUrl || '', image_ids: socialImages(doc) } }
        : { comment_id: doc.$id, post_id: doc.postId, evidence: { kind, body: doc.body || '' } };
    } else if (body.target_message) {
      message = await this.repo.get('messages', id(body.target_message));
      if (!message || message.sender_id !== target) reject(404, 'not_found', 'The reported message was not found.');
      await this.service.access(message.room_id, user.$id);
    } else {
      // A report cannot be used as a user-directory oracle. Require an already
      // shared conversation or an exact public username known to the reporter.
      const memberships = await this.repo.list('members', [eq('user_id', user.$id)]);
      let shared = false;
      for (const membership of memberships) if (await this.service.membership(membership.room_id, target)) { shared = true; break; }
      const profile = await this.repo.get('profiles', target);
      if (!shared && !profile?.username) reject(404, 'not_found', 'This person is not available to report.');
    }
    await this.auth.limit('reports:' + user.$id, 10, 3600);
    const report = await this.repo.create('reports', { reporter_id: user.$id, target_user: target,
      reason: body.reason, details, status: 'open', created_at: new Date(this.now()).toISOString(),
      ...(message ? { room_id: message.room_id, message_id: message.id,
        evidence: { body: message.body, attachment_ids: message.attachment_ids || [] } } : {}), ...social });
    return { id: report.id, submitted: true };
  }
  async rulesState(user) {
    const record = await this.repo.get('state', key('community-rules:' + user.$id));
    return { version: RULES_VERSION, accepted_version: record?.version || null };
  }
  async acceptRules(user, body) {
    if (body.version !== RULES_VERSION) reject(400, 'rules_changed', 'Read the current community rules before accepting.');
    const recordId = key('community-rules:' + user.$id), values = { kind: 'community_rules', user_id: user.$id,
      version: RULES_VERSION, accepted_at: new Date(this.now()).toISOString() };
    return this.repo.lock('community-rules:' + user.$id, async () => {
      if (await this.repo.get('state', recordId)) await this.repo.update('state', recordId, values);
      else await this.repo.create('state', values, recordId);
      return { accepted: true, version: RULES_VERSION };
    });
  }
}

// Operator-only entry point. No client RPC routes to these administrative acts.
export async function moderateReport(service, reportId, action, notes = '') {
  if (!['dismiss', 'takedown', 'suspend'].includes(action) || typeof notes !== 'string' || Buffer.byteLength(notes) > 2000)
    reject(400, 'validation', 'Choose dismiss, takedown or suspend, with bounded review notes.');
  const repo = service.repo;
  return repo.lock('moderation:' + id(reportId), async () => {
    const report = await repo.get('reports', reportId);
    if (!report) reject(404, 'not_found', 'Report not found.');
    if (report.status === 'resolved') {
      if (report.action !== action) reject(409, 'already_resolved', 'This report was already resolved differently.');
      return { resolved: true, action };
    }
    if (action === 'suspend') await service.auth.rest.request('PATCH', '/users/' + encodeURIComponent(report.target_user) + '/status', { body: { status: false } });
    if (action === 'takedown' && !report.message_id && (report.post_id || report.comment_id)) {
      const kind = report.evidence?.kind === 'comment' ? 'comments' : 'posts';
      const docId = kind === 'comments' ? report.comment_id : report.post_id;
      const doc = await socialDocument(service.auth.rest, repo, kind, docId);
      if (doc && doc.authorId !== report.target_user) reject(409, 'evidence_changed', 'The reported content no longer matches this report.');
      if (doc) await service.auth.rest.request('DELETE', socialPath(repo, kind, docId));
      // Image IDs come from the immutable evidence, so a retry still removes files.
      if (kind === 'posts') for (const imageId of report.evidence?.image_ids || []) {
        try { await service.auth.rest.request('DELETE', `/storage/buckets/${encodeURIComponent(repo.bucketId)}/files/${encodeURIComponent(imageId)}`); }
        catch (error) { if (error.status !== 404) throw error; }
      }
    } else if (action === 'takedown') {
      if (!report.message_id) reject(400, 'validation', 'A message, post or comment report is required for content takedown. Suspend the account for profile abuse.');
      await repo.lock('room:' + report.room_id, async () => {
        const message = await repo.get('messages', report.message_id);
        if (message && message.sender_id !== report.target_user) reject(409, 'evidence_changed', 'The reported content no longer matches this report.');
        // Keep the report's immutable attachment IDs for retry after an unpublish
        // succeeds but file cleanup fails; owner identity is checked on each row.
        await repo.remove('messages', report.message_id);
        for (const attachmentId of report.evidence?.attachment_ids || []) {
          const photo = await repo.get('attachments', attachmentId);
          if (!photo || photo.message_id !== report.message_id || photo.uploader_id !== report.target_user) continue;
          await repo.removeFile(photo.object_key); await repo.removeFile(photo.thumbnail_key);
          await repo.remove('attachments', photo.id);
        }
      });
    }
    await repo.update('reports', reportId, { ...report, status: 'resolved', action, notes,
      resolved_at: new Date().toISOString() });
    return { resolved: true, action };
  });
}
