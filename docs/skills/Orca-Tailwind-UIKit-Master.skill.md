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
- Keep container references when inserting prebuilt child nodes later.
- Use explicit addChild for existing node instances.
- Do not run moonc to generate .lua files for routine UI iteration.
- Keep style and layout changes in .moon sources; run them directly through Orca.

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
