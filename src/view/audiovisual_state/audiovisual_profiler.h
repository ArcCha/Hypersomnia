#pragma once
#include "augs/misc/profiler_mixin.h"

class audiovisual_profiler : public augs::profiler_mixin<audiovisual_profiler> {
public:
	audiovisual_profiler();

	// GEN INTROSPECTOR class audiovisual_profiler
	augs::time_measurements camera_visibility_query;
	
	augs::time_measurements advance;
	augs::time_measurements interpolation;
	augs::time_measurements particle_logic;
	augs::time_measurements wandering_pixels;
	augs::time_measurements sound_logic;

	augs::time_measurements post_solve;
	augs::time_measurements post_cleanup;

	augs::amount_measurements<std::size_t> visible_entities;
	// END GEN INTROSPECTOR
};