# Windows Phone settings references

Sources collected on 2026-11-03 while redesigning Chat's settings tab. The
question was how Windows Phone messaging apps laid out settings, and whether
they used Panorama for them. Links to source code are pinned to the commit
that was read.

Learn, Wikimedia Commons and the Web Archive were blocked in the research
session. The Microsoft pages below were found by search but not read in
full, and no period screenshots of settings screens were collected. Images
already in this folder are described in [Chat design decisions](../../DESIGN.md).

## Findings

- None of the seven open-source messaging apps below uses `Panorama` in any
  page. `Pivot` appears only for top-level lists (chats/contacts, friends,
  groups, search, media).
- Every settings screen is a plain page: the app title above a vertical
  `ScrollViewer`. Groups open as sub-pages.
- Telegram and VK both start settings with a large photo, your name and a
  status line, followed by icon rows, and end with log out. VK draws log out
  in red as the last row; Telegram moves it to the ellipsis menu.
- Toggles are `ToggleSwitch` (on/off) or `CheckBox` lists. VK's
  notifications page uses 20 check boxes and 5 toggle switches.
- Some measurements, as declared in the XAML (Windows Phone's 480-wide
  layout units):
  - Telegram: 100 px photo; rows with a 32 px icon, a 27 pt title and a
    subtle value underneath.
  - VK: 96 px photo; 64 px rows with a 32 px icon at 16 px and the label at
    72 px.

## Telegram for Windows Phone

Official client, open source:
[evgeny-nadymov/telegram-wp](https://github.com/evgeny-nadymov/telegram-wp)
at `fd98ac6d1863`.

- [Views folder](https://github.com/evgeny-nadymov/telegram-wp/tree/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views)
  contains 148 XAML pages and no Panorama. Pivot is used in `ShellView`,
  `Search/SearchShellView`, `Media/FullMediaView`,
  `Additional/StartupView` and `Contacts/SecretContactView`.
- [SettingsView.xaml](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Additional/SettingsView.xaml)
  shows the uppercase "SETTINGS" title and a scrolling column: photo and
  name, then rows for mobile, username, bio, notifications, chat settings,
  privacy & security, stickers and proxy. Its application bar has Set photo
  and Edit name; Ask a question, FAQ, Privacy policy and Log out are in the
  "…" menu.
- [NotificationsView.xaml](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Additional/NotificationsView.xaml):
  8 `ToggleSwitch` and 2 `ListPicker` controls, under large group headings.
- [ChatSettingsView.xaml](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Additional/ChatSettingsView.xaml):
  `CheckBox` lists for automatic downloads, and 3 `ToggleSwitch` controls.
- [PrivacySecurityView.xaml](https://github.com/evgeny-nadymov/telegram-wp/blob/fd98ac6d18637019218679b81cb92d983019014b/TelegramClient/Views/Additional/PrivacySecurityView.xaml):
  16 two-line button rows.

## VK for Windows Phone 8.1

Source of the official client:
[Makzz90/VKClient_re](https://github.com/Makzz90/VKClient_re/tree/5aa9ae7a28b1)
at `5aa9ae7a28b1`. It has 373 XAML pages and no Panorama.

- [settingspage.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingspage.xaml)
  starts with the photo, name and status, then an "Edit profile" accent row.
  Next come grouped sections with subtle headings: Notifications, General,
  Account, Privacy, Blacklist, Balance and the rest, then Help and About,
  then log out in red.
- [UC/settingssectionuc.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/UC/settingssectionuc.xaml)
  is the shared row: 64 px tall, 32 px icon, 25.333 pt title.
- [settingsnotifications.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingsnotifications.xaml):
  20 `CheckBox` and 5 `ToggleSwitch` controls.
- [settingsgeneralpage.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingsgeneralpage.xaml):
  8 `ToggleSwitch`, 5 `ListPicker`, 3 `CheckBox` and 2 `Slider` controls.
- Other settings sub-pages:
  [settingsaccountpage.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingsaccountpage.xaml),
  [settingsprivacypage.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingsprivacypage.xaml),
  [settingseditprofilepage.xaml](https://github.com/Makzz90/VKClient_re/blob/5aa9ae7a28b1/VKClient.Common/settingseditprofilepage.xaml).

## Smaller apps

These are period or hobby clients. WhatsappForWP and DeltaChatWP8.1 have
commits from 2026, so they are modern builds for old phones, not historical
evidence.

| App | Pivot | Settings |
| --- | --- | --- |
| [IRCCloud-WP](https://github.com/Dahlgren/IRCCloud-WP/tree/b6866cedf929) (WP8) | [MainPage](https://github.com/Dahlgren/IRCCloud-WP/blob/b6866cedf929/IRCCloud/MainPage.xaml) | [SettingsPage](https://github.com/Dahlgren/IRCCloud-WP/blob/b6866cedf929/IRCCloud/SettingsPage.xaml): plain page, 2 toggles |
| [VChat](https://github.com/igoriok/VChat/tree/8c6e480a218f) (2012, VK) | [MainView](https://github.com/igoriok/VChat/blob/8c6e480a218f/VChat/Views/MainView.xaml) | [SettingsView](https://github.com/igoriok/VChat/blob/8c6e480a218f/VChat/Views/SettingsView.xaml): plain page, 3 toggles |
| [JabberWindowsPhone](https://github.com/Styort/JabberWindowsPhone/tree/7c25db7a3b06) | MainPage | — |
| [WhatsappForWP](https://github.com/vincenzosco/WhatsappForWP/tree/91500f14fbf6) (unofficial) | — | [SettingsPage](https://github.com/vincenzosco/WhatsappForWP/blob/91500f14fbf6/WhatsappApp/Pages/SettingsPage.xaml): plain page, 4 toggles |
| [DeltaChatWP8.1](https://github.com/Symnok/DeltaChatWP8.1/tree/ff3b3bebcb94) | — | [SettingsPage](https://github.com/Symnok/DeltaChatWP8.1/blob/ff3b3bebcb94/DeltaChat.WP81/Views/SettingsPage.xaml): plain page, toggles, text field |

## Microsoft guidance

Found by search, not read in full from this session:

- [Panorama control design guidelines for Windows Phone](https://learn.microsoft.com/en-us/previous-versions/windows/apps/hh202912(v=vs.105))
  describe Panorama as a full-screen container and navigation model. They
  advise against nesting Pivot and Panorama and against scrollable controls
  inside a section, and say the control is portrait-only.
- [Panorama control for Windows Phone 8](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941104(v=vs.105))
- [Panorama control architecture for Windows Phone 8](https://learn.microsoft.com/en-us/previous-versions/windows/apps/ff941126(v=vs.105))
- The [Windows Phone UI cheatsheet](../../../../docs/windows-phone-ui-cheatsheet.md)
  has the Pivot/Panorama choice table and its sourced guidance. It says
  Chat's auth, Settings and creation flows are not candidates for Panorama.

## Searches that found nothing useful

None of these show a Windows Phone settings screen:
[Telegram on X](https://x.com/telegram/status/1222603130321412096?lang=en-GB),
[Kaspersky: Telegram privacy settings on Windows](https://privacy.kaspersky.com/articles/telegram-windows-medium/),
[Windows Central: switching from WhatsApp to Telegram](https://www.windowscentral.com/how-switch-whatsapp-telegram),
[Telegram for BlackBerry settings](https://core.telegram.org/blackberry/settings).
