/*
 * Kyria battery diagnostic typing behavior.
 *
 * Prints two lines into the focused application, for example:
 * LEFT   [########----]  67%
 * RIGHT  [##########--]  83%
 *
 * The function-layer trigger is on the left half, so the event's split
 * source identifies the LEFT peripheral. The other of the two slots is RIGHT;
 * this avoids assuming the halves were paired in a particular order.
 *
 * A level of zero is also ZMK's disconnected/unavailable sentinel. Display
 * "--%" in that case rather than misrepresenting it as a measured empty cell.
 */

#define DT_DRV_COMPAT zmk_behavior_kyria_battery

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/atomic.h>
#include <zephyr/logging/log.h>
#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/behavior_queue.h>
#include <zmk/hid.h>
#include <zmk/split/central.h>
#include <dt-bindings/zmk/keys.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#define KP_DEV DEVICE_DT_NAME(DT_INST(0, zmk_behavior_key_press))
#define BAR_WIDTH 12
#define TAP_MS 20
#define WAIT_MS 10
#define END_SEQUENCE 1

static atomic_t active_sequences;

/* ASCII for an English/US HID keycode map, matching the existing keymap. */
static uint32_t ascii_key(char ch) {
    switch (ch) {
    case 'L': return LS(L);
    case 'E': return LS(E);
    case 'F': return LS(F);
    case 'T': return LS(T);
    case 'R': return LS(R);
    case 'I': return LS(I);
    case 'G': return LS(G);
    case 'H': return LS(H);
    case ' ': return SPACE;
    case '[': return LBKT;
    case ']': return RBKT;
    case '#': return LS(N3);
    case '-': return MINUS;
    case '%': return LS(N5);
    case '\n': return ENTER;
    case '0': return N0;
    case '1': return N1;
    case '2': return N2;
    case '3': return N3;
    case '4': return N4;
    case '5': return N5;
    case '6': return N6;
    case '7': return N7;
    case '8': return N8;
    case '9': return N9;
    default: return SPACE;
    }
}

static void type_char(struct zmk_behavior_binding_event *event, char ch) {
    struct zmk_behavior_binding kp = {
        .behavior_dev = KP_DEV,
        .param1 = ascii_key(ch),
    };
    zmk_behavior_queue_add(event, kp, true, TAP_MS);
    zmk_behavior_queue_add(event, kp, false, WAIT_MS);
}

static void type_text(struct zmk_behavior_binding_event *event, const char *s) {
    while (*s) {
        type_char(event, *s++);
    }
}

static void type_line(struct zmk_behavior_binding_event *event,
                      const char *label, uint8_t pct) {
    type_text(event, label);
    type_char(event, '[');

    /* Round percentage to the closest of the 12 bar segments. */
    unsigned filled = pct == 0 ? 0 : ((unsigned)pct * BAR_WIDTH + 50) / 100;
    if (filled > BAR_WIDTH) {
        filled = BAR_WIDTH;
    }
    for (unsigned i = 0; i < BAR_WIDTH; i++) {
        type_char(event, i < filled ? '#' : '-');
    }
    type_text(event, "]  ");

    if (pct == 0) {
        type_text(event, "--");
    } else if (pct >= 100) {
        type_text(event, "100");
    } else {
        if (pct >= 10) {
            type_char(event, '0' + pct / 10);
        }
        type_char(event, '0' + pct % 10);
    }

    type_text(event, "%\n");
}

static void finish_sequence(void) {
    if (atomic_dec(&active_sequences) == 1) {
        zmk_hid_masked_modifiers_clear();
    }
}

static int on_pressed(struct zmk_behavior_binding *binding,
                      struct zmk_behavior_binding_event event) {
    const unsigned left_slot = event.source;
    const unsigned right_slot = left_slot == 0 ? 1 : 0;
    uint8_t left = 0, right = 0;

    if (left_slot >= ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT ||
        ZMK_SPLIT_CENTRAL_PERIPHERAL_COUNT != 2) {
        return ZMK_BEHAVIOR_OPAQUE;
    }

    zmk_split_central_get_peripheral_battery_level(left_slot, &left);
    zmk_split_central_get_peripheral_battery_level(right_slot, &right);

    /* Prevent a held modifier from altering the generated ASCII keystrokes. */
    atomic_inc(&active_sequences);
    zmk_hid_masked_modifiers_set(0xFF);

    type_line(&event, "LEFT   ", left);
    type_line(&event, "RIGHT  ", right);

    struct zmk_behavior_binding end = {
        .behavior_dev = binding->behavior_dev,
        .param1 = END_SEQUENCE,
    };
    if (zmk_behavior_queue_add(&event, end, false, 0) < 0) {
        LOG_WRN("Kyria battery print: behavior queue full");
        finish_sequence();
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_released(struct zmk_behavior_binding *binding,
                       struct zmk_behavior_binding_event event) {
    if (binding->param1 == END_SEQUENCE) {
        finish_sequence();
    }
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api api = {
    .binding_pressed = on_pressed,
    .binding_released = on_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = zmk_behavior_get_empty_param_metadata,
#endif
};

#define INST(n)                                                                                \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                             \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &api);

DT_INST_FOREACH_STATUS_OKAY(INST)
