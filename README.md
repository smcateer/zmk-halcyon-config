# Halcyon Kyria

My ZMK config for a wireless Halcyon Kyria with two encoders and a central dongle. The layout started as a copy of [my Voyager layout](https://configure.zsa.io/voyager/layouts/OwgPr/latest/0), with APTMAK on base. The firmware is based on [splitkb’s Halcyon config](https://github.com/splitkb/zmk-halcyon-config).

## Key addresses

I name keys by half, row and position. The first letter is the half (`L` or `R`). The second is the row (`T` for top, `M` for middle, `B` for bottom, `U` for upper thumb and `L` for lower thumb). Count inwards from the outside edge of each half, starting at `0`.

So `RT0` is the outermost key on the right top row and `RT5` is the innermost. `LL3` is the fourth key in from the outside on the left lower thumb row. These are physical positions and don’t change with the layer.

## Layout

[ZMK keymap](./config/halcyon_kyria.keymap) · [Interactive view of `main`](https://customkeymap.com/?load=https%3A%2F%2Fraw.githubusercontent.com%2Fsmcateer%2Fzmk-halcyon-config%2Fmain%2Fconfig%2Fhalcyon_kyria.keymap&view=single&layer=0)

I spend a fair bit of time with my left hand on the keyboard and my right on the trackball. Cut, copy, paste, undo and redo are all on the left side of NAV. I can select something with the trackball and do something with it without shifting either hand.

I use Vim and generally prefer sequences to chords. I tap the NAV thumb key, tap Copy, and I’m back on base. If I want to do several things on NAV, I hold the same thumb key. Symbols and Number work the same way. The thumb bindings are transparent on the other layers, so things like Space and Backspace stay where I expect them.

Symbols are roughly **mathsy on the left, typingish on the right**. The Number layer has digits on the right and arithmetic on the left. `/`, `*`, `-` and `+` are in the same places on both layers. I want to reach for `+` in the same place whether I’m typing code or entering a calculation.

## Encoders

The left dial switches workspaces. The right one scrolls. Holding a nearby key changes what the dial does for as long as I hold it. The firmware tracks that separately from the typing layers, so an encoder hold can outlast the one-shot NAV layer.

| Context | Left dial, clockwise / anticlockwise | Right dial, clockwise / anticlockwise |
| --- | --- | --- |
| Base | Next / previous workspace | Scroll down / up |
| Base + hold `LT5` or `RT4` | Tab / Shift+Tab | Fast scroll down / up |
| Base + hold `LT4` or `RT5` | Ctrl+Tab / Ctrl+Shift+Tab | Scroll right / left |
| NAV | — | Right / Left |
| NAV + hold `RT5` | — | Down / Up |
| NAV + hold `RT4` | — | Page Down / Page Up |

On base, `LT5` also holds Alt, so I can hold `LT5`, turn the left dial to pick a window, then let go to select it. All four base-layer keys still type their usual letters when tapped. On NAV, tapping `RT4` sends Home and tapping `RT5` does nothing. If holds overlap, paging wins, then vertical arrows, then the Ctrl+Tab mode, then the Tab mode.

## The build

Both halves run on coin cells, with the holders inside the plate stack. I used encoders in place of the displays, black anodised aluminium top plates, stainless-steel bottom plates and blank black LDSA caps. The upper thumb keys are convex so I can tell them apart from the lower row by feel.

To check the batteries, I hit `LT1` on Function and the dongle types this into whatever text field I’m in:

```text
LEFT   [###########-]  94%
RIGHT  [###########-]  95%
```

## Drawing the layout

The [render script](./scripts/render_kyria.py) turns the ZMK keymap into input for keymap-drawer. It includes the encoder push buttons and skips the eight unused firmware positions. The SVG is checked in alongside the keymap so the diagram can show what’s on the branch you’re looking at.
