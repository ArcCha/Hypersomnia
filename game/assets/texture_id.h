#pragma once
#include "augs/math/vec2.h"

namespace assets {
	enum class texture_id {
		INVALID,

		BLANK,

		CRATE,
		CRATE_DESTROYED,
		CAR_INSIDE,
		CAR_FRONT,

		TRUCK_INSIDE,
		TRUCK_FRONT,

		JMIX114,
		
		DEAD_TORSO,

		TEST_CROSSHAIR,
		TEST_PLAYER,

		TORSO_MOVING_FIRST,
		TORSO_MOVING_LAST = TORSO_MOVING_FIRST + 5,

		VIOLET_TORSO_MOVING_FIRST,
		VIOLET_TORSO_MOVING_LAST = VIOLET_TORSO_MOVING_FIRST + 5,

		BLUE_TORSO_MOVING_FIRST,
		BLUE_TORSO_MOVING_LAST = BLUE_TORSO_MOVING_FIRST + 5,

		SMOKE_PARTICLE_FIRST,
		SMOKE_PARTICLE_LAST = SMOKE_PARTICLE_FIRST + 6,

		PIXEL_THUNDER_FIRST,
		PIXEL_THUNDER_LAST = PIXEL_THUNDER_FIRST + 5,

		ASSAULT_RIFLE,
		BILMER2000,
		KEK9,
		SUBMACHINE,
		PISTOL,
		SHOTGUN,
		URBAN_CYAN_MACHETE,

		TEST_BACKGROUND,
		TEST_BACKGROUND2,

		SAMPLE_MAGAZINE,
		SMALL_MAGAZINE,
		SAMPLE_SUPPRESSOR,
		ROUND_TRACE,
		PINK_CHARGE,
		PINK_SHELL,
		CYAN_CHARGE,
		CYAN_SHELL,
		GREEN_CHARGE,
		GREEN_SHELL,
		BACKPACK,
		RIGHT_HAND_ICON,
		LEFT_HAND_ICON,
		DROP_HAND_ICON,

		GUI_CURSOR,
		GUI_CURSOR_HOVER,
		GUI_CURSOR_ADD,
		GUI_CURSOR_MINUS,
		GUI_CURSOR_ERROR,
		
		HUD_CIRCULAR_BAR_MEDIUM,

		CONTAINER_OPEN_ICON,
		CONTAINER_CLOSED_ICON,

		BUTTON_WITH_CUTS,

		ATTACHMENT_CIRCLE_FILLED,
		ATTACHMENT_CIRCLE_BORDER,
		PRIMARY_HAND_ICON,
		SECONDARY_HAND_ICON,
		SHOULDER_SLOT_ICON,
		ARMOR_SLOT_ICON,
		CHAMBER_SLOT_ICON,
		DETACHABLE_MAGAZINE_ICON,
		GUN_MUZZLE_SLOT_ICON,

		METROPOLIS_TILE_FIRST,
		METROPOLIS_TILE_LAST = METROPOLIS_TILE_FIRST + 49,

		HAVE_A_PLEASANT,
		AWAKENING,
		METROPOLIS,

		BRICK_WALL,
		ROAD,
		ROAD_FRONT_DIRT,

		BLINK_FIRST,
		BLINK_LAST = BLINK_FIRST + 7,
		WANDERING_SQUARE,
		WANDERING_CROSS,

		TRUCK_ENGINE,

		MENU_GAME_LOGO,

		HOTBAR_BUTTON_LT,
		HOTBAR_BUTTON_RT,
		HOTBAR_BUTTON_RB,
		HOTBAR_BUTTON_LB,

		HOTBAR_BUTTON_L,
		HOTBAR_BUTTON_T,
		HOTBAR_BUTTON_R,
		HOTBAR_BUTTON_B,

		HOTBAR_BUTTON_INSIDE,

		COUNT,
	};
	
	vec2i get_size(texture_id);
}

namespace augs {
	class texture;
}

augs::texture& operator*(const assets::texture_id& id);
bool operator!(const assets::texture_id& id);
