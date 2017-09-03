#pragma once
#include "game/view/audiovisual_state/systems/interpolation_system.h"
#include "game/view/audiovisual_state/systems/past_infection_system.h"
#include "game/view/audiovisual_state/systems/light_system.h"
#include "game/view/audiovisual_state/systems/particles_simulation_system.h"
#include "game/view/audiovisual_state/systems/wandering_pixels_system.h"
#include "game/view/audiovisual_state/systems/sound_system.h"
#include "game/view/audiovisual_state/systems/flying_number_indicator_system.h"
#include "game/view/audiovisual_state/systems/pure_color_highlight_system.h"
#include "game/view/audiovisual_state/systems/exploding_ring_system.h"
#include "game/view/audiovisual_state/systems/thunder_system.h"

namespace augs {
	template <class...>
	class storage_for_systems;
}

using all_audiovisual_systems = augs::storage_for_systems<
	interpolation_system,
	past_infection_system,
	light_system,
	particles_simulation_system,
	wandering_pixels_system,
	sound_system,
	flying_number_indicator_system,
	pure_color_highlight_system,
	exploding_ring_system,
	thunder_system
>;