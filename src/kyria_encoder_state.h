#pragma once
/* Encoder state is independent of ordinary keyboard layers.
 * 0 = none, 1 = middle, 2 = ring, 3 = NAV vertical. */
int kyria_encoder_active_mode(void);
