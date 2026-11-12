/*
 * Held encoder modes, independent of ZMK keyboard layers.
 * Only compiled on the dongle (split central).
 */

#define DT_DRV_COMPAT zmk_behavior_kyria_encoder_mode

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>

#include "kyria_encoder_state.h"

#define MODE_MIDDLE 1
#define MODE_RING 2
#define MODE_NAV_VERTICAL 3
#define MODE_MAX MODE_NAV_VERTICAL

/* Counts allow B/J or P/L to be held simultaneously without premature reset. */
static uint8_t held_modes[MODE_MAX + 1];

int kyria_encoder_active_mode(void) {
    if (held_modes[MODE_NAV_VERTICAL]) {
        return MODE_NAV_VERTICAL;
    }
    if (held_modes[MODE_RING]) {
        return MODE_RING;
    }
    if (held_modes[MODE_MIDDLE]) {
        return MODE_MIDDLE;
    }
    return 0;
}

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    const uint32_t mode = binding->param1;
    if (mode == 0 || mode > MODE_MAX) {
        return -EINVAL;
    }
    if (held_modes[mode] < UINT8_MAX) {
        held_modes[mode]++;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    const uint32_t mode = binding->param1;
    if (mode == 0 || mode > MODE_MAX) {
        return -EINVAL;
    }
    if (held_modes[mode] > 0) {
        held_modes[mode]--;
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api mode_api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
};

#define MODE_INST(n)                                                                            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                              \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &mode_api);

DT_INST_FOREACH_STATUS_OKAY(MODE_INST)
