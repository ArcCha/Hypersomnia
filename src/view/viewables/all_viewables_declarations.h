#pragma once
#include "augs/math/declare_math.h"
#include "augs/misc/declare_containers.h"

namespace assets {
	enum class game_image_id;
	enum class sound_buffer_id;
	enum class particle_effect_id;
}

template <class enum_key, class mapped>
using asset_map = augs::enum_associative_array<enum_key, mapped>;

namespace augs {
	class sound_buffer;
	struct sound_buffer_loading_input;
}

struct game_image_in_atlas;
struct game_image_loadables;
struct game_image_meta;
struct particle_effect;

using sound_buffer_inputs_map = asset_map<
	assets::sound_buffer_id,
	augs::sound_buffer_loading_input
>;

struct loaded_sounds;

using particle_effects_map = asset_map<
	assets::particle_effect_id,
	particle_effect
>;

using game_image_loadables_map = asset_map<
	assets::game_image_id,
	game_image_loadables
>;

using game_image_metas_map = asset_map<
	assets::game_image_id,
	game_image_meta
>;

using game_images_in_atlas_map = asset_map<
	assets::game_image_id,
	game_image_in_atlas
>;