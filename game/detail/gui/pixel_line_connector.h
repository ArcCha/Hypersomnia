#pragma once
#include "gui/rect.h"
#include <array>

std::vector<std::array<vec2i, 2>> get_connecting_pixel_lines(rects::ltrb<float> origin, rects::ltrb<float> target);
void draw_pixel_line_connector(rects::ltrb<float> origin, rects::ltrb<float> target,
	augs::gui::draw_info,
	augs::rgba col
	);
