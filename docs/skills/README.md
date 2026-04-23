# Skills Library

This folder contains reusable skill playbooks for Orca development.

Available skills
- Orca Object and Property Master: Orca XML API discovery and authoritative symbol lookup.
- Lapis to Orca Architecture Master: pattern translation from Lapis lifecycle/layout/content_for to Orca.
- Orca Tailwind and UIKit Master: idiomatic UIKit composition and Tailwind-first styling.
- Orca App MoonScript Lapis Builder: app-development guidance using MoonScript, Lapis patterns, Tailwind, and Orca dialect conventions.
- Orca Lua Lapis Internals Architect: engine-side Lua API design using Lapis internals patterns and MoonScript interoperability awareness.
- Orca C Engine WPF WinAPI Architect: C engine architecture, XML-first API design, WPF/WinAPI-inspired messaging, and Lua API negotiation.

Lapis references
- docs/lapis/lapis-user-guide.md: application-authoring reference for Orca app developers using Lapis-inspired patterns.
- docs/lapis/lapis-internals.md: framework/runtime reference for engine-side Lua architecture inspired by Lapis internals.

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
- Pre-release API policy: prefer clean architecture over temporary compatibility shims; if an API is not in active use yet, remove/rename it directly rather than keeping backward-compat layers.
- Generated-file policy: do not hand-edit generated files such as source/core/core_export.c, source/core/core.h, source/core/core_properties.h, plugins/*/*_export.c, plugins/*/*_properties.h, or include/orca.h; edit XML/templates and regenerate with make modules.
