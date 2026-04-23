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
- /Users/igor/Developer/orca/docs/lapis/lapis-user-guide.md
- /Users/igor/Developer/orca/docs/lapis/lapis-internals.md

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
- In MoonScript examples, always use MoonScript call syntax for methods (obj\method arg), not Lua colon form.
- Example: stack\addChild inner
- Never hand-edit generated C bindings/headers; edit XML/templates and regenerate with make modules.
- Ensure module paths are runtime-resolvable from Orca project roots.
- Never generate .lua files from .moon files.
- Author and edit only the .moon source for MoonScript modules.
- Do not add or maintain generated/synced .lua companions for .moon modules.
- Practical caveat: if both module.lua and module.moon exist for the same require path, current runtime resolution may pick module.lua first. Keep paired files aligned or use one canonical module file per path.
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
