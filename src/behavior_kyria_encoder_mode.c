/*
 * Held encoder modes, independent of ZMK keyboard layers.
 * Only compiled on the dongle (split central).
 */

#define DT_DRV_COMPAT zmk_behavior_kyria_encoder_mode

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <zmk/event_manager.h>
#include <zmk/events/position_state_changed.h>

#include "kyria_encoder_state.h"

#define MODE_MIDDLE 1
#define MODE_RING 2
#define MODE_NAV_VERTICAL 3
#define MODE_NAV_PAGE 4
#define MODE_MAX MODE_NAV_PAGE

/* ZMK hold-tap-backed mode keys use refcounts for overlapping holds. */
static uint8_t held_modes[MODE_MAX + 1];

/* Two position-based holds bypass unreliable mode binding paths:
 * BASE key 05 (B) has a macro for Alt, while its mode comes from this listener.
 * NAV  key 06 (J) has no keycode; holding it selects vertical arrow mode.
 * The dongle receives position events from both halves.
 */
#define B_POSITION 5
#define J_POSITION 6
#define NAV_LAYER DT_PROP(DT_NODELABEL(kyria_encoder_action), nav_layer)

static bool base_b_held;
static bool nav_j_held;

static int physical_encoder_mode_listener(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *ev = as_zmk_position_state_changed(eh);
    if (ev == NULL) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    if (ev->position == B_POSITION) {
        /* Only the BASE binding of position 05 is the B/Alt switcher. */
        base_b_held = ev->state &&
            zmk_keymap_layer_index_to_id(zmk_keymap_highest_layer_active()) ==
                zmk_keymap_layer_default();
    } else if (ev->position == J_POSITION) {
        /* A held J retains vertical mode after a one-shot NAV expires. */
        nav_j_held = ev->state && zmk_keymap_layer_active(NAV_LAYER);
    }

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(kyria_encoder_mode_positions, physical_encoder_mode_listener);
ZMK_SUBSCRIPTION(kyria_encoder_mode_positions, zmk_position_state_changed);

int kyria_encoder_active_mode(void) {
    if (held_modes[MODE_NAV_PAGE]) {
        return MODE_NAV_PAGE;
    }
    if (nav_j_held || held_modes[MODE_NAV_VERTICAL]) {
        return MODE_NAV_VERTICAL;
    }
    if (held_modes[MODE_RING]) {
        return MODE_RING;
    }
    if (base_b_held || held_modes[MODE_MIDDLE]) {
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
