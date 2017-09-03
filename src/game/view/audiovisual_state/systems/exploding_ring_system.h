#pragma once
#include "augs/misc/minmax.h"
#include "augs/misc/delta.h"

#include "augs/drawing/drawing.h"

#include "game/messages/exploding_ring_input.h"
#include "game/components/transform_component.h"

#include "augs/math/camera_cone.h"

class cosmos;
class particles_simulation_system;

class exploding_ring_system {
public:
	struct ring {
		exploding_ring_input in;
		double time_of_occurence_seconds = 0.0;
	};

	double global_time_seconds = 0.0;

	std::vector<ring> rings;

	void acquire_new_rings(const std::vector<exploding_ring_input>& rings);

	void advance(
		const cosmos& cosmos,
		const particle_effect_definitions&,
		const augs::delta dt,
		particles_simulation_system& particles_output_for_effects
	);

	void draw_rings(
		const augs::drawer_with_default output,
		augs::special_buffer& specials,
		const camera_cone camera,
		const cosmos& cosmos
	) const;

	void draw_highlights_of_rings(
		const augs::drawer output,
		const augs::texture_atlas_entry highlight_tex,
		const camera_cone camera,
		const cosmos& cosmos
	) const;

	void reserve_caches_for_entities(const size_t) const {}
};