#!/usr/bin/env python3
"""Convert the ZMK Halcyon Kyria keymap into keymap-drawer input.

The Kyria's firmware has 60 positions: 0-49 are physical keys, 50 and 59
are encoder push buttons, and 51-58 are unused module placeholders.
The drawing displays 52 physical controls and excludes those 8 placeholders.
This converter expects one key binding and its numeric // position on each line.
"""
import argparse
import json
from pathlib import Path
import re

LAYERS = ("base_layer", "num_layer", "function_layer", "nav_layer", "sym_layer")
VISIBLE_POSITIONS = list(range(51)) + [59]
UNUSED_POSITIONS = set(range(51, 59))
MODIFIERS = {"LG": "Win", "LC": "Ctrl", "LS": "Shift", "LA": "Alt",
             "RG": "RWin", "RC": "RCtrl", "RS": "RShift", "RA": "RAlt"}
KEY_NAMES = {
    "LGUI": "Win", "LEFT_GUI": "Win", "LCTRL": "Ctrl", "LSHIFT": "Shift",
    "LEFT_SHIFT": "Shift", "LALT": "Alt", "RCTRL": "RCtrl",
    "SPACE": "Space", "BSPC": "Bksp", "BACKSPACE": "Bksp", "DEL": "Del",
    "ENTER": "Enter", "ESC": "Esc", "ESCAPE": "Esc", "TAB": "Tab",
    "SQT": "'", "MINUS": "-", "EQUAL": "=", "PLUS": "+", "STAR": "*",
    "FSLH": "/", "BSLH": "\\", "DOT": ".", "COMMA": ",",
    "COLON": ":", "SEMI": ";", "AT": "@", "HASH": "#", "DLLR": "$",
    "PRCNT": "%", "CARET": "^", "AMPS": "&", "QMARK": "?",
    "EXCL": "!", "PIPE": "|", "LPAR": "(", "RPAR": ")",
    "LBKT": "[", "RBKT": "]", "LBRC": "{", "RBRC": "}",
    "LESS_THAN": "<", "GREATER_THAN": ">", "TILDE": "~",
    "LEFT": "←", "RIGHT": "→", "UP": "↑", "DOWN": "↓",
    "PG_DN": "PgDn", "PG_UP": "PgUp", "PRINTSCREEN": "PrtSc",
    "SCRL_DOWN": "Wheel↓", "SCRL_UP": "Wheel↑",
    "SCRL_LEFT": "Wheel←", "SCRL_RIGHT": "Wheel→",
}


def code_label(code):
    code = code.strip()
    match = re.fullmatch(r"([A-Z]+)\((.*)\)", code)
    if match and match.group(1) in MODIFIERS:
        return MODIFIERS[match.group(1)] + "+" + code_label(match.group(2))
    if re.fullmatch(r"N[0-9]", code):
        return code[1:]
    return KEY_NAMES.get(code, code)


def binding_label(binding):
    tokens = binding.split()
    behavior = tokens[0]
    args = tokens[1:]
    if behavior == "&none":
        return {"tap": "–"}
    if behavior == "&trans":
        return {"tap": "▽", "type": "trans"}
    if behavior in ("&hml", "&hmr", "&phtl", "&phtr", "&phtl_scroll"):
        if len(args) != 2:
            raise ValueError("Expected 2 args for hold-tap: " + binding)
        return {"tap": code_label(args[1]), "hold": code_label(args[0])}
    if behavior == "&kp":
        return {"tap": code_label(" ".join(args))}
    if behavior == "&sk":
        return {"tap": "Sticky " + code_label(" ".join(args))}
    if behavior == "&sl":
        return {"tap": "SL " + " ".join(args)}
    if behavior == "&tog":
        return {"tap": "Tog " + " ".join(args)}
    if behavior == "&to":
        return {"tap": "To " + " ".join(args)}
    if behavior == "&mkp":
        return {"tap": "Mouse " + " ".join(args).replace("MB", "")}
    if behavior == "&dot_qmark":
        return {"tap": ".", "shifted": "?"}
    if behavior == "&comma_excl":
        return {"tap": ",", "shifted": "!"}
    special = {"&key_repeat": "Repeat", "&caps_word": "CapsWord",
               "&m_amp_c_dot": "&c."}
    return {"tap": special.get(behavior, binding.replace("&", "", 1))}


def read_layer(source, name):
    match = re.search(r"(?ms)^\s*" + re.escape(name) + r"\s*\{(.*?)^\s*\};", source)
    if not match:
        raise ValueError("Missing layer: " + name)
    body = match.group(1)
    bindings = re.search(r"\bbindings\s*=\s*<(.+?)>;", body, re.S)
    if not bindings:
        raise ValueError("Missing bindings in " + name)
    entries = re.findall(r"(?m)^\s*(&[^\n/]*?)\s*//\s*(\d{1,2})\s*$", bindings.group(1))
    positions = {}
    for action, number in entries:
        position = int(number)
        if position in positions:
            raise ValueError(f"Duplicate position {position} in {name}")
        positions[position] = action.strip()
    if set(positions) != set(range(60)):
        raise ValueError(f"Expected positions 0-59 in {name}; got {sorted(positions)}")
    for pos in UNUSED_POSITIONS:
        if positions[pos] != "&none":
            raise ValueError(f"Nonempty unused module position {pos} on {name}")
    rotary = re.search(r"\bsensor-bindings\s*=\s*(.*?);", body, re.S)
    sensors = re.findall(r"<([^<>]+)>", rotary.group(1)) if rotary else []
    if sensors and len(sensors) != 4:
        raise ValueError(f"Expected four sensor bindings in {name}")
    return positions, sensors


def annotate_encoder(key, name, sensor):
    if key["tap"] == "–":
        key["tap"] = "Enc " + name
    if not sensor or sensor.startswith("&noop_sensor"):
        return
    parts = sensor.split()
    if len(parts) != 3 or parts[0] not in ("&inc_dec_kp", "&scroll_encoder"):
        raise ValueError("Unsupported encoder binding: " + sensor)
    key["shifted"] = "↻ " + code_label(parts[1])
    key["hold"] = "↺ " + code_label(parts[2])


def run(root, output_dir):
    source = (root / "config/halcyon_kyria.keymap").read_text(encoding="utf-8")
    keymap_editor_layout = json.loads((root / "config/halcyon_kyria.json").read_text(encoding="utf-8"))
    physical = keymap_editor_layout["layouts"]["halcyon_transform"]["layout"]
    # The Keymap Editor JSON contains all 60 firmware positions, including
    # eight unpopulated module slots. Only the first 50 are ordinary keys.
    if len(physical) != 60:
        raise ValueError(f"Expected 60 firmware positions, got {len(physical)}")
    geometry = [{"x": k["x"], "y": k["y"]} for k in physical[:50]]
    geometry.extend([{"x": 6.5, "y": 0}, {"x": 8.5, "y": 0}])
    assert len(geometry) == len(VISIBLE_POSITIONS) == 52

    labels = {"base_layer": "Base", "num_layer": "Num / Mouse", "function_layer": "Function",
              "nav_layer": "Navigation", "sym_layer": "Symbols"}
    layers = {}
    for name in LAYERS:
        positions, sensors = read_layer(source, name)
        keys = []
        for pos in VISIBLE_POSITIONS:
            key = binding_label(positions[pos])
            key["tl"] = f"{pos:02}"
            if pos in (50, 59):
                annotate_encoder(key, "L" if pos == 50 else "R", sensors[0 if pos == 50 else 1] if sensors else "")
            keys.append(key)
        layers[labels[name]] = keys

    output_dir.mkdir(exist_ok=True, parents=True)
    (output_dir / "halcyon_kyria-layout.json").write_text(json.dumps(geometry, indent=2) + "\n", encoding="utf-8")
    # JSON is valid YAML; avoiding extra runtime dependencies for this conversion step.
    (output_dir / "halcyon_kyria.yaml").write_text(json.dumps({"layers": layers}, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Generated {len(layers)} layers of {len(geometry)} physical controls (8 unused slots omitted)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", type=Path, default=Path("."))
    parser.add_argument("--output", type=Path, default=Path("diagrams"))
    args = parser.parse_args()
    run(args.root, args.output)
