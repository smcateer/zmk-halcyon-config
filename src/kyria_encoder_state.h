#pragma once

enum kyria_encoder_side {
    KYRIA_ENCODER_LEFT,
    KYRIA_ENCODER_RIGHT,
};

/* The nonzero values match ENC_MODE_* in halcyon_kyria.keymap. */
enum kyria_encoder_mode {
    MODE_NONE = 0,
    MODE_LEFT_TAB = 1,
    MODE_LEFT_CTRL_TAB = 2,
    MODE_RIGHT_NAV_VERTICAL = 3,
    MODE_RIGHT_NAV_PAGE = 4,
    MODE_RIGHT_FAST_SCROLL = 5,
    MODE_RIGHT_HORIZONTAL_SCROLL = 6,
    MODE_MAX = MODE_RIGHT_HORIZONTAL_SCROLL,
};

enum kyria_encoder_mode kyria_encoder_active_mode(enum kyria_encoder_side side);
