# Backend setup and deployment

Chat runs on the Appwrite **Octopus** project. Its seven private `chat_`
collections and indexes have been provisioned in `octopus-db` through the Appwrite
plugin. The trusted `orca-chat-api` Function is deployed and its server-only
verification secret is configured. Messaging providers still need setup. See
[Appwrite setup](appwrite/README.md), its [schema](appwrite/schema.json) and
[deployment manifest](appwrite/appwrite.config.json).

The client's public configuration is in [config.lua](../config.lua): the
endpoint, project ID and Function ID. It contains no secrets, so installed iOS
builds connect to Octopus without environment variables. Do not put API keys,
provider credentials or Function secrets in the app.

Use `CHAT_BACKEND=mock make chat` for the isolated offline backend. It serves
the same Chat API paths in process, so the app and tests need no network or
Appwrite account.

## Validation

```sh
make test-chat
make test-chat-appwrite-backend   # Node.js: Appwrite Function, offline
make test-media                   # Native Apple ImageIO tests on macOS
make test-network-headers         # Localhost-only headers and binary-byte regression
make test-headless
```

Live checks against the deployed backend are manual and never run in CI:
`make test-live-setup` once, then `make test-live`. See
[live tests](appwrite/live/README.md).

App/API/mock tests use no network. The HTTP regression deliberately uses a
loopback echo server to test the engine transport, with no external service.
The Function suite injects in-memory Appwrite REST and repository responses.
