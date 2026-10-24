import { createHandler } from "./handler.mjs";

Deno.serve(createHandler({ env: Deno.env.toObject() }));
