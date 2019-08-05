#pragma once
#include <optional>
#include <vector>
#include "view/character_camera.h"
#include "view/game_drawing_settings.h"
#include "view/necessary_resources.h"
#include "view/game_gui/special_indicator.h"
#include "view/audiovisual_state/particle_triangle_buffers.h"
#include "augs/graphics/renderer_settings.h"
#include "game/stateless_systems/visibility_system.h"

namespace augs {
	class thread_pool;

	namespace graphics { 
		class texture;
	}

	class renderer;
	struct baked_font;
}

struct frame_profiler;
struct audiovisual_state;
struct all_necessary_shaders;
struct all_necessary_fbos;
struct performance_settings;

class images_in_atlas_map;
class visible_entities;

/* Require all */

using illuminated_rendering_fbos = all_necessary_fbos;
using illuminated_rendering_shaders = all_necessary_shaders;

struct additional_highlight {
	entity_id id;
	rgba col;
};

struct illuminated_rendering_input {
	const character_camera camera;
	const camera_cone queried_cone;
	const vec2 pre_step_crosshair_displacement;
	const audiovisual_state& audiovisuals;
	const game_drawing_settings drawing;
	const necessary_images_in_atlas_map& necessary_images;
	const augs::baked_font& gui_font;
	const images_in_atlas_map& game_images;
	const double interpolation_ratio = 0.0;
	augs::renderer& renderer;
	frame_profiler& frame_performance;
	augs::graphics::texture* general_atlas;
	illuminated_rendering_fbos& fbos;
	const illuminated_rendering_shaders& shaders;
	const visible_entities& all_visible;
	const performance_settings& perf_settings;
	const augs::renderer_settings& renderer_settings;
	const std::vector<additional_highlight>& additional_highlights;
	const std::vector<special_indicator>& special_indicators;
	const special_indicator_meta& indicator_meta;
	const particle_triangle_buffers& drawn_particles;
	
	const std::vector<visibility_request>& light_requests;
	augs::thread_pool& pool;
};

void enqueue_illuminated_rendering_jobs(
	augs::thread_pool& pool, 
	const illuminated_rendering_input& in
);

void illuminated_rendering(const illuminated_rendering_input in);
