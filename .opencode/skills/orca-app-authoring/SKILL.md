---
name: orca-app-authoring
description: Create or reorganize an ORCA application, choosing its startup, screens, navigation, state, services, and styling boundaries. Use for app scaffolding, architecture decisions, and application-level refactors; use subsystem skills for isolated engine changes.
---

# ORCA Application Authoring

Use [Building ORCA applications](../../../docs/app/index.md) as the canonical
architecture guide. Read [App styling](../../../docs/app/styling.md) when choosing
or changing the app's visual system. Keep substantial conventions in those
documents rather than duplicating them in skills.

## Choose the authoring path

- For new stable screens, prefer XML/CSS with Lua controllers and separate state
  and service modules. This is a recommended default, not a requirement to
  convert existing applications.
- Preserve an explicit user choice or established Lua/MoonScript widget app.
  Both presentation paths use the same ORCA runtime.
- Select a relevant sample by mechanism. Banking and Weather use the current
  Widget/Application pattern; Adventure demonstrates native page navigation;
  Example demonstrates themes and CSS. Do not copy an entire sample or treat
  the chat prototype's single app module as the standard.

## Establish ownership

1. Choose `StartupScreen` or `StartupViewController`; keep the package
   declarative and runtime behavior in application/controller modules.
2. Give navigation one owner. Use `PageHost` for tabs and `NavigationHost` for
   push/pop where appropriate; do not assume URL routing already adapts to
   these controls. Preserve required drafts, selection, and scroll state.
3. Separate UI composition, event handling, application state, and service
   transport. Add only the modules the application actually needs.
4. Define how service data reaches bindings. Plain Lua tables are not automatic
   observable `DataContext` sources. For lists, details and forms, declare a
   schema and scope (`Data/*.xml`) over a provider and bind queries, views,
   commands and edits from XML; see `docs/app/datasources.md` and the
   `apps/ledger` example. Keep row layout in templates, domain rules in the
   provider, and only page chrome and multi-step flows in Lua.
5. Keep requests out of layout construction where possible. Use the supported
   coroutine context, explicit loading/error state, and a view/request lifetime
   check when results can arrive after navigation or a newer request.

## Route implementation

- XML structure and actions: `orca-xml-authoring`.
- Lua/MoonScript modules and components: `orca-lua-authoring`.
- XML events, controllers, and bindings: `orca-controller-binding`.
- Visual design and style implementation: `orca-ui-design` and `orca-css`.
- Resource libraries and scenes: `orca-xml-assets-scenes`.
- Missing native API/export: `orca-api-codegen` and, when relevant,
  `orca-lua-exposure`; do not invent methods or edit generated output.

Read the relevant generated API page before using a new engine capability.
Follow the repository's source-priority and scoped-read rules.

## Verify a vertical slice

Use `orca-testing` and [Testing](../../../docs/TESTING.md) for focused loading,
event/binding, navigation/state, collection, and service-error checks. Exercise
a real user action rather than only the construction helpers. Inspect a native
frame for design changes; headless assertions do not verify rendering. Report
service/platform limitations without turning prototype behavior into guarantees.
