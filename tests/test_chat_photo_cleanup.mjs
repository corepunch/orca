import assert from "node:assert/strict";
import { createHandler } from "../apps/chat/backend/supabase/functions/cleanup-chat-photos/handler.mjs";
const secret = "test-only-secret-".repeat(4);
const env = { SUPABASE_URL: "https://test.invalid", SUPABASE_SERVICE_ROLE_KEY: "private-test", CHAT_CLEANUP_SECRET: secret };
const row = { id: "photo-id", room_id: "room-id", status: "discarded", message_id: null,
  object_key: "room-id/photo-id/photo.jpg", thumbnail_key: "room-id/photo-id/thumbnail.jpg" };
function fixture(options = {}) {
  const calls = [];
  const handler = createHandler({ env, fetchImpl: async (url, request) => {
    calls.push({ url, request });
    assert.equal(request.headers.apikey, "private-test");
    if (url.endsWith("/rpc/claim_expired_chat_photos")) return Response.json([options.row || row]);
    if (url.endsWith("/storage/v1/object/chat-photos")) {
      assert.deepEqual(JSON.parse(request.body), { prefixes: [row.object_key, row.thumbnail_key] });
      return Response.json({}, { status: options.storageFails ? 503 : 200 });
    }
    assert(url.includes("&status=eq.discarded&message_id=is.null"), "Metadata deletion must be scoped to unlinked discarded reservations");
    assert.equal(request.method, "DELETE");
    return new Response(null, { status: 204 });
  } });
  return { handler, calls };
}
const request = () => new Request("https://function.invalid", { method: "POST", headers: { Authorization: "Bearer " + secret } });
let test = fixture();
let response = await test.handler(request());
assert.deepEqual(await response.json(), { claimed: 1, deleted: 1, failed: 0 });
assert.equal(test.calls.length, 3);
test = fixture({ storageFails: true });
response = await test.handler(request());
assert.equal(response.status, 503);
assert.equal(test.calls.length, 2, "Keep metadata until physical storage deletion succeeds, so a later job can retry");
test = fixture({ row: { ...row, status: "attached", message_id: "message-id" } });
assert.equal((await test.handler(request())).status, 503);
assert.equal(test.calls.length, 1, "Never delete a live message attachment");
test = fixture();
assert.equal((await test.handler(new Request("https://function.invalid", { method: "POST",
  headers: { Authorization: "Bearer user-session" } }))).status, 401);
assert.equal(test.calls.length, 0, "User sessions cannot authorize administrative cleanup");
console.log("PASS: photo cleanup (service authorization, byte-first deletion, retries and live-attachment protection)");
