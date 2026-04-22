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
- Use class field style when readable:
- class: "p-6 gap-3"
- Support shorthand selector style when needed:
- ".p-6.gap-3"
- Prefer class-based text sizing instead of raw FontSize where equivalents exist:
- text-sm, text-base, text-xl, text-2xl, text-3xl

UIKit composition rules
- Prefer closure style for idiomatic MoonScript.
- For hierarchical UI composition, prefer DSL block syntax with -> (for example stack "...", -> ...), instead of long node + chains.
- Use node + when attaching a pre-built node instance, but build nested structure with -> so parent-child intent stays visible.
- The body function passed to a widget (StackView class: "...", =>) is run AFTER OBJ_Clear wipes the node. Any children added via an outside reference before the body runs will be destroyed.
- To inject pre-built nodes (e.g. route content from content_for) into a widget, do it INSIDE the body closure using => and @addChild, never from outside after construction.
- Correct: StackView class: "p-6 gap-3", => ... if inner then @addChild inner
- Wrong: stack = StackView ...; stack\addChild inner  (child gets cleared when body runs)
- Use explicit addChild for existing node instances with MoonScript method syntax.
- In .moon snippets use @addChild inner inside => closures; never add pre-built children from outside the body.
- Never generate .lua files from .moon files.
- Keep style and layout changes in .moon sources only.
- Do not create or sync parallel .lua versions for MoonScript UI modules.

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
