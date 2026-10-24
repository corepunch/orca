import { inspectJpeg } from "./jpeg.mjs";

const json = (body, status = 200) => Response.json(body, { status });
const fail = (code, message, status) => json({ code, message }, status);
function key(env, modern, legacy) {
  try { return JSON.parse(env[modern] || "{}").default || env[legacy]; }
  catch { return env[legacy]; }
}

// Dependencies are explicit so authorization and storage failures can be tested
// with local fixtures; tests never need a Supabase account or network connection.
export function createHandler({ env, fetchImpl = fetch, now = Date.now }) {
  const url = env.SUPABASE_URL?.replace(/\/$/, "");
  const publicKey = key(env, "SUPABASE_PUBLISHABLE_KEYS", "SUPABASE_ANON_KEY");
  const adminKey = key(env, "SUPABASE_SECRET_KEYS", "SUPABASE_SERVICE_ROLE_KEY");
  const adminHeaders = { apikey: adminKey, "Content-Type": "application/json" };
  if (adminKey?.startsWith("eyJ")) adminHeaders.Authorization = "Bearer " + adminKey;
  // Secret-format API keys belong only in apikey; legacy role JWTs also work
  // through that header. Auth is independently checked using the caller's JWT.
  async function boundedBytes(response, expected, limit = 1000000) {
    if (!response.ok || !response.body || (expected !== null && (expected < 1 || expected > limit))) throw new Error("missing_photo");
    const reader = response.body.getReader(), chunks = [];
    let size = 0;
    while (true) {
      const { done, value } = await reader.read();
      if (done) break;
      size += value.byteLength;
      if ((expected !== null && size > expected) || size > limit) { await reader.cancel(); throw new Error("invalid_photo"); }
      chunks.push(value);
    }
    if (!size || (expected !== null && size !== expected)) throw new Error("invalid_photo");
    const bytes = new Uint8Array(size);
    let offset = 0;
    for (const chunk of chunks) { bytes.set(chunk, offset); offset += chunk.length; }
    return bytes;
  }
  return async function handle(request) {
    if (request.method !== "POST") return fail("method_not_allowed", "Use POST.", 405);
    if (!url || !publicKey || !adminKey) return fail("configuration", "Photo storage is not configured.", 503);
    const authorization = request.headers.get("Authorization");
    if (!authorization?.match(/^Bearer \S+$/i)) return fail("unauthenticated", "Sign in to continue.", 401);
    try {
      const auth = await fetchImpl(url + "/auth/v1/user", { headers: { apikey: publicKey, Authorization: authorization } });
      if (!auth.ok) return fail("unauthenticated", "Sign in to continue.", 401);
      const user = await auth.json();
      if (!user?.id) return fail("unauthenticated", "Sign in to continue.", 401);
      // A finalization request is tiny; avoid parsing an unbounded JSON body.
      let raw;
      try { raw = await boundedBytes(new Response(request.body), null, 1024); }
      catch { return fail("validation", "Choose a valid photo.", 400); }
      let body;
      try { body = JSON.parse(new TextDecoder().decode(raw)); }
      catch { return fail("validation", "Choose a valid photo.", 400); }
      const id = body?.attachment_id;
      if (raw.length > 1024 || typeof id !== "string" || !/^[0-9a-f]{8}-(?:[0-9a-f]{4}-){3}[0-9a-f]{12}$/i.test(id)) {
        return fail("validation", "Choose a valid photo.", 400);
      }
      const rows = await fetchImpl(url + "/rest/v1/chat_attachments?select=*&id=eq." + encodeURIComponent(id), { headers: adminHeaders });
      if (!rows.ok) return fail("storage", "Could not load the photo reservation.", 503);
      const [photo] = await rows.json();
      if (!photo || photo.uploader_id !== user.id) return fail("forbidden", "This photo is unavailable.", 403);
      if (!["pending", "ready"].includes(photo.status) || photo.message_id
          || !Number.isFinite(Date.parse(photo.created_at)) || now() - Date.parse(photo.created_at) > 3600000) {
        return fail("invalid_state", "This photo reservation has expired or is already used.", 409);
      }
      const prefix = photo.room_id + "/" + photo.id + "/";
      if (photo.object_key !== prefix + "photo.jpg" || photo.thumbnail_key !== prefix + "thumbnail.jpg") {
        return fail("invalid_photo", "The photo reservation is invalid.", 422);
      }
      for (const [object, expected, width, height, edge] of [
        [photo.object_key, photo.byte_count, photo.width, photo.height, 1600],
        [photo.thumbnail_key, photo.thumbnail_byte_count, photo.thumbnail_width, photo.thumbnail_height, 256],
      ]) {
        if (!Number.isInteger(expected) || !Number.isInteger(width) || !Number.isInteger(height)) throw new Error("invalid_photo");
        const response = await fetchImpl(url + "/storage/v1/object/authenticated/chat-photos/" + object, { headers: adminHeaders });
        const bytes = await boundedBytes(response, expected);
        let actual;
        try { actual = inspectJpeg(bytes, edge); } catch { throw new Error("invalid_photo"); }
        if (actual.width !== width || actual.height !== height) throw new Error("invalid_photo");
      }
      // The service-only RPC locks the room/reservation and checks current
      // membership/publishing permission again, closing the upload/removal race.
      const ready = await fetchImpl(url + "/rest/v1/rpc/mark_chat_photo_ready", {
        method: "POST", headers: adminHeaders, body: JSON.stringify({ attachment_id: id, uploader: user.id }),
      });
      if (!ready.ok) {
        const error = await ready.json().catch(() => ({}));
        return fail("forbidden", error.message || "You cannot publish in this conversation.", 403);
      }
      return json(await ready.json());
    } catch (error) {
      if (["invalid_photo", "missing_photo"].includes(error?.message)) {
        return fail("invalid_photo", "The uploaded JPEG does not match its reservation. Choose the photo again.", 422);
      }
      return fail("storage", "Could not finalize the photo. Please try again.", 503);
    }
  };
}
