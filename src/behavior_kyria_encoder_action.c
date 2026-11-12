/*
 * Dynamic encoder routing on the central dongle.
 *
 * Existing ZMK sensor-rotate-var converts physical ticks into rotation IDs;
 * this behavior routes those IDs according to held encoder modes and NAV.
 * Each output goes through the stock &kp or &msc behavior.
 */

#define DT_DRV_COMPAT zmk_behavior_kyria_encoder_action

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/keymap.h>
#include <dt-bindings/zmk/keys.h>
#include <dt-bindings/zmk/pointing.h>

#include "kyria_encoder_state.h"

/* Rotations: left CW/CCW = 0/1, right CW/CCW = 2/3. */
#define ROTATIONS 4
#define RIGHT_CW 2
#define MODE_MIDDLE 1
#define MODE_RING 2
#define MODE_NAV_VERTICAL 3

struct encoder_config {
    const char *key_press_dev;
    const char *mouse_scroll_dev;
    uint8_t nav_layer;
    int16_t base_scroll;
    uint8_t fast_multiplier;
};

struct output {
    bool enabled;
    bool mouse;
    uint32_t code;
};

/* Save the selected behavior from press to release: a mode can be released
 * while an encoder's 40 ms queued tap is still in progress. */
static struct {
    bool pending;
    struct zmk_behavior_binding binding;
} active[ROTATIONS];

static struct output key(uint32_t code) {
    return (struct output){.enabled = true, .mouse = false, .code = code};
}

static struct output wheel(uint32_t code) {
    return (struct output){.enabled = true, .mouse = true, .code = code};
}

static struct output select_output(const struct encoder_config *cfg, uint32_t rotation) {
    const bool right = rotation >= RIGHT_CW;
    const bool clockwise = (rotation % 2) == 0;
    const int16_t step = cfg->base_scroll;
    const int16_t fast = step * cfg->fast_multiplier;

    switch (kyria_encoder_active_mode()) {
    case MODE_NAV_VERTICAL:
        return right ? key(clockwise ? DOWN : UP) : (struct output){0};
    case MODE_RING:
        return right ? wheel(clockwise ? MOVE_X(step) : MOVE_X(-step))
                     : key(clockwise ? LC(TAB) : LS(LC(TAB)));
    case MODE_MIDDLE:
        return right ? wheel(clockwise ? MOVE_Y(-fast) : MOVE_Y(fast))
                     : key(clockwise ? TAB : LS(TAB));
    default:
        break;
    }

    /* Respect ordinary layers when no encoder mode is held. Non-NAV
     * non-BASE layers have no rotation output, matching the old keymap. */
    const zmk_keymap_layer_id_t top =
        zmk_keymap_layer_index_to_id(zmk_keymap_highest_layer_active());
    if (top == cfg->nav_layer) {
        return right ? key(clockwise ? RIGHT : LEFT) : (struct output){0};
    }
    if (top == zmk_keymap_layer_default()) {
        return right ? wheel(clockwise ? MOVE_Y(-step) : MOVE_Y(step))
                     : key(clockwise ? LG(LC(RIGHT)) : LG(LC(LEFT)));
    }
    return (struct output){0};
}

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    const uint32_t id = binding->param1;
    if (id >= ROTATIONS) {
        return -EINVAL;
    }
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct encoder_config *cfg = dev->config;
    const struct output selected = select_output(cfg, id);

    if (!selected.enabled) {
        active[id].pending = false;
        return ZMK_BEHAVIOR_OPAQUE;
    }
    active[id].binding = (struct zmk_behavior_binding){
        .behavior_dev = selected.mouse ? cfg->mouse_scroll_dev : cfg->key_press_dev,
        .param1 = selected.code,
    };
    active[id].pending = true;
    return zmk_behavior_invoke_binding(&active[id].binding, event, true);
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    const uint32_t id = binding->param1;
    if (id >= ROTATIONS) {
        return -EINVAL;
    }
    if (!active[id].pending) {
        return ZMK_BEHAVIOR_OPAQUE;
    }
    active[id].pending = false;
    return zmk_behavior_invoke_binding(&active[id].binding, event, false);
}

static const struct behavior_driver_api action_api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
};

#define ACTION_INST(n)                                                                          \
    static const struct encoder_config config_##n = {                                           \
        .key_press_dev = DEVICE_DT_NAME(DT_INST_PHANDLE(n, key_press)),                         \
        .mouse_scroll_dev = DEVICE_DT_NAME(DT_INST_PHANDLE(n, mouse_scroll)),                   \
        .nav_layer = DT_INST_PROP(n, nav_layer),                                                \
        .base_scroll = DT_INST_PROP(n, base_scroll),                                            \
        .fast_multiplier = DT_INST_PROP(n, fast_multiplier),                                    \
    };                                                                                          \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &config_##n, POST_KERNEL,                      \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &action_api);

DT_INST_FOREACH_STATUS_OKAY(ACTION_INST)
