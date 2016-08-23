#include "ingredients.h"

#include "game/components/position_copying_component.h"

#include "game/components/crosshair_component.h"
#include "game/components/sprite_component.h"
#include "game/components/movement_component.h"
#include "game/components/rotation_copying_component.h"
#include "game/components/animation_component.h"
#include "game/components/animation_response_component.h"
#include "game/components/physics_component.h"
#include "game/components/car_component.h"
#include "game/components/trigger_component.h"
#include "game/components/physics_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/special_physics_component.h"
#include "game/components/name_component.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace prefabs {
	entity_handle create_car(cosmos& world, components::transform spawn_transform) {
		auto front = world.create_entity("front");
		auto interior = world.create_entity("interior");
		auto left_wheel = world.create_entity("left_wheel");

		front.add_sub_entity(interior);
		front.add_sub_entity(left_wheel);
		name_entity(front, entity_name::TRUCK);

		{
			auto& sprite = front += components::sprite();
			auto& render = front += components::render();
			auto& transform = front += components::transform();
			auto& car = front += components::car();
			components::physics physics_definition;
			components::fixtures colliders;

			car.left_wheel_trigger = left_wheel;
			car.input_acceleration.set(2500, 4500) /= 3;
			car.acceleration_length = 4500 / 5;
			transform = spawn_transform;

			sprite.set(assets::texture_id::TRUCK_FRONT);
			//sprite.set(assets::texture_id::TRUCK_FRONT, augs::rgba(0, 255, 255));
			//sprite.size.x = 200;
			//sprite.size.y = 100;

			render.layer = render_layer::DYNAMIC_BODY;

			physics_definition.linear_damping = 0.4f;
			physics_definition.angular_damping = 2.f;

			auto& fixture = colliders.new_collider();
			fixture.shape.from_renderable(front);

			fixture.filter = filters::dynamic_object();
			fixture.density = 0.6f;

			front += physics_definition;
			front += colliders;
			front.get<components::fixtures>().set_owner_body(front);
			//physics.air_resistance = 0.2f;
		}

		{
			auto& sprite = interior += components::sprite();
			auto& render = interior += components::render();
			auto& transform = interior += components::transform();
			components::fixtures colliders;

			transform = spawn_transform;

			render.layer = render_layer::CAR_INTERIOR;

			sprite.set(assets::texture_id::TRUCK_INSIDE);
			//sprite.set(assets::texture_id::TRUCK_INSIDE, augs::rgba(122, 0, 122, 255));
			//sprite.size.x = 250;
			//sprite.size.y = 550;

			auto& fixture = colliders.new_collider();
			fixture.shape.from_renderable(interior);
			fixture.density = 0.6f;
			fixture.filter = filters::friction_ground();

			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2) * -1, 0);
			
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;
			colliders.is_friction_ground = true;

			interior += colliders;

			interior.get<components::fixtures>().set_owner_body(front);
		}

		{
			auto& sprite = left_wheel += components::sprite();
			auto& render = left_wheel += components::render();
			auto& transform = left_wheel += components::transform();
			auto& trigger = left_wheel += components::trigger();
			components::fixtures colliders;

			transform = spawn_transform;
			trigger.entity_to_be_notified = front;
			trigger.react_to_collision_detectors = false;
			trigger.react_to_query_detectors = true;

			render.layer = render_layer::CAR_WHEEL;

			sprite.set(assets::texture_id::CAR_INSIDE, augs::rgba(29, 0, 0, 255));
			sprite.size.set(60, 30);

			auto& fixture = colliders.new_collider();

			fixture.shape.from_renderable(left_wheel);
			fixture.density = 0.6f;
			fixture.filter = filters::trigger();
			fixture.sensor = true;

			vec2 offset((front.get<components::sprite>().size.x / 2 + sprite.size.x / 2 + 20) * -1, 0);
			colliders.offsets_for_created_shapes[colliders_offset_type::SHAPE_OFFSET].pos = offset;

			left_wheel += colliders;

			left_wheel.get<components::fixtures>().set_owner_body(front);
		}

		front.add_standard_components();
		left_wheel.add_standard_components();
		interior.add_standard_components();

		return front;
	}

}