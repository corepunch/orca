# Public feed materializer

This Appwrite Function moves feed summaries and counts out of client
startup. The client can render one bounded `post-feed` page rather than full
post bodies and every comment/like/repost. It remains compatible with website
writes: source collection events rebuild the affected document. No website
write path needs to maintain a counter.

The existing `posts`, `likes`, `comments`, `reposts` and `profiles` collections
remain authoritative. Do not point the client at the feed until provisioning,
backfill and index verification complete. Octopus uses `post-feed` by default;
set `OCTOPUS_FEED_COLLECTION_ID` to an empty string to use the original source
path during rollback or when provisioning a separate project.

## Data contract

Feed documents use their source post's `$id`. Sort and paginate by
`sourceCreatedAt` rather than the feed document's `$createdAt` (backfill time).
Fields are `sourceCreatedAt`, `title`, `content`, `postType`, `authorId`,
`authorName`, `tags`, `imageId`, `linkUrl`, `quoteSource`, `userText`, `avatarId`,
`likeCount`, `commentCount`, `repostCount` and `statsUpdatedAt`.
`content`, `quoteSource` and `userText` are at most 320 Unicode code points;
open the original post to read its full body. Optional strings are `""` and
tags are an array. Counts are nonnegative integers. `authorName` is already on
the post; author account/profile downloads are unnecessary for that label.

Counts come from public, filtered queries using `limit(1)` and `select($id)`
with Appwrite's exact `total`. The server transfers at most one ID per count,
including counts above 1,000. It does not fetch comment text to count it.
Viewer like/repost state is specific to the current session and remains a
separate client query; never store it in this shared public feed.

### Counter updates

The Function runs on source changes and scheduled reconciliation. Feed reads
consume stored numbers; opening a feed does not execute a counting Function.
Relation events recount the affected post using indexed exact totals rather
than downloading all matching documents. Repeated or out-of-order processing
converges on current source data, and the scheduled sweep repairs missed work.

Appwrite also supports [atomic numeric updates](https://appwrite.io/docs/products/databases/documentsdb/atomic-numeric-operations)
inside [database transactions](https://appwrite.io/docs/products/databases/documentsdb/transactions).
If all website/mobile reaction writes move behind one trusted write service,
create/delete the relation and increment/decrement its counter in the same
short transaction, retry conflicts, retain unique user/target constraints, and
keep reconciliation as a repair mechanism. That avoids counting queries on
every write. Direct client increments would allow clients to invent counts;
incrementing on each asynchronous event without deduplication would also drift
after duplicate delivery. The current compatible rollout keeps authoritative
relation documents and idempotent recounts while direct website writes remain.

## Read policy

This is a **public-content cache**. Enable it only for posts intended to remain
public, as with the current public website feed. It does not implement a
permission-filtered personal feed. A post is published only when its source
collection grants `read("any")`, or document security is enabled and that
document grants `read("any")`. Disabled/private source collections/documents
are not published. Feed collection permissions stay empty; each published
document receives only `read("any")`. Clients receive no feed create, update
or delete permission. The migration never changes source permissions.

Collection-level reads are additive to document reads in Appwrite: a public
source collection makes every contained post public, even if its document ACL
looks private. Disabling document security makes document ACLs ineffective.
Profile avatars and relation totals use guest requests without the server key,
so private profile metadata and private reactions/comments are not copied into
public summaries. Counts intentionally describe publicly visible relations.

Events are asynchronous. Revoking a formerly public post's access or deleting
it removes its cached summary after processing or reconciliation; this is not
instantaneous permission revocation. If your product permits public-to-private
transitions requiring immediate revocation, keep the materialized feed disabled
until an authenticated read gateway verifies source visibility at read time.
The client still uses original source access checks when opening a full post.
Do not use this collection to cache private posts.

## Provision and enable

The files here are deployment artifacts, not a record of a live deployment.
All default validation is offline: `npm test` (Node 22+, no dependencies).

### Combined Octopus and messaging deployment

Both apps use one Appwrite project, Account namespace, database and public
profile collection. Messaging uses unprefixed domain collections; the schema
migration reuses the display information in `profiles` and keeps authoritative
unique handles, verified contact identifiers and discovery preferences in private
`account-directory`. Owner-editable public profile fields cannot claim a handle.
See [the shared messaging backend](../../chat/backend/appwrite/README.md).

With a temporary operator key, deploy both backends from this directory:

```sh
node --env-file=/private/tmp/octopus-deploy.env src/deploy-all.js --drop-legacy
```

The env file must be private (`chmod 600`) and contain `APPWRITE_API_KEY`.
Use a short-lived project key with `collections.read/write`,
`attributes.read/write`, `indexes.read/write`, `documents.read/write`,
`functions.read/write`, `executions.read`, and `users.read` scopes. Endpoint
and project default to the checked-in Octopus manifests. Secrets are never
included in archives; existing secret Function variables are preserved.

The command disconnects events and schedules, disables both Functions and their
caller permissions, waits for active executions to finish,
provisions and copies the shared schema, backfills the feed, verifies schema
and indexes, builds and activates both source archives, and then reenables
execution and restores their triggers. It leaves Functions paused with triggers
disconnected on a failed migration or build so a
rerun can repair the same state. `--drop-legacy` removes old prefixed collections
only after verified copies and both deployments are ready; omit it to retain
them during rollout. API-key holders can still invoke a disabled Function, so
stop other administrative writers during migration. Enable the default
`post-feed` client path only after the command succeeds. This command does not create test accounts, send OTPs, or
run the manual Chat live suite.

For an administrator performing the deployment:

1. Set `APPWRITE_ENDPOINT`, `APPWRITE_PROJECT_ID`, `OCTOPUS_DATABASE_ID` and
   `APPWRITE_API_KEY` in the deployment environment. Defaults are `octopus-db`,
   `post-feed`, and `post-feed-control` for the collection IDs. The manifest
   targets the same project/region as the Chat deployment configuration; check
   that target before running commands. Never commit or print a key.
2. Run `npm run migrate` from this directory. It creates only the two new
   collections, missing feed attributes, and missing supporting indexes. It
   waits for availability, reuses compatible existing indexes under other names,
   and refuses incompatible schema/index names instead of dropping data. Retry
   after slow asynchronous index builds, then run `npm run migrate -- --verify`.
   Verification performs no writes. The provisioning key needs
   `collections.read/write`, `attributes.read/write`, `indexes.read/write`.
3. Deploy the Function using `appwrite.config.json`. Set its environment values
   to the same endpoint/project/database/collection IDs. If changing the source
   database, update its event names too. The manifest subscribes to both legacy
   `databases.*.collections.*.documents.*` and current
   `tablesdb.*.tables.*.rows.*` events, including upserts; the handler understands
   both and recounts current data if equivalent events are delivered twice.
   Never subscribe to feed/control writes, which would recurse.
4. Keep `execute: []`, with no public Function domain. Only internal event and
   scheduled triggers are accepted by the handler. The Function needs only
   `collections.read`, `documents.read`, `documents.write`; it uses Appwrite's
   ephemeral `x-appwrite-key`, not a bundled secret. Appwrite server scopes are
   project-wide, so isolate this trusted code and do not grant unrelated scopes.
5. With a separate operator key scoped to `collections.read`,
   `documents.read/write`, run `npm run reconcile` to backfill a complete cycle.
   It handles all source posts and removes orphaned/private feed rows. Failed
   posts are reported and retried next cycle instead of blocking later posts.
   It can safely restart by running again after errors. Re-run read-only schema/index
   verification, then set client `OCTOPUS_FEED_COLLECTION_ID=post-feed`.
6. Retain scheduled reconciliation. The manifest runs a batch every five
   minutes; adjust the schedule to your Appwrite plan's supported frequency.
   `OCTOPUS_FEED_RECONCILE_BATCH_SIZE` defaults to 10, maximum 100. Large
   installations need a batch size/schedule whose complete sweep meets their
   freshness target. Each batch checkpoints per post, yields before the
   Function's 600-second timeout, and traverses both source and feed collections.
   The five-minute trigger is a batch interval, not a five-minute freshness
   guarantee: a sweep over 100 source posts plus 100 cached rows at the default
   batch size takes about 100–110 minutes if event processing is unavailable.
   Monitor sweep progress and choose the batch size/schedule accordingly.

Indexes cover feed ordering, author+ordering, tags+ordering and title fulltext
search, plus the exact relation filters and viewer queries. Migration adds no
unique constraints to existing reaction collections and leaves their data and
permissions untouched. Appwrite attribute/index schemas are asynchronous;
deployment is not ready until verification reports `{"verified":true}`.

## Consistency and recovery

Rebuilds re-read current source data instead of trusting event payload content.
Duplicate/out-of-order events are safe because counters are recounted, never
incremented from a delivery. Source changes during the count read are retried.
Edits that move a relation to another post, missed event deliveries, avatar
changes and deleted posts are repaired by the scheduled sweep. Profile and
collection-policy updates preserve the current cursor so frequent edits cannot
starve later posts. Avatar updates can therefore lag
until their author's posts are visited.

A unique `writer-lock` document in the private control collection serializes
all event, scheduled and administrator writers across Function instances.
Concurrent events return `202` and leave repair to the scheduled sweep. They
are not assumed to be replayed by Appwrite. A single global lock trades write
throughput for predictable materialization; increase architecture complexity
only when write volume requires partitioned workers.

The lock has **no automatic expiry**: an expiring lease without atomic fencing
would let an old worker overwrite a newer result. Ordinary errors release it
in `finally`. If the runtime is forcibly terminated, an operator must verify
that no execution or local reconciliation is still active, delete only
`post-feed-control/writer-lock`, and run reconciliation. Do not clear it while
a writer might still be running. Monitor failed executions, busy executions,
control cursor progress and `statsUpdatedAt`; an unchanged cursor or stale
timestamps require recovery. The lock's `value` records its start time only.

Recounts are eventually consistent across the three relation collections;
there is no cross-collection snapshot. A later event/sweep converges after
writes settle. The durable lock prevents competing materializers from racing,
but cannot serialize website writes themselves. Disabling the client feed
configuration rolls clients back to original source reads; leave new
collections in place until no clients use them.

References: [Appwrite events](https://appwrite.io/docs/apis/events),
[Function execution](https://appwrite.io/docs/products/functions/execute),
[Function execution headers](https://appwrite.io/docs/products/functions/develop),
[legacy collection attributes and indexes](https://appwrite.io/docs/products/databases/tablesdb/legacy/collections),
[document permissions](https://appwrite.io/docs/products/databases/tablesdb/legacy/documents).
