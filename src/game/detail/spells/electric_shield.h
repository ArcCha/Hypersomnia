#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/detail/spells/spell_structs.h"

struct electric_shield_instance {
	augs::stepped_cooldown cast_cooldown;
};

struct electric_shield {
	using instance = electric_shield_instance;

	spell_common_data common;
	spell_appearance appearance;
	unsigned perk_duration_seconds = 0u;
};