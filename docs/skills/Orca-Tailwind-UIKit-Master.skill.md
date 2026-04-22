# Orca Tailwind and UIKit Master Skill

Purpose
- Specialize in Orca UIKit composition and Tailwind plugin usage.

Canonical references
- share/plugins/tailwind.lua
- plugins/UIKit/UIKit.xml
- docs/lua-scene-loading.md
- samples/Example/Screens/ApplicationTW.xml
- samples/Banking/

Style system principles
- Prefer Tailwind class strings for layout, spacing, and typography.
- Tailwind-first rule: when a custom Tailwind utility exists in share/plugins/tailwind.lua, use class utilities instead of direct UIKit property fields.
- Keep direct properties only for values not represented by existing utilities (for example Grid Rows, exact pixel heights not in spacing scale, or custom non-token colors).
- Use class field style when readable:
- class: "p-6 gap-3"
- Support shorthand selector style when needed:
- ".p-6.gap-3"
- Prefer class-based text sizing instead of raw FontSize where equivalents exist:
- text-sm, text-base, text-xl, text-2xl, text-3xl

UIKit composition rules
- Prefer closure style for idiomatic MoonScript.
- For hierarchical UI composition, prefer DSL block syntax with -> (for example stack "...", -> ...), instead of long node + chains.
- Use node + only when attaching a pre-built node instance; for normal UI trees prefer constructor + block syntax.
- Never hand-edit generated C bindings/headers; edit XML/templates and regenerate with make modules.
- The body function passed to a widget (StackView class: "...", =>) is run AFTER OBJ_Clear wipes the node. Any children added via an outside reference before the body runs will be destroyed.
- To inject pre-built nodes (e.g. route content from content_for) into a widget, do it INSIDE the body closure.
- Correct (concise): Grid Rows: "64px 1fr 72px", => make_header title; (inner or make_placeholder!); make_footer active_route, navigate
- Wrong: stack = StackView ...; stack\addChild inner from outside body (child can be cleared when body runs)
- Never generate .lua files from .moon files.
- Keep style and layout changes in .moon sources only.
- Do not create or sync parallel .lua versions for MoonScript UI modules.

Concise MoonScript style (team convention)
- Prefer concise constructor syntax over verbose object-table style where readable.
- Keep class: explicit for clarity, even in concise form.
- Prefer this style: StackView class: "w-12 flex-col items-center", => ...
- Prefer this style: TextBlock class: "text-xs", item.label
- Avoid unnecessary braces for simple widget construction blocks.
- Avoid temporary locals when a direct expression is clear.

UIKit image/icon gotchas (validated on Weather sample)
- Ensure icon/image assets are mounted as a project reference. Add an assets subproject in package.lua when icons are loaded from assets/: ProjectReferences = { { Name = "assets", Path = "assets" }, ... }.
- For ImageView, use Source (not Image) when binding SVG icon paths in MoonScript object tables.
- For SVG masking in query params, use mask=true (not type=mask), e.g. Source: "assets/icons/home.svg?width=26&mask=true".

Reference pattern
- Banking footer demonstrates the preferred hierarchy-first style: stack ".bg-muted.w-full.h-full.justify-evenly", -> ... with children declared inside the -> block.

Text content idiom
- Prefer positional value style for TextBlock and Button text content, matching Lapis-style readability.
- Preferred:
	- TextBlock class: "text-3xl", title
	- Button class: "btn btn-primary", "Save"
- Avoid when not needed:
	- TextBlock Text: title, class: "text-3xl"
- Treat this positional value as object text content (Object.SetTextContent / Text semantics).

Behavior requirements
- Avoid mixing conflicting style paradigms without reason.
- Keep class naming consistent and readable.
- Explain class choices in terms of spacing, hierarchy, and intent.
- Agent policy: for UIKit/MoonScript edits, default to Tailwind utilities whenever possible and justify any remaining raw property assignments.

Common conversions
- FontSize 14 -> class: "text-sm"
- FontSize 16 -> class: "text-base"
- FontSize 20 -> class: "text-xl"
- FontSize 24 -> class: "text-2xl"
- FontSize 28 -> class: "text-3xl"

Output contract
- Concise visual intent summary.
- Exact patch with class updates.
- Validation step using project run task or make weather.
