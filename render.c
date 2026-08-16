#include <cairo/cairo.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "pool-buffer.h"
#include "render.h"
#include "slurp.h"

static void set_source_u32(cairo_t *cairo, uint32_t color) {
	cairo_set_source_rgba(cairo, (color >> (3 * 8) & 0xFF) / 255.0,
		(color >> (2 * 8) & 0xFF) / 255.0,
		(color >> (1 * 8) & 0xFF) / 255.0,
		(color >> (0 * 8) & 0xFF) / 255.0);
}

static void set_source_u32_alpha(cairo_t *cairo, uint32_t color, double alpha_mult) {
	double a = ((color >> (0 * 8) & 0xFF) / 255.0) * alpha_mult;
	cairo_set_source_rgba(cairo, (color >> (3 * 8) & 0xFF) / 255.0,
		(color >> (2 * 8) & 0xFF) / 255.0,
		(color >> (1 * 8) & 0xFF) / 255.0,
		a);
}

static double get_box_radius(struct slurp_output *output, double w, double h, double default_radius) {
	// Fullscreen monitor outputs have sharp rectangular corners
	if (w >= output->logical_geometry.width - 2 && h >= output->logical_geometry.height - 2) {
		return 0.0;
	}
	return default_radius;
}

static void draw_rounded_rect(cairo_t *cairo, double x, double y, double w, double h, double r) {
	if (r <= 0.0 || w <= 0.0 || h <= 0.0) {
		cairo_rectangle(cairo, x, y, w, h);
		return;
	}
	double max_r = (w < h ? w : h) / 2.0;
	if (r > max_r) {
		r = max_r;
	}
	double degrees = 3.14159265358979323846 / 180.0;
	cairo_new_sub_path(cairo);
	cairo_arc(cairo, x + w - r, y + r, r, -90.0 * degrees, 0.0 * degrees);
	cairo_arc(cairo, x + w - r, y + h - r, r, 0.0 * degrees, 90.0 * degrees);
	cairo_arc(cairo, x + r, y + h - r, r, 90.0 * degrees, 180.0 * degrees);
	cairo_arc(cairo, x + r, y + r, r, 180.0 * degrees, 270.0 * degrees);
	cairo_close_path(cairo);
}

void render(struct slurp_output *output) {
	struct slurp_state *state = output->state;
	struct pool_buffer *buffer = output->current_buffer;
	cairo_t *cairo = buffer->cairo;

	// Clear
	cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
	set_source_u32(cairo, state->colors.background);
	cairo_paint(cairo);

	// Draw base option boxes from input
	struct slurp_box *choice_box;
	wl_list_for_each(choice_box, &state->boxes, link) {
		if (box_intersect(&output->logical_geometry, choice_box)) {
			double r = get_box_radius(output, choice_box->width, choice_box->height, state->border_radius);
			set_source_u32(cairo, state->colors.choice);
			draw_rounded_rect(cairo, choice_box->x, choice_box->y,
				choice_box->width, choice_box->height, r);
			cairo_fill(cairo);
		}
	}

	struct slurp_seat *seat;
	wl_list_for_each(seat, &state->seats, link) {
		struct slurp_selection *current_selection =
			slurp_seat_current_selection(seat);

		// 1. Crosshairs if enabled
		if (!current_selection->has_selection && state->crosshairs) {
			struct slurp_box *output_box = &output->logical_geometry;
			if (in_box(output_box, current_selection->x, current_selection->y)) {
				set_source_u32(cairo, state->colors.border);
				cairo_rectangle(cairo, output_box->x, current_selection->y, output->logical_geometry.width, 1);
				cairo_fill(cairo);
				cairo_rectangle(cairo, current_selection->x, output->logical_geometry.y, 1, output->logical_geometry.height);
				cairo_fill(cairo);
			}
		}

		// 2. Smooth animated hover highlight over windows
		if (seat->button_state == WL_POINTER_BUTTON_STATE_RELEASED &&
				seat->anim.active && seat->anim.alpha > 0.005) {
			struct slurp_box anim_geom = {
				.x = (int32_t)seat->anim.x,
				.y = (int32_t)seat->anim.y,
				.width = (int32_t)seat->anim.width,
				.height = (int32_t)seat->anim.height,
			};
			if (box_intersect(&output->logical_geometry, &anim_geom)) {
				double r = get_box_radius(output, seat->anim.width, seat->anim.height, state->border_radius);
				draw_rounded_rect(cairo, seat->anim.x, seat->anim.y,
					seat->anim.width, seat->anim.height, r);
				set_source_u32_alpha(cairo, state->colors.selection, seat->anim.alpha);
				cairo_fill_preserve(cairo);

				cairo_set_line_width(cairo, state->border_weight);
				set_source_u32_alpha(cairo, state->colors.border, seat->anim.alpha);
				cairo_stroke(cairo);
			}
		}

		// 3. Active manual drag selection
		if (seat->button_state == WL_POINTER_BUTTON_STATE_PRESSED && current_selection->has_selection) {
			if (!box_intersect(&output->logical_geometry, &current_selection->selection)) {
				continue;
			}
			struct slurp_box *sel_box = &current_selection->selection;

			double r = get_box_radius(output, sel_box->width, sel_box->height, state->border_radius);
			draw_rounded_rect(cairo, sel_box->x, sel_box->y,
				sel_box->width, sel_box->height, r);
			set_source_u32(cairo, state->colors.selection);
			cairo_fill_preserve(cairo);

			cairo_set_line_width(cairo, state->border_weight);
			set_source_u32(cairo, state->colors.border);
			cairo_stroke(cairo);

			if (state->display_dimensions) {
				cairo_select_font_face(cairo, state->font_family,
						       CAIRO_FONT_SLANT_NORMAL,
						       CAIRO_FONT_WEIGHT_NORMAL);
				cairo_set_font_size(cairo, 14);
				set_source_u32(cairo, state->colors.border);
				char dimensions[12];
				snprintf(dimensions, sizeof(dimensions), "%ix%i",
					 sel_box->width, sel_box->height);
				cairo_move_to(cairo, sel_box->x + sel_box->width + 10,
					      sel_box->y + sel_box->height + 20);
				cairo_show_text(cairo, dimensions);
			}
		}
	}
}
