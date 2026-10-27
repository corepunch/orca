# TODO

- [ ] Profile `apps/chat` swipes on a physical device and record frame times,
  layout cost, and dropped frames before claiming smoothness. Windows Phone 7
  emphasized GPU/composition-thread animation and minimal UI-thread work
  ([Microsoft reference](https://download.microsoft.com/download/3/1/b/31b6c512-d66f-4138-b639-94711e95faca/msdn_0411dg.pdf)).
  ORCA still performs layout during animation; use the device profile to guide
  reducing that work and confirm the resulting swipe behavior is smooth.

## Chat: complete live backend testing

The manual live suite (`make test-live`, see
[`apps/chat/backend/appwrite/live`](apps/chat/backend/appwrite/live/README.md))
has only run against the offline emulator so far. Remaining work to test the
deployed backend end to end:

- [ ] **First real run.** Run `make test-live-setup` and `make test-live`
  against Octopus, then fix or record every real failure.
- [ ] **Redeploy `orca-chat-api`.** The active deployment `6add961c522a0e94c9b8`
  predates `peer_id` and the `edit_chat_message`/`delete_chat_message` RPCs.
  Record the new deployment ID and archive hash in the backend README, then run
  `CHAT_LIVE_ONLY=direct,messages,photos make test-live`.
- [ ] **Staging project.** Live runs currently write to production Octopus.
  Create a separate Appwrite project with the same `schema.json`, function and
  variables. Make it the default live target (`CHAT_LIVE_PROJECT_ID`) and keep
  production runs as a short smoke check.
- [ ] **Scoped server key for cleanup.** Create a dedicated `APPWRITE_API_KEY`
  with `documents.read/write`, `files.write` and `users.read/write`, stored
  outside the repo. Without it, runs leave conversations behind and the
  verified-email discovery test is skipped.
- [ ] **Scheduled run.** Add a `workflow_dispatch` plus weekly scheduled GitHub
  Actions job, not part of PR CI. It runs the suite against staging using the
  accounts file and the key as secrets, then uploads the report.
- [ ] **Delivery providers.** Configure Appwrite Messaging email and SMS
  providers and `CHAT_OTP_HMAC_KEY`. Then add live tests for identity linking,
  email OTP sign-in and phone OTP sign-in. These need a readable test inbox (a
  catch-all domain or a mail-testing service) and test phone numbers (check
  Auth mock numbers or the SMS provider's test credentials). None of these
  flows has been verified live.
- [ ] **Run the real Lua client live.** The Node suite mirrors the Lua paths but
  doesn't run them. Add an opt-in ORCA test (`tests/test_chat_live.lua`, skipped
  unless `CHAT_LIVE=1`) that drives `api.chat` through `api.orca_transport` and
  checks native HTTP, Account cookie extraction, JWT renewal and binary photo
  bytes. Then do a manual pass on an iOS device: sign in, find a friend, chat,
  send a photo, edit, delete, and check Keychain restore after relaunch.
- [ ] **Real photos.** Live photo tests upload structural JPEGs. Add one
  real photo prepared by `media.processPhoto` (macOS) so decode and display are
  verified with the deployed function.
- [ ] **Limits that need time or more accounts.** These need the server key, to
  backdate documents or create disposable users through the Users API:
  - invitation exhaustion (10 uses) and 24-hour expiry;
  - photo reservation expiry and lazy cleanup (1 hour), and the limit of 20
    unfinished uploads;
  - a 50-member group;
  - the 10,000-row query cap.
- [ ] **Concurrency and locks.** Post to one room from several accounts in
  parallel. Check that `busy` retries succeed and that no `kind=lock` rows stay
  in `chat_state` after the run. Rehearse the stuck-lock recovery steps.
- [ ] **Latency budget.** Synchronous executions have a hard 30-second limit.
  Record each execution's `duration`, including cold starts, and warn when p95
  exceeds a few seconds. `listRooms` has no page limit and slows down as rooms
  accumulate.
- [ ] **Account deletion.** No flow or test covers removing a user's chat data
  (memberships, messages, photos) when their account is deleted.

## Chat: Appwrite best practices to adopt

- [ ] **Move from the Documents API to TablesDB.** Appwrite deprecated the
  document-based methods, which now get only security fixes. New work should use
  tables and rows. `Repository` in `src/rest.js` and `schema.json` still use
  collections and documents
  ([announcement](https://appwrite.io/blog/post/product-update-august-2025)).
- [ ] **Use transactions and atomic operators instead of lock documents.**
  TablesDB transactions commit several row operations atomically with
  rollback ([docs](https://appwrite.io/docs/products/databases/tablesdb/transactions)),
  and operators update values at the storage layer
  ([docs](https://appwrite.io/docs/products/databases/tablesdb/operators)).
  Good candidates:
  - creating a room with its memberships;
  - posting a message with its attachment updates;
  - deleting a message with its photos;
  - invite `uses`;
  - rate-limit counters.

  Without transactions, a killed execution can leave a fail-closed room lock
  that needs manual recovery.
- [ ] **Plan permissions for Realtime before replacing polling.** Realtime only
  delivers rows the subscriber can read. Chat tables currently have no client
  permissions, so subscriptions would receive nothing. Permissions don't cascade
  through relationships, so each message row would need explicit member read
  permissions, or the design needs a per-room team. Subscribe to the narrowest
  channel and unsubscribe when leaving a thread
  ([guide](https://appwrite.io/blog/post/common-appwrite-mistakes-and-how-to-avoid-them)).
- [ ] **Push notifications through Appwrite Messaging.** Add APNs and FCM
  providers and register a push target at sign-in. Update the target when the
  device token rotates, and target users by ID for messages
  ([docs](https://appwrite.io/docs/products/messaging/send-push-notifications)).
- [ ] **Local function runtime for integration tests.** Use `appwrite run
  function` (Docker, hot reload) with development-only `.env` values, not
  `--with-variables`, which pulls production secrets
  ([docs](https://appwrite.io/docs/products/functions/develop-locally)).
  Alternatively, run a disposable self-hosted Appwrite in Docker
  (`docker compose down -v` afterwards). Either one exercises the real runtime,
  indexes and unique constraints, which `live/emulator.js` only approximates.
- [ ] **Schema as code.** Keep `appwrite.config.json` and the schema in Git and
  apply them with the CLI (`pull`/`push`) from CI to staging, then production.
  A column rename is a delete plus create, so add the new field, backfill it,
  then remove the old one
  ([guide](https://appwrite.io/blog/post/how-to-execute-database-migration-with-appwrite-cli)).
- [ ] **Keep the function's scopes minimal and stay within the sync limit.**
  Dynamic keys inherit every scope the function is granted, project-wide. Move
  slow work (large rooms, photo cleanup) to asynchronous executions or scheduled
  functions instead of the 30-second synchronous path
  ([dynamic keys](https://appwrite.io/blog/post/how-to-leverage-dynamic-api-keys-for-better-security),
  [functions](https://appwrite.io/blog/post/serverless-functions-best-practices)).
- [ ] **Firewall rate limits.** Add Appwrite Firewall rules limited by user ID
  for function executions and OTP sends, in addition to the function's own
  claim-based limits
  ([use cases](https://appwrite.io/blog/post/appwrite-firewall-use-cases)).
- [ ] **A private bucket for chat photos.** Ciphertext lives in the public
  `post-images` bucket because the plan quota blocked a new bucket. When the
  quota allows, move it to a private bucket with file security, keeping the
  encryption.
