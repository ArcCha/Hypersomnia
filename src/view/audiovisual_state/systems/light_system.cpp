#include "augs/graphics/vertex.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/shader.h"
#include "augs/graphics/fbo.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

#include "game/components/light_component.h"
#include "game/components/render_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "game/messages/visibility_information.h"

#include "game/systems_stateless/visibility_system.h"

#include "view/viewables/game_image.h"
#include "view/rendering_scripts/draw_entity.h"

#include "view/audiovisual_state/systems/light_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/particles_simulation_system.h"

void light_system::reserve_caches_for_entities(const size_t n) {
	per_entity_cache.resize(n);
}

light_system::cache::cache() {
	std::fill(all_variation_values.begin(), all_variation_values.end(), 0.f);
}

void light_system::advance_attenuation_variations(
	const cosmos& cosmos,
	const augs::delta dt
) {
	global_time_seconds += dt.in_seconds();

	cosmos.for_each(
		processing_subjects::WITH_LIGHT,
		[&](const auto it) {
			const auto& light = it.get<components::light>();
			auto& cache = per_entity_cache[linear_cache_key(it)];

			const auto delta = dt.in_seconds();

			light.constant.variation.update_value(rng, cache.all_variation_values[0], delta);
			light.linear.variation.update_value(rng, cache.all_variation_values[1], delta);
			light.quadratic.variation.update_value(rng, cache.all_variation_values[2], delta);

			light.wall_constant.variation.update_value(rng, cache.all_variation_values[3], delta);
			light.wall_linear.variation.update_value(rng, cache.all_variation_values[4], delta);
			light.wall_quadratic.variation.update_value(rng, cache.all_variation_values[5], delta);

			light.position_variations[0].update_value(rng, cache.all_variation_values[6], delta);
			light.position_variations[1].update_value(rng, cache.all_variation_values[7], delta);
		}
	);
}

void light_system::render_all_lights(const light_system_input in) const {
	auto& renderer = in.renderer;
	
	const auto output = augs::drawer{ renderer.get_triangle_buffer() };
	const auto& light_shader = in.light_shader;
	const auto& standard_shader = in.standard_shader;
	const auto projection_matrix = in.projection_matrix;

	const auto& cosmos = in.cosm;

	ensure_eq(0, renderer.get_triangle_count());

	in.light_fbo.set_as_current();
	
	renderer.set_clear_color({ 25, 51, 51, 255 });
	renderer.clear_current_fbo();
	renderer.set_clear_color({ 0, 0, 0, 0 });

	light_shader.set_as_current();

	const auto light_pos_uniform = light_shader.get_uniform_location("light_pos");
	const auto light_max_distance_uniform = light_shader.get_uniform_location("max_distance");
	const auto light_attenuation_uniform = light_shader.get_uniform_location("light_attenuation");
	const auto light_multiply_color_uniform = light_shader.get_uniform_location("multiply_color");
	const auto projection_matrix_uniform = light_shader.get_uniform_location("projection_matrix");
	const auto& interp = in.interpolation;
	const auto& particles = in.particles;
	
	const auto& visible_per_layer = in.visible_per_layer;

	std::vector<messages::visibility_information_request> requests;
	std::vector<messages::visibility_information_response> responses;

	light_shader.set_projection(projection_matrix);

	auto draw_layer = [&](const render_layer r, const renderable_drawing_type type = renderable_drawing_type::NEON_MAPS) {
		draw_entities(visible_per_layer[r], cosmos, output, in.game_images, in.camera, global_time_seconds, in.interpolation, type);
	};

	cosmos.for_each(
		processing_subjects::WITH_LIGHT,
		[&](const auto light_entity) {
			const auto& cache = per_entity_cache[linear_cache_key(light_entity)];
			const auto light_displacement = vec2(cache.all_variation_values[6], cache.all_variation_values[7]);

			messages::visibility_information_request request;
			request.eye_transform = light_entity.get_viewing_transform(interp);
			request.eye_transform.pos += light_displacement;
			request.filter = filters::line_of_sight_query();
			request.square_side = light_entity.get<components::light>().max_distance.base_value;
			request.subject = light_entity;

			requests.push_back(request);
		}
	);

	{
		std::vector<messages::line_of_sight_response> dummy;
		visibility_system().respond_to_visibility_information_requests(cosmos, {}, requests, dummy, responses);
	}

	renderer.set_additive_blending();

	for (size_t i = 0; i < responses.size(); ++i) {
		const auto& r = responses[i];
		const auto& light_entity = cosmos[requests[i].subject];
		const auto& light = light_entity.get<components::light>();
		const auto world_light_pos = requests[i].eye_transform.pos;

		for (size_t t = 0; t < r.get_num_triangles(); ++t) {
			const auto world_light_tri = r.get_world_triangle(t, world_light_pos);
			augs::vertex_triangle renderable_light_tri;

			renderable_light_tri.vertices[0].pos = in.camera[world_light_tri[0]];
			renderable_light_tri.vertices[1].pos = in.camera[world_light_tri[1]];
			renderable_light_tri.vertices[2].pos = in.camera[world_light_tri[2]];

			auto considered_color = light.color;
			
			if (considered_color == black) {
				considered_color.set_hsv({ fmod(global_time_seconds / 16.f, 1.f), 1.0, 1.0 });
			}

			renderable_light_tri.vertices[0].color = considered_color;
			renderable_light_tri.vertices[1].color = considered_color;
			renderable_light_tri.vertices[2].color = considered_color;

			renderer.push_triangle(renderable_light_tri);
		}

		//for (size_t d = 0; d < r.get_num_discontinuities(); ++d) {
		//	const auto world_discontinuity = *r.get_discontinuity(d);
		//	
		//	if (!world_discontinuity.is_boundary) {
		//		vertex_triangle renderable_light_tri;
		//
		//		const float distance_from_light = (requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//		const float angle = 80.f / ((distance_from_light+0.1f)/50.f);
		//		
		//		//(requests[i].eye_transform.pos - world_discontinuity.points.first).length();
		//
		//		if (world_discontinuity.winding == world_discontinuity.RIGHT) {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(-angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//		else {
		//			renderable_light_tri.vertices[0].pos = world_discontinuity.points.first + camera_offset;
		//			renderable_light_tri.vertices[1].pos = world_discontinuity.points.second + camera_offset;
		//			renderable_light_tri.vertices[2].pos = vec2(world_discontinuity.points.second).rotate(angle, world_discontinuity.points.first) + camera_offset;
		//		}
		//
		//		renderable_light_tri.vertices[0].color = light.color;
		//		renderable_light_tri.vertices[1].color = light.color;
		//		renderable_light_tri.vertices[2].color = light.color;
		//
		//		renderer.push_triangle(renderable_light_tri);
		//	}
		//}

		const auto& cache = per_entity_cache[linear_cache_key(light_entity)];

		const auto light_frag_pos = in.camera.get_screen_space_revert_y(world_light_pos);

		light_shader.set_uniform(light_pos_uniform, light_frag_pos);
		light_shader.set_uniform(light_max_distance_uniform, light.max_distance.base_value);
		
		light_shader.set_uniform(
			light_attenuation_uniform,
			vec3 {
				cache.all_variation_values[0] + light.constant.base_value,
				cache.all_variation_values[1] + light.linear.base_value,
				cache.all_variation_values[2] + light.quadratic.base_value
			}
		);
		
		light_shader.set_uniform(
			light_multiply_color_uniform,
			white.rgb()
		);

		renderer.call_triangles();
		renderer.clear_triangles();
		
		light_shader.set_as_current();

		light_shader.set_uniform(light_max_distance_uniform, light.wall_max_distance.base_value);
		
		light_shader.set_uniform(light_attenuation_uniform,
			vec3 {
				cache.all_variation_values[3] + light.wall_constant.base_value,
				cache.all_variation_values[4] + light.wall_linear.base_value,
				cache.all_variation_values[5] + light.wall_quadratic.base_value
			}
		);
		
		light_shader.set_uniform(
			light_multiply_color_uniform,
			light.color.rgb()
		);
		
		draw_layer(render_layer::DYNAMIC_BODY, renderable_drawing_type::NORMAL);

		renderer.call_triangles();
		renderer.clear_triangles();

		light_shader.set_uniform(
			light_multiply_color_uniform,
			white.rgb()
		);
	}

	standard_shader.set_as_current();

	draw_layer(render_layer::DYNAMIC_BODY);
	draw_layer(render_layer::SMALL_DYNAMIC_BODY);
	draw_layer(render_layer::FLYING_BULLETS);
	draw_layer(render_layer::CAR_INTERIOR);
	draw_layer(render_layer::CAR_WHEEL);
	draw_layer(render_layer::NEON_CAPTIONS);
	draw_layer(render_layer::ON_GROUND);

	{
		components::sprite::drawing_input basic(output);
		basic.camera = in.camera;
		basic.use_neon_map = true;

		particles.draw_particles_as_sprites(
			in.game_images,
			basic,
			render_layer::ILLUMINATING_PARTICLES
		);
	}

	in.neon_callback();

	renderer.call_triangles();
	renderer.clear_triangles();

	renderer.set_standard_blending();

	augs::graphics::fbo::set_current_to_none();

	renderer.set_active_texture(2);
	in.light_fbo.get_texture().bind();
	renderer.set_active_texture(0);
}