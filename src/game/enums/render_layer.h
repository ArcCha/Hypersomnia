#pragma once
/*

	TODO: Some render layers might correspond to distinct entity types.
	Rendering performance could be vastly improved by assigning an ordering based on these types,
	as we would avoid dispatching per-entity.
*/

enum class render_layer {
	// GEN INTROSPECTOR enum class render_layer
	INVALID,

	INSECTS,

	OVERLAID_CALLOUT_MARKERS,
	CALLOUT_MARKERS,

	POINT_MARKERS,
	AREA_MARKERS,
	LIGHTS,
	ILLUMINATING_WANDERING_PIXELS,
	CONTINUOUS_PARTICLES,
	CONTINUOUS_SOUNDS,
	DIM_WANDERING_PIXELS,

	NEON_CAPTIONS,
	FLYING_BULLETS,

	OVER_SENTIENCES,
	SENTIENCES,

	GLASS_BODY,
	OVER_SMALL_DYNAMIC_BODY,
	SMALL_DYNAMIC_BODY,
	OVER_DYNAMIC_BODY,
	DYNAMIC_BODY,

	CAR_WHEEL,
	CAR_INTERIOR,

	WATER_SURFACES,
	WATER_COLOR_OVERLAYS,
	AQUARIUM_BUBBLES,
	UPPER_FISH,
	BOTTOM_FISH,
	AQUARIUM_DUNES,
	AQUARIUM_FLOWERS,

	PLANTED_BOMBS,

	FLOOR_NEON_OVERLAY,
	BODY_NEON_OVERLAY,

	ON_ON_FLOOR,
	ON_FLOOR,
	FLOOR_AND_ROAD,
	GROUND,
	UNDER_GROUND,

	NEON_OCCLUDING_DYNAMIC_BODY,

	COUNT
	// END GEN INTROSPECTOR
};