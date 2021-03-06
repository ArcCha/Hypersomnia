#pragma once
#include "game/components/sprite_component.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/pad_bytes.h"

namespace components {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct components::wandering_pixels
		rgba colorize = white;
		unsigned particles_count = 0u;
		bool keep_particles_within_bounds = false;
		pad_bytes<3> pad;
		// END GEN INTROSPECTOR
	};
}

using wandering_pixels_frames = augs::constant_size_vector<invariants::sprite, 10>;

namespace invariants {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct invariants::wandering_pixels
		assets::plain_animation_id animation_id;
		unsigned max_direction_ms = 4000;
		unsigned direction_interp_ms = 700;
		augs::bound<unsigned> base_velocity = { 3, 38 };
		// END GEN INTROSPECTOR
	};
}