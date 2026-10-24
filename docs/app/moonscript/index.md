# MoonScript App Writing

MoonScript apps can compose UIKit objects through `Application` and `Widget`.
Banking and Weather currently use this path; it remains supported alongside
XML-first applications.

Start with [Building ORCA applications](../index.md) for startup, navigation,
state, service boundaries, and the distinction between current support and
recommended defaults. Use [App styling](../styling.md) for themes and CSS;
Lua/MoonScript and XML share the same styling pipeline.

For a scoped implementation reference, read
[Weather's app entry](../../../samples/Weather/App.moon) and the one screen
relevant to the feature. API and workflow guidance lives in
[Lua API](../../LUA_API.md).

