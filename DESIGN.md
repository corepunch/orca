# Windows Phone design references and screen patterns

Research collected on 8 November 2026 for ORCA. This document gathers the
Windows Phone 7/8 Metro and Windows 10 Mobile references discussed during
screen-design research, including authentication, forms, conversations,
lists, details and horizontal navigation.

This is a reference catalogue, not a new visual policy for every ORCA app.
Chat deliberately follows Windows Phone / Metro; other apps may choose a
different direction.

## Read alongside

- [Chat design decisions and references](apps/chat/DESIGN.md): the current
  product baseline, branding, authentication exceptions, command bars,
  photo messages and explicit user decisions.
- [Windows Phone UI cheatsheet](docs/windows-phone-ui-cheatsheet.md):
  versioned metrics, units, spacing ownership, control selection,
  Pivot/Panorama guidance and screen-review checks.
- [Windows Phone transitions](docs/windows-phone-transitions.md):
  historical motion references and ORCA adaptations.

Preserve the distinction between historical Microsoft guidance, third-party
examples, ORCA implementation behavior and explicit app decisions. A reference
image does not establish that its layout is implemented in ORCA.

## Generations and sources

| Generation | Reference | What it provides |
| --- | --- | --- |
| Windows Phone 7 | [Microsoft: UI Design and Interaction Guide announcement, 18 March 2010][wp7-first] | Original announcement of the platform interaction guide |
| Windows Phone 7 | [Microsoft: updated UI guide and design templates, 27 July 2010][wp7-resources] | Version 2.0 guide and a collection of 28 layered Photoshop templates with the Segoe WP font family |
| Windows Phone 7 | [Microsoft-authored v2.0 guide, mirrored PDF][wp7-pdf] | July 2010 prerelease guidance on controls, typography, navigation, input and system behavior |
| Windows Phone 7 era | [Microsoft: login/input-screen guidance, 11 November 2010][login] | Keyboard-safe actions and when to request authentication |
| Windows Phone 7 era | [MSDN: translating common design principles, December 2011][principles] | Application Bar and Panorama examples from real apps |
| Windows Phone 8 era | [Microsoft: eight design tips, 18 January 2013][wp8-tips] | Navigation planning, grid, themes, live tiles and content density |
| Windows Phone 8 era | [Microsoft: design resources, 19 December 2013][wp8-resources] | Design-learning resources, including the Windows Phone 8 Design Bootcamp |
| Metro / WP8 era | [Telerik: expanded design-template catalogue, 7 November 2012][telerik-expanded] | More than 50 templates, including registration, sign-in, forms, messaging and lists |
| Metro / WP8 era | [Telerik: template walkthrough, 11 April 2013][telerik-walkthrough] | About/How To pages, forms, navigation, search/filter, tables and validation examples |
| Windows 10 Mobile | [Microsoft: design direction, 29 April 2015][win10-direction] | Adaptive UX, navigation choices and proposed Mail, Calendar, People and Photos designs |
| Windows 10 Mobile / UWP | [Microsoft: scaling phone designs, 27 June 2016][uwp-scaling] | Six screen families and their adaptation across phone and larger displays |

The Telerik pages are primary sources for Telerik's own templates, **not
Microsoft's official screen specifications**. Their publication dates place
them in the Metro/WP8 period; that does not certify every screenshot as a
particular OS build.

The 2015 Windows 10 design article includes explorations and planned changes.
Do not label all its images as screenshots of the final shipped OS.
Windows 10 Mobile belongs to the UWP design generation; avoid treating its
navigation and control APIs as interchangeable with WP7/8.

## Screen gallery

Images below are externally hosted by their original publishers and can stop
loading if those publishers move them. Source links identify their context.
They are historical references; no image licence is inferred from public
availability. They are not ORCA screenshots or bundled production assets.

### Registration and authentication forms

**Source:** [Telerik's 2012 template catalogue][telerik-expanded].

The catalogue explicitly includes registration, sign-in, check-out and
multipurpose forms. The following is its form illustration, linked here as a
registration/form reference; it is not an official Microsoft-mandated sign-up
screen.

![Telerik Metro registration/form reference](https://www.telerik.com/sfimages/default-source/blogs/data-form-3-png)

Study the vertical form structure, clear field identities, rectangular
controls and typographic hierarchy. Use the source catalogue for the family
of authentication patterns rather than assuming this one image represents
both login and sign-up.

**Microsoft's specific login recommendations:** [2010 input-screen article][login].

- Keep the submit action visible while the on-screen keyboard is open.
- On that platform, the standard Application Bar remained above the keyboard
  and could hold standard OK, Cancel or Save actions.
- If using text buttons in the page, place them where the keyboard does not
  obscure them; the article describes a compact layout with up to two input
  fields and one or two text buttons.
- Let people understand or preview the app's value before demanding
  registration, and request authentication when necessary.

These are historical recommendations, not evidence of ORCA keyboard behavior.
For Chat, retain the existing branded authentication shell and explicit
product decisions. Check actual keyboard insets, scrolling and action
visibility on the target host.

**Additional official sign-in example:** Microsoft's
[Windows Phone Swedish localization style guide, 2014][localization], section
4.1.5, contains a Microsoft-account sign-in illustration with labels above
email and password fields, password visibility, supporting text and
previous/sign-in actions. It is a localization guide, not the main HMI guide;
the filename and title identify Swedish despite some search-engine labels.

### Conversation / messaging

**Source:** [Telerik's 2012 template catalogue][telerik-expanded], the
RadConversationView examples.

![Telerik Windows Phone conversation template](https://www.telerik.com/sfimages/default-source/blogs/conversation-1-png)

Use this to inspect message grouping, participant identification, text density
and the relationship between content and commands. It does not override
[Chat's existing footer and photo-message decisions](apps/chat/DESIGN.md).
The catalogue also includes multi-user conversations and comments.

### Expandable list

**Source:** [Telerik's 2012 template catalogue][telerik-expanded], the
RadExpanderControl examples.

![Telerik Windows Phone expandable list template](https://www.telerik.com/sfimages/default-source/blogs/expanders-5-png)

Useful for inspecting how additional information can be revealed within a
list. Decide whether in-place expansion or a separate detail page better
serves the actual task; this is an example, not a universal navigation rule.

### Detail page with tabbed views

**Source:** [Telerik's 2012 template catalogue][telerik-expanded].

![Telerik Windows Phone details template with tab views](https://www.telerik.com/sfimages/default-source/blogs/details-3-png)

A reference for organizing related views of one item. Keep contextual
information and actions understandable as the selected section changes.
Do not infer ORCA control support from Telerik's implementation.

### Multipurpose form / search and filtering

**Source:** [Telerik's 2013 walkthrough][telerik-walkthrough], Figure 3.

![Telerik Windows Phone multipurpose form](https://www.telerik.com/sfimages/default-source/blogs/3_thumb-3-1f678ffdd509-jpg)

The walkthrough adapts this template into a travel form with departure and
arrival airports, dates, passengers and validation. It is useful for field
alignment, grouped input and selection-page references. Its later figures
show airport selection, an incomplete form and a completed form.

### Panorama: content wider than the display

**Source:** [MSDN, December 2011][principles], Figure 2:
the 911 Memorial application.

![Panorama example published by MSDN](https://learn.microsoft.com/en-us/archive/msdn-magazine/2011/december/images/hh580745.smith_fig02%28en-us%2Cmsdn.10%29.png)

Panorama presents a horizontal canvas extending beyond the viewport. A visible
part of the next section provides a navigation hint. This is an app example
published in Microsoft's developer documentation, not a screenshot of every
system hub.

Pivot switches between related views; Panorama provides a broader horizontal
overview. Use the [versioned cheatsheet](docs/windows-phone-ui-cheatsheet.md)
for the detailed choice, restrictions and gesture ownership. Do not nest
competing horizontal navigation merely to reproduce a screenshot.

## Windows 10 Mobile: screen families and adaptation

The [2016 Microsoft article][uwp-scaling] is particularly useful when the
question is how a whole screen should work rather than how a control looks.

| Screen family | Phone pattern | Larger-display adaptation described by Microsoft |
| --- | --- | --- |
| Tabs / Pivot | Several related views | Resize content and review alignment within the wider Pivot |
| Feed reader | Updates with filters and commands | Resize content; reposition commands/navigation; consider a navigation pane |
| Ebook / magazine | Reading layout | Reflow text into columns or side-by-side pages |
| List and details | List opens a separate detail page | Show list and details side by side where space permits |
| Lifestyle / photography | Rich image-led content | Reconsider composition using the available UWP controls |
| Utility | Focused single-purpose screen | Resize and reposition interactions for the device |

These are Microsoft migration strategies, not confirmation that equivalent
ORCA layouts or controls already exist.

The [2015 design-direction article][win10-direction] also discusses:

- Choosing among a hamburger menu, Pivot, tabs and an app bar based on the task.
- Adapting navigation to the device rather than making every form factor
  identical.
- Bottom command placement in phone Mail and Calendar.
- People imagery and Photos navigation.

Read those illustrations with their original preview/planned-design captions.

## Applying the references in ORCA

These are review recommendations, not additional historical requirements:

1. Choose the generation and relevant screen pattern before copying details.
2. Start from the current app design document and the closest implemented
   ORCA component.
3. Preserve shared text alignment and clear labels. Do not turn screenshot
   pixels directly into ORCA logical units.
4. Test forms with the keyboard visible, long/localized labels, validation,
   loading and retry states.
5. Check narrow and wide layouts, both themes, safe areas and actual hit areas.
6. Keep research, proposals and verified implementation status explicit.

For detailed metrics and control contracts, use the maintained
[cheatsheet](docs/windows-phone-ui-cheatsheet.md) rather than duplicating
constants here. This catalogue does not change current authentication,
navigation, backend or accessibility behavior.

[wp7-first]: https://blogs.windows.com/windowsdeveloper/2010/03/18/windows-phone-7-series-ui-design-interaction-guide/
[wp7-resources]: https://blogs.windows.com/windowsdeveloper/2010/07/27/windows-phone-7-design-resources-ui-guide-and-design-templates/
[wp7-pdf]: https://www.pocketpc.ch/attachments/windows-phone-7-entwicklung/36554d1283328945-windows-phone-7-ui-design-guide-wichtigste-checkliste-ui-design-interaction-guide-windows-phone-7-v2.0.pdf
[login]: https://blogs.windows.com/windowsdeveloper/2010/11/11/2-of-10-make-sure-your-buttons-are-visible-even-when-the-keyboard-is-displayed/
[principles]: https://learn.microsoft.com/en-us/archive/msdn-magazine/2011/december/windows-phone-how-to-translate-common-design-principles-to-the-windows-phone
[wp8-tips]: https://blogs.windows.com/windowsdeveloper/2013/01/18/8-tips-for-designing-windows-phone-apps/
[wp8-resources]: https://blogs.windows.com/windowsdeveloper/2013/12/19/design-resources-for-windows-phone-and-windows-app-development/
[telerik-expanded]: https://www.telerik.com/blogs/design-templates-for-windows-phone-keep-gettin-better
[telerik-walkthrough]: https://www.telerik.com/blogs/design-templates-for-windows-phone1
[win10-direction]: https://blogs.windows.com/windowsexperience/2015/04/29/windows-10-design-getting-the-balance-right/
[uwp-scaling]: https://blogs.windows.com/windowsdeveloper/2016/06/27/scaling-your-phone-app-design-to-all-uwp-device-families/
[localization]: https://download.microsoft.com/download/8/1/E/81E5C05E-9511-4B6C-AD76-4C7B7896509E/sv-SE_Windowsphone_StyleGuide.pdf
