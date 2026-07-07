Yes, several things:
1. How to attach Lua to XML pages — no documentation on code-behind patterns. Had to reverse-engineer from Editor sample's LayerPrefabPlaceholder + MoonScript samples, and you had to tell me about Controller=.
2. Data source mutation status — DATASOURCE_DESIGN.md describes DataProvider.Insert but doesn't clearly say "not implemented." Had to grep all .cgen files to confirm it's missing.
3. StartupViewController vs StartupScreen — the distinction and when to use which wasn't documented. Read startup.lua to figure it out.
4. LayerPrefabPlaceholder resolution — how PlaceholderTemplate path resolves to a file. Dug through fs_xml.c:518-526 to find it calls FS_LoadObject() which tries registered loaders by extension.
5. PostMessage timing with Lua — you had to explain that messages are processed when Lua state is available. This lifecycle detail isn't documented anywhere.
6. Input can't be wired from XML — no documentation that Submit/TextInput events require Lua callbacks. Discovered by reading UIKit.cgen message definitions.
7. Node.LoadView vs Object.Start vs ViewDidLoad — the message lifecycle and when each fires. Had to grep multiple C handlers to piece it together.
8. OBJ_Rebuild / body_ref mechanism — how body() in widgets connects to the C side. Read object_lua.c to understand the coroutine + registry ref pattern.
Most of these are "how do I do X" questions that a cookbook or FAQ would answer. The architecture docs describe what exists but not how to use it for common tasks.