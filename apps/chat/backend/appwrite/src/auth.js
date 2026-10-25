import { createHmac, randomBytes, randomInt, randomUUID, timingSafeEqual } from 'node:crypto';
import { key, reject } from './rest.js';

const enc = encodeURIComponent;
export function identifier(value) {
  if (typeof value !== 'string') reject(400, 'validation', 'Use an email or international phone number.');
  value = value.trim();
  if (value.startsWith('+')) {
    value = value.replace(/[\s()-]/g, '');
    if (/^\+[1-9]\d{7,14}$/.test(value)) return { field: 'phone', value };
  } else if (value.length <= 254 && /^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(value)) return { field: 'email', value: value.toLowerCase() };
  reject(400, 'validation', 'Use an email or international phone number.');
}
export function publicUser(user) {
  return { id: user.$id, email: user.emailVerification ? user.email : null, phone: user.phoneVerification ? user.phone : null,
    user_metadata: { display_name: user.name || '' } };
}

export class Auth {
  constructor(rest, repository, { now = Date.now, otpHmacKey, emailProviderId, smsProviderId } = {}) {
    this.rest = rest; this.repo = repository; this.now = now; this.otpHmacKey = otpHmacKey;
    this.providers = { email: emailProviderId, phone: smsProviderId };
  }
  async limit(name, maximum, seconds) {
    const bucket = Math.floor(this.now() / (seconds * 1000)), start = randomInt(maximum);
    // Unique immutable claims enforce the quota without a global lock that a killed
    // invocation could leave held. Random starts avoid serial contention on slot zero.
    for (let offset = 0; offset < maximum; offset++) {
      const slot = (start + offset) % maximum, id = key(`rate:${name}:${bucket}:${slot}`);
      try {
        await this.repo.create('state', { kind: 'rate_claim', name, bucket, slot, expires_at: (bucket + 1) * seconds * 1000 }, id);
        return;
      } catch (error) { if (error.status !== 409) throw error; }
    }
    reject(429, 'rate_limit', 'Too many attempts. Please wait before trying again.');
  }
  async account(jwt) {
    if (typeof jwt !== 'string' || !jwt || jwt.length > 4096) reject(401, 'unauthenticated', 'Sign in to continue.');
    return this.rest.request('GET', '/account', { admin: false, jwt });
  }
  digest(salt, code) { return createHmac('sha256', this.otpHmacKey).update(salt + ':' + code).digest('hex'); }
  async removeTarget(userId, targetId) {
    try { await this.rest.request('DELETE', `/users/${enc(userId)}/targets/${enc(targetId)}`); }
    catch (error) { if (error.status !== 404) throw error; }
  }
  async requestLink(user, body) {
    const who = identifier(body.email || body.phone), id = key('link:' + user.$id);
    if (!this.otpHmacKey || this.otpHmacKey.length < 32 || !this.providers[who.field]) {
      reject(503, 'configuration', 'Verified identity linking needs an Appwrite Messaging provider and server OTP secret.');
    }
    return this.repo.lock('identity:' + user.$id, async () => {
      const old = await this.repo.get('state', id);
      if (old && old.sent_at + 60000 > this.now()) reject(429, 'rate_limit', 'Wait one minute before requesting another code.');
      await this.limit('link:' + user.$id, 10, 3600); await this.limit('link-target:' + who.value, 10, 3600);
      if (old?.target_id) await this.removeTarget(user.$id, old.target_id);
      // Reject identities belonging to another account before sending, and recheck uniqueness at update.
      const params = new URLSearchParams(); params.append('queries[]', JSON.stringify({ method: 'equal', attribute: who.field, values: [who.value] }));
      const existing = await this.rest.request('GET', '/users?' + params);
      if (existing.users.some(other => other.$id !== user.$id)) reject(409, 'identity_exists', 'That identifier belongs to another account.');
      const targetId = randomUUID(), code = String(randomInt(1000000)).padStart(6, '0'), salt = randomBytes(16).toString('hex');
      const row = { kind: 'link', field: who.field, identifier: who.value, user_id: user.$id, target_id: targetId,
        sent_at: this.now(), expires_at: this.now() + 300000, attempts: 0, salt, digest: this.digest(salt, code), delivered: false };
      if (old) await this.repo.update('state', id, row); else await this.repo.create('state', row, id);
      try {
        await this.rest.request('POST', `/users/${enc(user.$id)}/targets`, { body: { targetId, providerType: who.field === 'email' ? 'email' : 'sms',
          identifier: who.value, providerId: this.providers[who.field], name: 'Chat identity verification' } });
        const content = `Your Chat verification code is ${code}. It expires in 5 minutes.`;
        await this.rest.request('POST', '/messaging/messages/' + (who.field === 'email' ? 'email' : 'sms'), {
          body: { messageId: randomUUID(), targets: [targetId], content, ...(who.field === 'email' ? { subject: 'Verify your Chat email' } : {}) },
        });
        await this.repo.update('state', id, { ...row, delivered: true });
      } catch (error) {
        // Keep the private challenge if cleanup fails so a retry can remove its target.
        await this.removeTarget(user.$id, targetId); await this.repo.remove('state', id); throw error;
      }
      return { identifier: who.value, delivery: who.field, expires_in: 300, retry_after: 60 };
    });
  }
  async verifyLink(user, body) {
    const who = identifier(body.email || body.phone), id = key('link:' + user.$id);
    if (!/^\d{6}$/.test(body.token || '')) reject(400, 'validation', 'Enter the six-digit verification code.');
    return this.repo.lock('identity:' + user.$id, async () => {
      const row = await this.repo.get('state', id);
      if (!row || !row.delivered || row.field !== who.field || row.identifier !== who.value || row.expires_at <= this.now() || row.attempts >= 5) {
        if (row?.target_id && (row.expires_at <= this.now() || row.attempts >= 5)) {
          await this.removeTarget(user.$id, row.target_id); await this.repo.remove('state', id);
        }
        reject(400, 'invalid_code', 'Request a new verification code.');
      }
      row.attempts++; await this.repo.update('state', id, row);
      const actual = Buffer.from(this.digest(row.salt, body.token), 'hex'), expected = Buffer.from(row.digest, 'hex');
      if (!timingSafeEqual(actual, expected)) reject(400, 'invalid_code', 'The verification code is incorrect.');
      // Consume proof before changing Auth: a failed verification update cannot replay an old code.
      await this.repo.update('state', id, { ...row, delivered: false });
      await this.rest.request('PATCH', `/users/${enc(user.$id)}/${who.field}`, { body: { [who.field]: who.value } });
      await this.rest.request('PATCH', `/users/${enc(user.$id)}/verification${who.field === 'phone' ? '/phone' : ''}`, { body: { [who.field + 'Verification']: true } });
      await this.removeTarget(user.$id, row.target_id);
      await this.repo.update('state', id, { kind: 'link_cooldown', sent_at: row.sent_at, expires_at: row.expires_at });
      return { user: publicUser(await this.rest.request('GET', `/users/${enc(user.$id)}`)) };
    });
  }
}
