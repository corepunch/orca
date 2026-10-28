# Backend setup and deployment

Chat runs on the Appwrite **Octopus** project. Its seven private `chat_`
collections and indexes have been provisioned in `octopus-db` through the Appwrite
plugin. The trusted `orca-chat-api` Function is deployed and its server-only
verification secret is configured. Messaging providers still need setup. See
[Appwrite setup](appwrite/README.md), its [schema](appwrite/schema.json) and
[deployment manifest](appwrite/appwrite.config.json).

New-message push uses Appwrite Messaging with a direct APNs provider. Configure
the provider and signed Apple build, set the client's public push provider ID,
and enable the Function's `CHAT_PUSH_ENABLED=true` server variable as described
in [APNs activation](appwrite/README.md#new-message-push-apns). Alerts contain
generic text and open the authorized conversation for the signed-in recipient.
Push queue failures preserve the saved message; delivery is best-effort and
requires provider configuration outside the repository.

The client's public configuration is in [config.lua](../config.lua): the
endpoint, project ID and Function ID. It contains no secrets, so installed iOS
builds connect to Octopus without environment variables. Do not put API keys,
provider credentials or Function secrets in the app.

Use `CHAT_BACKEND=mock make chat` for the isolated offline backend. It serves
the same Chat API paths in process, so the app and tests need no network or
Appwrite account.

## Validation

For routine Appwrite changes, run only the affected unit tests:

```sh
make test-chat-appwrite           # Lua adapter; no ORCA startup or rebuild
make test-chat-appwrite-backend   # Node.js: Appwrite Function, offline
```

Select individual Function cases with Node's `--test-name-pattern` when possible.
UI-only changes need no Appwrite tests. Do not repeat passing checks without
relevant changes. Integration and end-to-end tests, including the offline
live-suite emulator, run only on explicit user request and never automatically
after edits, builds or deployments. See [live tests](appwrite/live/README.md).

App/API/mock tests use no network. The HTTP regression deliberately uses a
loopback echo server to test the engine transport, with no external service.
The Function suite injects in-memory Appwrite REST and repository responses.
