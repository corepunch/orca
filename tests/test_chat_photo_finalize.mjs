import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { inspectJpeg } from "../apps/chat/backend/supabase/functions/finalize-chat-photo/jpeg.mjs";
import { createHandler } from "../apps/chat/backend/supabase/functions/finalize-chat-photo/handler.mjs";

const bytes = new Uint8Array(await readFile(new URL("./fixtures/chat/photo.jpg", import.meta.url)));
assert.deepEqual(inspectJpeg(bytes, 1600), { width: 32, height: 24 });
assert.throws(() => inspectJpeg(bytes, 16));
assert.throws(() => inspectJpeg(bytes.slice(0, -2), 1600));
assert.throws(() => inspectJpeg(new Uint8Array([255, 216, 255, 217]), 1600));
assert.throws(() => inspectJpeg(new Uint8Array(1000001), 1600));
const gps = Uint8Array.from([69,120,105,102,0,0,77,77,0,42,0,0,0,8,0,1,
  0x88,0x25,0,4,0,0,0,1,0,0,0,26,0,0,0,0]);
const privateJpeg = new Uint8Array(bytes.length + gps.length + 4);
privateJpeg.set(bytes.subarray(0, 2)); privateJpeg.set([255, 225, 0, gps.length + 2], 2);
privateJpeg.set(gps, 6); privateJpeg.set(bytes.subarray(2), gps.length + 6);
assert.throws(() => inspectJpeg(privateJpeg, 1600), "Location-bearing source metadata must be rejected");

const id = "00000000-0000-4000-a000-000000000001";
const room = "00000000-0000-4000-a000-000000000002";
const now = Date.parse("2026-10-24T12:00:00Z");
const original = {
  id, room_id: room, uploader_id: "me", status: "pending", message_id: null,
  created_at: new Date(now - 1000).toISOString(),
  object_key: `${room}/${id}/photo.jpg`, thumbnail_key: `${room}/${id}/thumbnail.jpg`,
  byte_count: bytes.length, thumbnail_byte_count: bytes.length,
  width: 32, height: 24, thumbnail_width: 32, thumbnail_height: 24,
};
const env = { SUPABASE_URL: "https://test.invalid", SUPABASE_PUBLISHABLE_KEYS: '{"default":"public-test"}',
  SUPABASE_SECRET_KEYS: '{"default":"private-test"}' };
function backend(overrides = {}) {
  const calls = [];
  const photo = { ...original, ...overrides.photo };
  const handler = createHandler({ env, now: () => now, fetchImpl: async (url, options) => {
    calls.push({ url, options });
    if (url.endsWith("/auth/v1/user")) {
      assert.equal(options.headers.apikey, "public-test");
      assert.equal(options.headers.Authorization, "Bearer user-token");
      return Response.json({ id: "me" }, { status: overrides.authStatus || 200 });
    }
    assert.equal(options.headers.apikey, "private-test", "Administrative operations stay in the trusted handler");
    assert.equal(options.headers.Authorization, undefined, "Modern API keys are not user JWTs");
    if (url.includes("/rest/v1/chat_attachments?")) return Response.json([photo]);
    if (url.includes("/storage/v1/object/")) return new Response(overrides.bytes || bytes);
    if (url.endsWith("/rpc/mark_chat_photo_ready")) {
      assert.deepEqual(JSON.parse(options.body), { attachment_id: id, uploader: "me" });
      return overrides.permissionLost ? Response.json({ message: "Membership changed" }, { status: 403 })
        : Response.json({ ...photo, status: "ready" });
    }
    assert.fail("Unexpected backend request " + url);
  } });
  return { handler, calls };
}
function request(body = { attachment_id: id }) {
  // No Content-Length: streamed/chunked HTTP callers must work as well.
  return new Request("https://function.invalid", { method: "POST",
    headers: { Authorization: "Bearer user-token" }, body: JSON.stringify(body) });
}
let test = backend();
let result = await test.handler(request());
assert.equal(result.status, 200);
assert.equal((await result.json()).status, "ready");
assert.equal(test.calls.length, 5, "Verify Auth, reservation, both images, then commit");
for (const overrides of [
  { photo: { uploader_id: "someone-else" } },
  { photo: { status: "discarded" } },
  { photo: { created_at: new Date(now - 3600001).toISOString() } },
  { photo: { object_key: "other-room/photo.jpg" } },
  { photo: { width: 123 } },
  { photo: { byte_count: bytes.length - 1 } },
  { bytes: bytes.slice(0, -2) },
]) {
  test = backend(overrides);
  result = await test.handler(request());
  assert(result.status >= 400 && result.status < 500);
  assert(!test.calls.some(call => call.url.endsWith("mark_chat_photo_ready")), "Rejected photos must never become ready");
}
test = backend({ permissionLost: true });
assert.equal((await test.handler(request())).status, 403, "Final commit must recheck publishing permission");
test = backend({ authStatus: 401 });
assert.equal((await test.handler(request())).status, 401);
assert.equal(test.calls.length, 1, "Invalid user credentials must stop before privileged access");
test = backend();
assert.equal((await test.handler(request({ attachment_id: "bad" }))).status, 400);
assert.equal((await test.handler(new Request("https://function.invalid", { method: "POST" }))).status, 401);
assert.equal((await test.handler(request({ padding: "x".repeat(2048) }))).status, 400);
console.log("PASS: trusted photo finalizer (JPEG structure, dimensions, size, identity, expiry and atomic permission checks)");
