# Lucide icons

These are raw SVGs from the official
[Lucide repository](https://github.com/lucide-icons/lucide/tree/main/icons),
downloaded on 2026-10-23 (`users-round`, `megaphone`, `compass`, `ticket`,
`smartphone`, `mail`, `key-round`, `user-round-plus` and `check` on 2026-10-30,
`camera`, `users` and `user` on 2026-10-31).
The upstream license is included in `LICENSE`.

All are unmodified except `check-bold.svg`: Lucide's `check` with
`stroke-width` raised from 2 to 3.5, a customisation Lucide supports, for the
heavier Windows Phone check box mark.

Each local filename matches its upstream filename, except `circle-help.svg`,
which uses the current upstream
[`circle-question-mark.svg`](https://github.com/lucide-icons/lucide/blob/main/icons/circle-question-mark.svg)
for Lucide's older `circle-help` name.

The SVGs use `stroke="currentColor"`, a 24×24 view box, rounded strokes, and no
fill. ORCA `VectorView` treats these as template alpha masks and tints them
with inherited `Foreground` (`color` in CSS). Set the view size explicitly;
20–24 px works well for controls. Keep the surrounding touch target larger.
