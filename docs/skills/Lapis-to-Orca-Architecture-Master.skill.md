# Lapis to Orca Architecture Master Skill

Purpose
- Translate proven Lapis architecture patterns into Orca Lua and MoonScript safely and idiomatically.

Lapis references
- /Users/igor/Developer/lapis-master/lapis/application.moon
- /Users/igor/Developer/lapis-master/lapis/request.moon
- /Users/igor/Developer/lapis-master/lapis/html.moon
- /Users/igor/Developer/lapis-master/lapis/etlua.moon
- /Users/igor/Developer/lapis-master/spec/request_spec.moon
- /Users/igor/Developer/lapis-master/spec/html_spec.moon

Orca references
- share/orca/core2/application.lua
- share/orca/core2/widget.lua
- share/orca/core2/router.lua
- docs/lua-scene-loading.md
- samples/Weather/
- samples/Banking/

Core mapping model
- Lapis request state -> Orca render context table in dispatch
- Lapis helper chain -> Orca include_helper chain
- Lapis content_for storage -> Orca ctx.content dictionary
- Lapis layout render pass -> Orca layout widget content pass

Translation rules
- Prevent helper lookup from shadowing core widget methods.
- Build UI trees with Orca container ownership rules.
- Ensure module paths are runtime-resolvable from Orca project roots.
- Do not run moonc to generate .lua files as part of normal Orca development flow.
- Prefer editing and running .moon files directly; Orca can execute MoonScript on the fly.
- Prefer Lapis-like trailing value text style for text-bearing widgets:
- TextBlock class: "text-3xl", title
- Button class: "btn", "Submit"
- Map this style to Orca text content behavior (Object.SetTextContent / Text).

Debug checklist for layout issues
1. StartupViewController resolves.
2. layout module require path resolves.
3. content_for is called with attached render context.
4. helper chain augments behavior rather than overriding core methods.
5. routed content is inserted into the intended container.

Response contract
- Name the Lapis pattern being replicated.
- Show Orca equivalent.
- Call out semantic differences.
- Propose smallest safe patch and validation command.

Validation preference
- Prefer make weather or sample-specific run commands.
- Report runtime output deltas before and after.
