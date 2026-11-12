# Halcyon Kyria layout

* My own personal layout for my Halcyon Kyria (dual encoders)
* Layout originated as a duplicate of my old Voyager layout on [Oryx](https://configure.zsa.io/voyager/layouts/OwgPr/latest/0)
* Visualize the layout [here](https://customkeymap.com/?load=https%3A%2F%2Fraw.githubusercontent.com%2Fsmcateer%2Fzmk-halcyon-config%2Fmain%2Fconfig%2Fhalcyon_kyria.keymap&view=single&layer=0)
* Generated from splitkb/zmk-halcyon-config

# Layout

![layout diagram](https://raw.githubusercontent.com/smcateer/zmk-halcyon-config/refs/heads/main/diagrams/halcyon_kyria.svg)

## Encoder controls

The encoder controller tracks held encoder modes independently of the
five ordinary keyboard layers (BASE, NUM, FUNC, NAV, SYM).

| Context | Left encoder CW / CCW | Right encoder CW / CCW |
| --- | --- | --- |
| BASE | Next / previous workspace | Wheel down / up |
| Hold B or L (MIDDLE) | Tab / Shift+Tab | Fast vertical wheel (5×) |
| Hold P or J (RING) | Ctrl+Tab / Ctrl+Shift+Tab | Horizontal wheel right / left |
| NAV | No action | Right / Left arrow |
| NAV + hold key 06 | No action | Down / Up arrow |
| NAV + hold key 07 | No action | Page Down / Page Up |

Hold B also holds Alt, turning the left encoder into an Alt+Tab switcher.
Releasing B releases Alt and selects the highlighted window. Other ordinary
layers have no default rotation action.

If modes overlap, priority is NAV paging > NAV vertical > RING > MIDDLE; otherwise NAV
or BASE determine the action. NAV key 06 (formerly unused) activates vertical
arrows for as long as it is held, without producing a tap character. NAV key 07
preserves its Home tap action, but holding it changes the right encoder to
Page Down / Page Up (clockwise / anticlockwise). These held modes continue even
if the one-shot NAV layer is consumed by pressing the mode key.

All four B/J/P/L keys retain their tap characters, with the existing 250 ms
encoder-mode hold timing. The 50-unit normal wheel magnitude and 5× fast
scroll multiplier remain defined at the top of the keymap.
