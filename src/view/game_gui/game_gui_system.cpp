#include "augs/templates/container_templates.h"
#include "augs/gui/button_corners.h"

#include "game/enums/slot_function.h"

#include "game/detail/inventory/inventory_slot_handle.h"

#include "game/cosmos/entity_handle.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/cosmos/data_living_one_step.h"
#include "game/cosmos/cosmos.h"

#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"
#include "game/messages/performed_transfer_message.h"

#include "view/game_gui/game_gui_system.h"
#include "view/game_gui/game_gui_element_location.h"
#include "view/game_gui/elements/character_gui.h"
#include "view/game_gui/elements/slot_button.h"
#include "view/game_gui/elements/item_button.h"
#include "game/messages/changed_identities_message.h"

#include "game/detail/entity_handle_mixins/for_each_slot_and_item.hpp"
#include "game/detail/entity_handle_mixins/make_wielding_transfers.hpp"

static int to_hotbar_index(const game_gui_intent_type type) {
	switch (type) {
	case game_gui_intent_type::HOTBAR_BUTTON_0: return 0;
	case game_gui_intent_type::HOTBAR_BUTTON_1: return 1;
	case game_gui_intent_type::HOTBAR_BUTTON_2: return 2;
	case game_gui_intent_type::HOTBAR_BUTTON_3: return 3;
	case game_gui_intent_type::HOTBAR_BUTTON_4: return 4;
	case game_gui_intent_type::HOTBAR_BUTTON_5: return 5;
	case game_gui_intent_type::HOTBAR_BUTTON_6: return 6;
	case game_gui_intent_type::HOTBAR_BUTTON_7: return 7;
	case game_gui_intent_type::HOTBAR_BUTTON_8: return 8;
	case game_gui_intent_type::HOTBAR_BUTTON_9: return 9;
	default: return -1;
	}
}

static int to_special_action_index(const game_gui_intent_type type) {
	switch (type) {
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_1: return 0;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_2: return 1;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_3: return 2;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_4: return 3;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_5: return 4;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_6: return 5;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_7: return 6;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_8: return 7;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_9: return 8;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_10: return 9;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_11: return 10;
	case game_gui_intent_type::SPECIAL_ACTION_BUTTON_12: return 11;
	default: return -1;
	}
}

cosmic_entropy game_gui_system::get_and_clear_pending_events() {
	cosmic_entropy out;

	out.transfer_requests = pending_transfers;
	out.cast_spells_per_entity = spell_requests;

	pending_transfers.clear();
	spell_requests.clear();

	return out;
}

void game_gui_system::clear_all_pending_events() {
	pending_transfers.clear();
	spell_requests.clear();
}

character_gui& game_gui_system::get_character_gui(const entity_id id) {
	const auto it = character_guis.find(id);

	if (it == character_guis.end()) {
		auto& new_gui = (*character_guis.try_emplace(id).first).second;
		
		new_gui.action_buttons[0].bound_spell.set<exaltation>();
		new_gui.action_buttons[1].bound_spell.set<fury_of_the_aeons>();
		new_gui.action_buttons[2].bound_spell.set<ultimate_wrath_of_the_aeons>();
		new_gui.action_buttons[3].bound_spell.set<haste>();
		new_gui.action_buttons[4].bound_spell.set<echoes_of_the_higher_realms>();
		new_gui.action_buttons[5].bound_spell.set<electric_triad>();
		new_gui.action_buttons[6].bound_spell.set<electric_shield>();
		
		return new_gui;
	}
	
	return (*it).second;
}

const character_gui& game_gui_system::get_character_gui(const entity_id id) const {
	return character_guis.at(id);
}

slot_button& game_gui_system::get_slot_button(const inventory_slot_id id) {
	return slot_buttons[id];
}

const slot_button& game_gui_system::get_slot_button(const inventory_slot_id id) const {
	return slot_buttons.at(id);
}

item_button& game_gui_system::get_item_button(const entity_id id) {
	return item_buttons[id];
}

const item_button& game_gui_system::get_item_button(const entity_id id) const {
	return item_buttons.at(id);
}

void game_gui_system::queue_transfer(const item_slot_transfer_request req) {
	pending_transfers.push_back(req);
}

void game_gui_system::queue_transfers(const wielding_result res) {
	// ensure(res.successful());

	if (res.successful()) {
		concatenate(pending_transfers, res.transfers);
	}
}
	
bool game_gui_system::control_gui_world(
	const game_gui_context context,
	const augs::event::change change
) {
	const auto root_entity = context.get_subject_entity();

	if (root_entity.dead()) {
		return false;
	}

	const auto& cosm = root_entity.get_cosmos();
	auto& element = context.get_character_gui();

	if (root_entity.has<components::item_slot_transfers>()) {
		auto handle_drag_of = [&](const auto& item_entity) {
			auto& dragged_charges = element.dragged_charges;

			if (change.was_pressed(augs::event::keys::key::RMOUSE)) {
				if (world.held_rect_is_dragged) {
					pending_transfers.push_back(item_slot_transfer_request::drop_some(item_entity, dragged_charges));
					return true;
				}
			}

			if (change.msg == augs::event::message::wheel) {
				const auto item = item_entity.template get<components::item>();

				const auto delta = change.data.scroll.amount;

				dragged_charges += delta;

				if (dragged_charges <= 0) {
					dragged_charges = item.get_charges() + dragged_charges;
				}
				if (dragged_charges > item.get_charges()) {
					dragged_charges = dragged_charges - item.get_charges();
				}

				return true;
			}

			return false;
		};

		if (const auto held_rect = context.get_if<item_button_in_item>(world.rect_held_by_lmb)) {
			const auto& item_entity = cosm[held_rect.get_location().item_id];

			if (handle_drag_of(item_entity)) {
				return true;
			}
		}

		if (const auto held_rect = context.get_if<hotbar_button_in_character_gui>(world.rect_held_by_lmb)) {
			if (handle_drag_of(held_rect->get_assigned_entity(root_entity))) {
				return true;
			}
		}
	}

	const auto gui_events = world.consume_raw_input_and_generate_gui_events(
		context,
		change
	);

	world.respond_to_events(
		context,
		gui_events
	);

	if (!gui_events.empty()) {
		return true;
	}

	return false;
}

void game_gui_system::control_hotbar_and_action_button(
	const const_entity_handle gui_entity,
	const game_gui_intent intent
) {
	if (gui_entity.dead()) {
		return;
	}

	if (/* not_applicable */ 
		!gui_entity.has<components::item_slot_transfers>()
		|| !gui_entity.has<components::sentience>()
	) {
		return;
	}

	auto& gui = get_character_gui(gui_entity);

	{
		auto r = intent;

		if (r.was_pressed()) {
			auto requested_index = static_cast<std::size_t>(-1);

			if (r.intent == game_gui_intent_type::HOLSTER) {
				requested_index = 0;
			}
			else if (r.intent == game_gui_intent_type::HOLSTER_SECONDARY) {
				requested_index = 1;
			}

			if (requested_index != static_cast<std::size_t>(-1)) {
				const auto resolved_index = gui_entity.calc_hand_action(requested_index).hand_index;

				if (resolved_index != static_cast<std::size_t>(-1)) {
					auto new_setup = gui.get_actual_selection_setup(gui_entity);
					new_setup.hand_selections[resolved_index].unset();
					queue_transfers(gui.make_and_push_hotbar_selection_setup(new_setup, gui_entity));
				}
			}
		}
	}

	{
		const auto& i = intent;

		if (const auto hotbar_button_index = to_hotbar_index(i.intent);
			hotbar_button_index >= 0 && 
			static_cast<std::size_t>(hotbar_button_index) < gui.hotbar_buttons.size()
		) {
			auto& currently_held_index = gui.currently_held_hotbar_button_index;

			if (gui.hotbar_buttons[currently_held_index].get_assigned_entity(gui_entity).dead()) {
				if (currently_held_index > -1) {
					/* Clear currently held index because nothing is assigned already */
					currently_held_index = -1;
				}
			}

			if (gui.hotbar_buttons[hotbar_button_index].get_assigned_entity(gui_entity)) {
				/* Something is assigned to that button */
				if (i.was_pressed()) {
					const bool should_dual_wield = currently_held_index > -1;

					if (should_dual_wield) {
						const auto setup = gui.get_setup_from_button_indices(
							gui_entity, 
							currently_held_index, 
							hotbar_button_index
						);

						queue_transfers(gui.make_and_push_hotbar_selection_setup(setup, gui_entity));
						gui.push_new_setup_when_index_released = -1;
					}
					else {
						auto new_setup = gui.get_setup_from_button_indices(
							gui_entity, 
							hotbar_button_index
						);

						const auto actual_setup = gui.get_actual_selection_setup(gui_entity);

						gui.save_setup(actual_setup);

						if (new_setup == actual_setup) {
							auto& ar = new_setup.hand_selections;
							std::swap(ar[0], ar[1]);
						}

						queue_transfers(gui.make_wielding_transfers_for(new_setup, gui_entity));

						gui.push_new_setup = new_setup;
						gui.push_new_setup_when_index_released = hotbar_button_index;
					}

					currently_held_index = hotbar_button_index;
				}
				else {
					if (hotbar_button_index == currently_held_index) {
						currently_held_index = -1;
					}
				
					if (hotbar_button_index == gui.push_new_setup_when_index_released) {
						gui.push_setup(gui.push_new_setup);
						gui.push_new_setup_when_index_released = -1;
					}
				}
			}
		}
		else if (
			const auto special_action_index = to_special_action_index(i.intent);
			special_action_index >= 0 && 
			static_cast<std::size_t>(special_action_index) < gui.action_buttons.size()
		) {
			auto& action_b = gui.action_buttons[special_action_index];
			action_b.detector.update_appearance(i.was_pressed() ? gui_event::ldown : gui_event::lup);

			if (i.was_pressed()) {
				const auto bound_spell = action_b.bound_spell;

				if (bound_spell.is_set() && gui_entity.get<components::sentience>().is_learnt(bound_spell)) {
					spell_requests[gui_entity] = bound_spell;
				}
			}
		}
		else if (i.intent == game_gui_intent_type::PREVIOUS_HOTBAR_SELECTION_SETUP && i.was_pressed()) {
			const auto wielding = gui.make_wielding_transfers_for_previous_hotbar_selection_setup(gui_entity);

			if (wielding.successful()) {
				queue_transfers(wielding);
			}
		}
	}
}

void game_gui_system::build_tree_data(const game_gui_context context) {
	world.build_tree_data_into(context);
}

void game_gui_system::advance(
	const game_gui_context context,
	const augs::delta dt
) {
	world.advance_elements(context, dt);
}

void game_gui_system::rebuild_layouts(
	const game_gui_context context
) {
	const auto root_entity = context.get_subject_entity();
	const auto& necessarys = context.get_necessary_images();
	const auto& image_defs = context.get_image_metas();
	auto& element = context.get_character_gui();

	if (root_entity.has<components::item_slot_transfers>()) {
		const auto screen_size = context.get_screen_size();

		int max_hotbar_height = 0;

		{
			int total_width = 0;

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				const auto& hb = element.hotbar_buttons[i];

				const auto bbox = hb.get_bbox(necessarys, image_defs, root_entity);
				max_hotbar_height = std::max(max_hotbar_height, bbox.y);

				total_width += bbox.x;
			}

			const int left_rc_spacing = 2;
			const int right_rc_spacing = 1;

			int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

			const auto set_rc = [&](auto& hb) {
				const auto bbox = hb.get_bbox(necessarys, image_defs, root_entity);

				hb.rc = xywh(xywhi(current_x, screen_size.y - max_hotbar_height - 50, bbox.x + left_rc_spacing + right_rc_spacing, max_hotbar_height));

				current_x += bbox.x + left_rc_spacing + right_rc_spacing;
			};

			for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
				set_rc(element.hotbar_buttons[i]);
			}
		}

		{
			const auto action_button_size = context.get_necessary_images().at(assets::necessary_image_id::ACTION_BUTTON_BORDER).get_original_size();

			auto total_width = static_cast<int>(element.action_buttons.size()) * action_button_size.x;

			const int left_rc_spacing = 4;
			const int right_rc_spacing = 3;

			int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

			const auto set_rc = [&](auto& hb) {
				const auto bbox = action_button_size;

				hb.rc = xywh(xywhi(
					current_x, screen_size.y - action_button_size.y - 9 - max_hotbar_height - 50, 
					bbox.x + left_rc_spacing + right_rc_spacing, 
					action_button_size.y));

				current_x += bbox.x + left_rc_spacing + right_rc_spacing;
			};

			for (size_t i = 0; i < element.action_buttons.size(); ++i) {
				set_rc(element.action_buttons[i]);
			}
		}
	}

	world.rebuild_layouts(context);
}

template <class E>
bool should_fill_hotbar_from_right(const E& handle) {
	const auto& item = handle.template get<invariants::item>();

	if (item.categories_for_slot_compatibility.test(item_category::BACK_WEARABLE)) {
		if (!handle.template has<invariants::explosive>()) {
			return true;
		}
	}

	if (item.categories_for_slot_compatibility.test(item_category::BELT_WEARABLE)) {
		return true;
	}

	return false;
}

void game_gui_system::standard_post_solve(const const_logic_step step) {
	const auto& cosm = step.get_cosmos();

	auto migrate_nodes = [&](auto& migrated, const auto& key_map) {
		using M = remove_cref<decltype(migrated)>;

		M result;

		for (auto& it : migrated) {
			if (const auto new_id = mapped_or_nullptr(key_map, it.first)) {
				result.try_emplace(*new_id, std::move(it.second));
			}
			else {
				result.try_emplace(it.first, std::move(it.second));
			}
		}

		migrated = std::move(result);
	};

	for (const auto& changed : step.get_queue<messages::changed_identities_message>()) {
		migrate_nodes(item_buttons, changed.changes);
		migrate_nodes(character_guis, changed.changes);

		auto migrate_id = [&](auto& id) {
			if (const auto new_id = mapped_or_nullptr(changed.changes, id)) {
				id = *new_id;
			}
			else {
				id = {};
			}
		};

		for (auto& it : character_guis) {
			auto& ch = it.second;

			for (auto& h : ch.hotbar_buttons) {
				migrate_id(h.last_assigned_entity);
			}

			for (auto& l : ch.last_setups) {
				for (auto& s : l.hand_selections) {
					migrate_id(s);
				}
			}

			for (auto& s : ch.push_new_setup.hand_selections) {
				migrate_id(s);
			}
		}
	}

	for (const auto& transfer : step.get_queue<messages::performed_transfer_message>()) {
		const auto transferred_item = cosm[transfer.item];
		const auto target_slot = cosm[transfer.target_slot];

		const bool same_capability = transfer.result.relation == capability_relation::THE_SAME;

		const bool interested =
			target_slot.alive()
			&& transferred_item.alive()
			&& target_slot.get_type() != slot_function::PERSONAL_DEPOSIT
			&& (transfer.result.is_pickup() || (same_capability && !target_slot->is_mounted_slot()))
		;

		if (!interested) {
			continue;
		}

		const bool always_reassign_button = 
			transfer.result.is_pickup()
		;

		auto& gui = get_character_gui(transfer.target_root);

		if (!always_reassign_button) {
			if (gui.hotbar_assignment_exists_for(transferred_item)) {
				continue;
			}
		}

		auto add = [&](const auto handle) {
			gui.assign_item_to_first_free_hotbar_button(
				cosm[transfer.target_root],
				handle,
				should_fill_hotbar_from_right(handle)
			);
		};

		add(transferred_item);

		auto should_recurse = [](const auto item_entity) {
			const auto& item = item_entity.template get<invariants::item>();

			if (item.categories_for_slot_compatibility.test(item_category::BACK_WEARABLE)) {
				return true;
			}

			return false;
		};

		if (should_recurse(transferred_item)) {
			transferred_item.for_each_contained_item_recursive(
				[&add, should_recurse](const auto child_item) {
					add(child_item);

					if (should_recurse(child_item)) {
						return recursive_callback_result::CONTINUE_AND_RECURSE;
					}

					return recursive_callback_result::CONTINUE_DONT_RECURSE;
				}
			);
		}
	}
}

void game_gui_system::standard_post_cleanup(const const_logic_step step) {
	if (step.any_deletion_occured()) {
		clear_dead_entities(step.get_cosmos());
	}
}

void game_gui_system::clear_dead_entities(const cosmos& new_cosmos) {
	const auto eraser = [&](auto& caches) {
		erase_if(caches, [&](const auto& it) {
			return new_cosmos[it.first].dead();
		});
	};

	eraser(character_guis);
	eraser(item_buttons);
	eraser(slot_buttons);
}
