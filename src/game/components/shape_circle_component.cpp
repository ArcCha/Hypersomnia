#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/physics/b2Fixture_index_in_component.h"
#include "game/components/shape_circle_component.h"
#include "game/systems_inferred/physics_system.h"

template <bool C>
bool basic_shape_circle_synchronizer<C>::is_activated() const {
	return get_raw_component().activated;
}

template <bool C>
real32 basic_shape_circle_synchronizer<C>::get_radius() const {
	return get_raw_component().radius;
}

using S = components::shape_circle;

void component_synchronizer<false, S>::reinference() const {
	handle.get_cosmos().partial_reinference<physics_system>(handle);
}

void component_synchronizer<false, S>::set_activated(const bool flag) const {
	if (flag == get_raw_component().activated) {
		return;
	}

	get_raw_component().activated = flag;
	reinference();
}

template class basic_shape_circle_synchronizer<false>;
template class basic_shape_circle_synchronizer<true>;