#pragma once

inline auto get_current_callout(const cosmos& cosm, const vec2 pos) {
	auto& entities = thread_local_visible_entities();

	tree_of_npo_filter tree_types;
	tree_types.types[tree_of_npo_type::CALLOUT_MARKERS] = true;

	entities.acquire_non_physical({
		cosm,
		camera_cone(camera_eye(pos, 1.f), vec2i::square(1)),
		accuracy_type::EXACT,
		render_layer_filter::all(),
		tree_types
	});

	if (const auto result = entities.get_first_fulfilling([](auto&&...){ return true; }); result.is_set()) {
		return result;
	}

	return entity_id();
}

template <class E, class I>
entity_id get_current_callout(const E& viewed_character, const I& interp) {
	if (viewed_character) {
		if (const auto tr = viewed_character.find_viewing_transform(interp)) {
			return ::get_current_callout(viewed_character.get_cosmos(), tr->pos);
		}
	}

	return entity_id();
}

