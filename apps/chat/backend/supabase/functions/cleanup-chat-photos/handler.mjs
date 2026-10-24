function key(env, modern, legacy) {
  try { return JSON.parse(env[modern] || "{}").default || env[legacy]; }
  catch { return env[legacy]; }
}

export function createHandler({ env, fetchImpl = fetch }) {
  const url = env.SUPABASE_URL?.replace(/\/$/, "");
  const admin = key(env, "SUPABASE_SECRET_KEYS", "SUPABASE_SERVICE_ROLE_KEY");
  const secret = env.CHAT_CLEANUP_SECRET;
  const headers = { apikey: admin, "Content-Type": "application/json" };
  if (admin?.startsWith("eyJ")) headers.Authorization = "Bearer " + admin;
  return async request => {
    if (request.method !== "POST") return Response.json({ code: "method_not_allowed" }, { status: 405 });
    if (!url || !admin || !secret || secret.length < 32) {
      return Response.json({ code: "configuration" }, { status: 503 });
    }
    if (request.headers.get("Authorization") !== "Bearer " + secret) {
      return Response.json({ code: "unauthenticated" }, { status: 401 });
    }
    let claimed = 0, deleted = 0, failed = 0;
    try {
      const claim = await fetchImpl(url + "/rest/v1/rpc/claim_expired_chat_photos", {
        method: "POST", headers, body: JSON.stringify({ batch_limit: 100 }),
      });
      if (!claim.ok) throw new Error("claim");
      const photos = await claim.json();
      if (!Array.isArray(photos) || photos.length > 100) throw new Error("claim");
      claimed = photos.length;
      for (const photo of photos) {
        try {
          const prefix = `${photo.room_id}/${photo.id}/`;
          if (photo.status !== "discarded" || photo.message_id || photo.object_key !== prefix + "photo.jpg"
              || photo.thumbnail_key !== prefix + "thumbnail.jpg") throw new Error("reservation");
          const removed = await fetchImpl(url + "/storage/v1/object/chat-photos", {
            method: "DELETE", headers, body: JSON.stringify({ prefixes: [photo.object_key, photo.thumbnail_key] }),
          });
          if (!removed.ok) throw new Error("storage");
          // Storage API deletion removes the bytes too. Direct SQL deletion of
          // storage.objects would leave physical files behind.
          const metadata = await fetchImpl(url + "/rest/v1/chat_attachments?id=eq." + encodeURIComponent(photo.id)
            + "&status=eq.discarded&message_id=is.null", { method: "DELETE", headers });
          if (!metadata.ok) throw new Error("metadata");
          deleted++;
        } catch { failed++; }
      }
      return Response.json({ claimed, deleted, failed }, { status: failed ? 503 : 200 });
    } catch {
      return Response.json({ code: "cleanup", claimed, deleted, failed }, { status: 503 });
    }
  };
}
