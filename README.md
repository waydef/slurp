# slurp

select a region in a wayland compositor and print it to standard output.

this fork adds smooth animations, rounded corners, and gap bridging for wayland compositors like hyprland, sway, and river.

## features in this fork

- rounded corners (`-r`): renders boxes and selections with corner radiuses (default: 20px), matching window rounding in modern compositors.
- smooth hover animations: interpolates box geometry and opacity when moving between windows instead of snapping abruptly.
- window gap bridging: bridges small gaps between tiled windows so the selection glides across adjacent splits without falling back to the whole display.

## dependencies

- `meson`
- `ninja`
- `wayland`
- `cairo`
- `libxkbcommon`
- `scdoc` (optional, for man pages)

## building and installing

```sh
git clone https://github.com/waydef/slurp.git
cd slurp
meson setup build
ninja -C build
sudo cp build/slurp /usr/local/bin/slurp
```

## usage

select a region interactively:

```sh
slurp
```

custom corner radius (for example, 12px, or 0 for sharp corners):

```sh
slurp -r 12
```

select a window under sway:

```sh
swaymsg -t get_tree | jq -r '.. | select(.pid? and .visible?) | .rect | "\(.x),\(.y) \(.width)x\(.height)"' | slurp
```

copy a selected area to clipboard with grim:

```sh
grim -g "$(slurp)" - | wl-copy
```

## license

mit. original work by simon ser and contributors.

## кратко на русском

форк утилиты slurp для wayland. добавляет скругление углов (-r), плавные анимации перехода рамки между окнами и преодоление межконтурных зазоров тайлинга (hyprland, sway, river).
