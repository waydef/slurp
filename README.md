# slurp

Select a region in a Wayland compositor and print it to standard output.

This fork adds smooth animations, rounded corners, and gap bridging for modern Wayland setups like Hyprland, Sway, and River.

## Features added in this fork

- **Rounded corners (`-R`)**: Renders boxes and selections with corner radiuses (default: 20px). Matches window rounding in modern compositors.
- **Smooth hover animations**: Interpolates box size, position, and opacity when moving between windows instead of jumping abruptly.
- **Window gap bridging**: Bridges small gaps between tiled windows so the selection glides across adjacent splits without snapping to the whole display.

## Dependencies

- `meson`
- `ninja`
- `wayland`
- `cairo`
- `libxkbcommon`
- `scdoc` (optional, for man pages)

## Building and installing

```sh
git clone https://github.com/waydef/slurp.git
cd slurp
meson setup build
ninja -C build
sudo cp build/slurp /usr/local/bin/slurp
```

## Usage

Select a region interactively:

```sh
slurp
```

Custom corner radius (e.g. 12px, or 0 for sharp corners):

```sh
slurp -R 12
```

Select a window under Sway:

```sh
swaymsg -t get_tree | jq -r '.. | select(.pid? and .visible?) | .rect | "\(.x),\(.y) \(.width)x\(.height)"' | slurp
```

Combined with grim to copy an area to clipboard:

```sh
grim -g "$(slurp)" - | wl-copy
```

## License

MIT. Original work by [Simon Ser](https://github.com/emersion/slurp) and contributors.
