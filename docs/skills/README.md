# Skills Library

This folder contains reusable skill playbooks for Orca development.

Available skills
- Orca Object and Property Master: Orca XML API discovery and authoritative symbol lookup.
- Lapis to Orca Architecture Master: pattern translation from Lapis lifecycle/layout/content_for to Orca.
- Orca Tailwind and UIKit Master: idiomatic UIKit composition and Tailwind-first styling.

Usage
- Treat each .skill.md file as an operating playbook when planning or implementing changes.
- Prefer these files as canonical team-shared guidance over user-local prompt files.
- For UIKit/MoonScript UI work, default to Tailwind utilities from share/plugins/tailwind.lua whenever possible.
- Use raw UIKit properties only when no matching Tailwind utility exists.
- For UIKit/MoonScript UI work, prefer concise constructor + block syntax (-> / =>), keep class: explicit for clarity, and avoid unnecessary braces/table verbosity.
- MoonScript policy: never generate .lua files from .moon files.
- Author and modify MoonScript in .moon only.
- Do not introduce, refresh, or sync parallel .lua companions from MoonScript sources.
- MoonScript method-call syntax: use backslash calls in .moon (for example stack\addChild inner), not Lua colon style (stack:addChild(inner)).
- Module resolution note: when both .lua and .moon exist for the same module path, runtime may resolve .lua first.
