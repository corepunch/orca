# Backend setup and deployment

No Supabase account, project, database, bucket, provider subscription, or cloud
function has been created by this work. The runnable default is the isolated
offline mock. A live service requires a project owned by you or your organization.

## Why Supabase here

Supabase fits this app's existing Lua HTTP client and relational data: accounts,
room memberships, roles, invitations, messages, and attachments. Postgres
transactions enforce operations such as canonical direct-chat creation and
publishing a message with its validated photos. RLS and private storage enforce
access outside the UI. Its [phone login](https://supabase.com/docs/guides/auth/phone-login)
requires a configured SMS provider; it does not include a free SMS delivery
service automatically.

Firebase is a viable alternative, especially when adopting its mobile SDKs and
[Firestore offline caching/listeners](https://firebase.google.com/docs/firestore/manage-data/enable-offline).
Those benefits would require an ORCA native SDK integration; using REST alone
does not provide the SDK's offline synchronization. Its
[Apple phone-auth flow](https://firebase.google.com/docs/auth/ios/phone-auth)
also needs app verification integration. For this Lua application, retain
Supabase and the shared client/model boundary. The local mock is interchangeable
with that client; a Firebase backend would need a separate protocol adapter,
not just a different project URL.

## Create and configure a live project

1. Create an account and project in the [Supabase dashboard](https://supabase.com/dashboard).
   Choose an appropriate hosting region and keep the database password private.
2. Run these SQL files in order using the project's SQL editor:
   [schema.sql](schema.sql), [002_contacts.sql](002_contacts.sql), and
   [003_messenger.sql](003_messenger.sql). Existing installations should run
   only unapplied migrations. The third migration assigns owners to old rooms
   deterministically because the original schema did not record creators;
   review that mapping before applying it to a database with real users.
3. Enable email and phone authentication. Configure email delivery and make the
   email template include `{{ .Token }}` for the app's code-entry flow.
   Configure an SMS provider in Supabase Auth for phone codes. Use six-digit
   codes, five-minute expiry, and at least a 60-second resend interval to match
   the mock policy. Production rate limits and provider configuration belong
   in the backend, not Lua constants or client secrets.
4. Keep identity verification enabled. Linking is performed while signed in;
   the client verifies that the same account actually owns the newly confirmed
   identifier. Secure email changes may require both old and new email codes;
   the UI remains in verification until the authoritative profile confirms the
   requested email. Do not disable verification to make tests appear to pass.
5. Deploy both Edge Functions from this directory using the Supabase CLI:

   ```sh
   supabase functions deploy finalize-chat-photo --project-ref YOUR_PROJECT_REF --workdir apps/chat/backend
   supabase functions deploy cleanup-chat-photos --project-ref YOUR_PROJECT_REF --workdir apps/chat/backend
   ```

   Run these example commands from the repository root. Sources and function
   configuration are under [supabase/](supabase/config.toml). The finalizer
   verifies caller JWTs through the Auth service before any privileged access.
   Its service credentials are read only from the function environment, using
   Supabase's injected key dictionaries or legacy environment variables.
6. Set a random server-only `CHAT_CLEANUP_SECRET` of at least 32 characters in
   Edge Function secrets. Configure a backend Cron job to POST to
   `/functions/v1/cleanup-chat-photos` every five minutes with
   `Authorization: Bearer <CHAT_CLEANUP_SECRET>`. Store the job secret in a
   server-side secret store such as Vault. Each run handles up to 100 expired
   reservations; monitor failures and adjust capacity for traffic. This job
   and its secret have not been provisioned automatically.
7. Copy only the project URL and **publishable** key into app configuration:

   ```sh
   CHAT_BACKEND=supabase SUPABASE_URL=https://YOUR_PROJECT.supabase.co \
   SUPABASE_PUBLISHABLE_KEY=sb_publishable_YOUR_KEY make chat
   ```

   Do not put a database password, provider credentials, service-role JWT,
   secret API key, or cleanup secret in the app. No live credentials are
   required for `CHAT_BACKEND=mock make chat` or the offline tests.

## Media lifecycle

The client prepares a JPEG and thumbnail before reserving an upload. The
reservation binds exact byte counts, dimensions, uploader, conversation, and
immutable object paths. Both objects are uploaded to the private `chat-photos`
bucket; no overwrite policy is granted.

The trusted finalizer verifies the user with Auth, bounds the downloaded data,
checks the actual JPEG marker/scan structure, rejects location metadata, and
compares dimensions and byte counts with the reservation. This is structural
inspection, not a second pixel-entropy decoder. The native encoder tests
independently decode real output and verify its pixels/metadata. A service-only
RPC then locks the reservation and rechecks current publishing permission
before marking it ready. Only ready, unexpired, uploader-owned attachments can
be linked atomically to a message; subscribers cannot publish in channels.

Removed drafts discard their prepared uploads. Expired pending/ready/discarded
reservations are claimed by the cleanup function under row locks. It removes
physical objects through the Storage API, then deletes unlinked metadata.
Failures keep enough metadata for retries. Never delete `storage.objects`
directly in SQL, because that does not remove the underlying file bytes.

## Validation

```sh
make test-chat
make test-media               # Native Apple ImageIO tests on macOS
make test-chat-finalizer      # Node.js: trusted finalizer and cleanup fixtures
make test-network-headers     # Localhost-only headers and binary-byte regression
make test-headless
```

App/API/mock tests use no network. The HTTP regression deliberately uses a
loopback echo server to test the engine transport, with no external service.
The Edge handler tests inject in-memory Auth/database/storage responses.

Before calling a deployment ready, validate it against a real project:
phone/email delivery and linking, unauthorized RPC/table access, member removal,
channel subscriber publishing denial, private media downloads, upload-size
restrictions, expired/revoked invites, concurrent direct creation, and cleanup
retries. SQL migrations and Edge deployment remain unverified against a live
Supabase instance until that project exists. Static review and offline mocks
do not prove database policies or provider behavior.
