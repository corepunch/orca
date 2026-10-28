# Loading image previews

Use this guide for remote photos, staged preview callbacks, cache identity and
slow-image investigations. Engine API comes from generated references; current
app behavior comes from the owning loader and tests. External capabilities below
were checked on 2026-10-28; confirm plan/version details before relying on them.

## Choose the delivery path before the codec

Thumbnail → full means two complete encoded images. Each can be decoded with
the existing decoder; it does not require incremental decoding or JPEG 2000.
Do not migrate uploads or add a decoder merely to display a smaller image first.

`stb_image` supports baseline and progressive JPEG, but has no JPEG 2000 decoder.
Its `read`/`skip`/`eof` callbacks supply encoded input to a synchronous decode;
they are not notifications that a lower-quality decoded frame is ready. The
public load API returns a pixel buffer or failure, without a resumable scan/frame
interface. Check the bundled version and compile flags before assuming that a
format accepted by a server is accepted by ORCA. See the
[upstream header](https://github.com/nothings/stb/blob/master/stb_image.h).

The application paths differ:

| Application | Preview and final delivery | Ownership |
| --- | --- | --- |
| Chat | Stored thumbnail and full JPEG, decrypted by the authenticated Chat Function | Private temporary files, scoped to the app/session |
| Octopus | Appwrite 96-pixel JPEG at quality 35, then the selected photo variant/original fallback | Temporary previews; completed guest variants may use the bounded persistent cache |

Chat encrypts bytes **before** storing them in the shared bucket. Storage sees
ciphertext, so a Storage image transformation cannot replace its decrypting
Function. Appwrite's own bucket encryption at rest is a separate, transparent
storage feature. Preserve the existing route rather than making Chat objects
public to obtain thumbnails. See [Chat storage](../../apps/chat/README.md) and
[Appwrite buckets](https://appwrite.io/docs/products/storage/buckets).

## Diagnose time to the first usable preview

Measure the stages separately: queue wait, metadata, thumbnail transfer, decode,
and first paint. A fast byte download does not prove that the preview was drawn.
Record timings and request identity without logging credentials or private URLs.

- Reuse attachment metadata already supplied with messages. Chat's
  `getPhoto(attachment, thumbnail, onPreview)` can use known object keys;
  string IDs and incomplete metadata retain the lookup path. Each object read
  still performs server authorization. Avoid a second metadata request per stage.
- Prioritize queued visible thumbnails over queued full images. Holding both
  download slots through thumbnail **and** full can leave subsequent visible
  photos blank. Chat separates these stages within one deduplicated request;
  Octopus currently keeps one slot for its callback-based pair. Do not describe
  these schedulers as identical, or increase concurrency before identifying the
  bottleneck. Priority does not preempt transfers already running.
- Check session, request generation, live target registration and visibility
  before a deferred worker starts networking and again before applying results.
  Removing one subscriber does not cancel work still needed by another.
- Paint a usable preview immediately. A wrapper that repaints only after its
  coroutine finishes can conceal the preview until the full image arrives.
  Request a paint from the preview update using the app's existing repaint path.

Relevant implementations are [Chat loader](../../apps/chat/photo_loader.lua),
[Chat viewer](../../apps/chat/views/photo_viewer.lua),
[photo API](../../apps/chat/api/features.lua) and
[Octopus loader](../../apps/octopus/photo_loader.lua).

## Keep the two stages distinct

Use separate immutable paths and cache identities for preview and completed
images. Include the source revision and image variant; partition private data
by its session/project ownership. A thumbnail must not satisfy a full-image cache
lookup or invoke a final-completion callback. Retry can reuse a usable preview.

Decode the candidate **before** clearing the previous image. JPEG markers or an
HTTP success status alone do not prove decodability. In the existing ImageView
path, send `Node.LoadView` and check `Source`. A corrupt final JPEG must leave a
decoded preview visible with retry, rather than replacing it with empty content.
An optional thumbnail or consumer callback failure must still allow the full
image; an explicit callback `false` cancels remaining work under the API contract.

Reserve layout from original dimensions when known. A low-resolution image's
rounded dimensions should not change the message frame, scroll anchor or crop.
Use the app's established loading icon; remove it when the image appears and
keep navigation and photo hit targets usable. Chat currently uses a centered
packaged Lucide icon for pending timeline photos.

## Appwrite transformation details

Appwrite's preview endpoint returns a separate transformed image. Supplying one
of width/height preserves aspect ratio; supplying both requests a crop. Choose
explicit output compatible with the client, such as `output=jpg`, rather than
assuming all server formats have local decoders. JP2 is not listed as an output.
Appwrite caches transformed results; use a small stable set of variant sizes
and qualities rather than generating a new URL for every layout fluctuation.
That server cache does not replace app cache ownership or stale-result checks.
See [image transformations](https://appwrite.io/docs/products/storage/images).

Transformation availability and limits depend on deployment. The current Cloud
documentation excludes transformations from Free; self-hosted preview limits
are configurable. The CLI reference still describes a 10 MB limit while the
self-hosted default is 20 MB. Do not encode either as a universal service limit.
Keep a compatible original/stored-thumbnail fallback and avoid retrying an
unavailable optional transform on every paint. See
[Cloud billing](https://appwrite.io/docs/advanced/billing/image-transformations),
[configuration](https://appwrite.io/docs/advanced/self-hosting/configuration/environment-variables)
and [CLI reference](https://appwrite.io/docs/tooling/command-line/buckets).

File tokens grant access without the usual session; creating a token is a
sharing decision, not a speed optimization. Bucket and file read permissions
are alternatives: a restrictive file permission does not override a public
bucket grant. Preserve the existing access model when tuning delivery. See
[file tokens](https://appwrite.io/docs/products/storage/file-tokens) and
[permissions](https://appwrite.io/docs/products/storage/permissions).

## Additional native options, when needed

Decoded memory depends on pixel dimensions, not compressed file length. An
8-bit RGBA buffer needs roughly `width * height * 4` bytes before additional
decode/texture allocations. For large images displayed small, investigate native
downsampling instead of decoding at full size and then resizing. Apple's ImageIO
thumbnail API supports this; it is a native implementation option, not an
existing promise of ORCA's `ImageView`. See
[Apple's image memory guidance](https://developer.apple.com/videos/play/wwdc2018/416/).

For a future single-transfer incremental decoder on Apple platforms, investigate
`CGImageSourceCreateIncremental`, `CGImageSourceUpdateData` and image status.
UpdateData takes **all accumulated bytes**, not only the latest chunk. A partial
image is usable only when enough data has arrived; final completion is separate.
This still needs chunk delivery from the transport and a renderer update path;
it is not supplied by a Lua download-progress callback. Treat it as a separate
platform feature, with the current portable thumbnail path as fallback. See
[incremental ImageIO loading](https://developer.apple.com/library/archive/documentation/GraphicsImaging/Conceptual/ImageIOGuide/imageio_source/ikpg_source.html)
and [UpdateData](https://developer.apple.com/documentation/imageio/cgimagesourceupdatedata(_:_:_:)).

## Focused validation and build provenance

Suspend the full response and assert that a decoded preview is already attached
and a paint was requested. With more visible images than slots, assert thumbnail
priority across different records. Cover duplicate subscribers, missing/corrupt
thumbnails, corrupt finals retaining good previews, retry, and logout/detach
before a queued worker starts. Assert that preview callbacks neither complete
the operation nor populate the final cache.

Use the smallest affected offline suites: `tests/test_chat_api.lua`,
`tests/test_chat_appwrite.lua`, `tests/test_chat_photo_loader.lua`,
`tests/test_chat_photo_viewer.lua`, `tests/test_chat_timeline.lua`, or the matching
Octopus API/photo tests. Follow [Testing](../TESTING.md); live backend checks
remain explicitly requested work.

If an animated placeholder makes an event-queue drain hang, bound the run and
inspect pending work and binary/source compatibility before changing unrelated
UI behavior. A retained visible animation can keep producing work; a mock's
successful return is not proof that the queue is idle. Record the actual binary
used, particularly when testing a clean worktree with another checkout's binary.

A merged PR and passing desktop tests do not identify the source packaged in a
TestFlight build. Check archive/build provenance when available; otherwise state
that the installed build was not verified. Publishing new source does not update
an already installed app or publish a replacement build.
