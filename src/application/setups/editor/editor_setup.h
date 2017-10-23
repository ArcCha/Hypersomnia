#pragma once
#include <future>
#include <map>

#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"

#include "application/workspace.h"

#include "application/debug_character_selection.h"
#include "application/debug_settings.h"

#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_tab.h"
#include "application/setups/editor/editor_recent_paths.h"
#include "application/setups/editor/current_tab_access_cache.h"

struct config_lua_table;

namespace augs {
	class window;

	namespace event {
		struct change;
		struct state;
	}
}

struct editor_popup {
	std::string title;
	std::string message;
	std::string details;

	bool details_expanded = false;
};

class editor_setup : private current_tab_access_cache<editor_setup> {
	friend class current_tab_access_cache<editor_setup>;

	double global_time_seconds = 0.0;

	struct autosave_input {
		sol::state& lua;
	};

	const autosave_input destructor_autosave_input;
	augs::timer autosave_timer;

	editor_settings settings;

	std::optional<editor_popup> current_popup;

	bool show_summary = true;
	bool show_player = true;
	bool show_common_state = false;
	bool show_entities = false;
	bool show_go_to_all = false;

	double player_speed = 1.0;
	bool player_paused = true;

	editor_recent_paths recent;

	std::vector<editor_tab> tabs;
	std::vector<std::unique_ptr<workspace>> works;
	
	entity_id hovered_entity;

	void on_tab_changed();
	void set_locally_viewed(const entity_id);

	template <class F>
	bool try_to_open_new_tab(F&& new_workspace_provider) {
		const auto new_index = has_current_tab() ? current_index + 1 : 0;

		works.reserve(works.size() + 1);
		tabs.reserve(tabs.size() + 1);

		tabs.emplace(tabs.begin() + new_index);
		works.emplace(works.begin() + new_index, std::make_unique<workspace>());

		if (const bool successfully_opened = new_workspace_provider(tabs[new_index], *works[new_index])) {
			set_current_tab(new_index);
			return true;
		}

		tabs.erase(tabs.begin() + new_index);
		works.erase(works.begin() + new_index);

		return false;
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_file_dialog;
	std::future<std::optional<std::string>> save_file_dialog;

	void set_popup(const editor_popup);
	
	using path_operation = workspace_path_op;

	bool open_workspace_in_new_tab(path_operation);
	void save_current_tab_to(path_operation);

	void fill_with_minimal_scene(sol::state& lua);
	void fill_with_test_scene(sol::state& lua);

	void autosave(const autosave_input) const;
	void open_last_tabs(sol::state& lua);

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool handles_escape = true;
	static constexpr bool has_additional_highlights = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& workspace_path);
	
	~editor_setup();

	FORCE_INLINE auto get_audiovisual_speed() const {
		return player_paused ? 0.0 : player_speed;
	}

	FORCE_INLINE const auto& get_viewed_cosmos() const {
		return has_current_tab() ? work().world : cosmos::empty; 
	}

	FORCE_INLINE auto get_interpolation_ratio() const {
		return timer.fraction_of_step_until_next_step(get_viewed_cosmos().get_fixed_delta());
	}

	FORCE_INLINE auto get_viewed_character_id() const {
		return has_current_tab() ? work().locally_viewed : entity_id();
	}

	FORCE_INLINE auto get_viewed_character() const {
		return get_viewed_cosmos()[get_viewed_character_id()];
	}

	FORCE_INLINE const auto& get_viewable_defs() const {
		return has_current_tab() ? work().viewables : all_viewables_defs::empty;
	}

	void perform_custom_imgui(
		sol::state& lua,
		augs::window& owner,
		const bool in_direct_gameplay,
		const camera_cone current_cone
	);

	void customize_for_viewing(config_lua_table& cfg) const;
	void apply(const config_lua_table& cfg);

	template <class... Callbacks>
	void advance(
		augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		global_time_seconds += frame_delta.in_seconds();

		if (!player_paused) {
			timer.advance(frame_delta *= player_speed);
		}

		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			total_collected_entropy.clear_dead_entities(work().world);

			work().advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);

	bool handle_top_level_window_input(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua
	);

	bool handle_unfetched_window_input(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua,

		const camera_cone current_cone
	);

	bool escape();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(sol::state& lua, const augs::window& owner);
	void save_as(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void play_pause();
	void stop();
	void prev();
	void next();

	void new_tab();
	void next_tab();
	void prev_tab();
	void close_tab();
	void close_tab(const std::size_t i);

	void go_to_all();
	void open_containing_folder();

	template <class F>
	void for_each_additional_highlight(F callback) const {
		if (has_current_tab() && player_paused) {
			if (get_viewed_character().alive()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(work().locally_viewed, color);
			}

			for (const auto& e : tab().selected_entities) {
				callback(e, settings.selected_entity_color);
			}

			if (work().world[hovered_entity].alive()) {
				callback(hovered_entity, settings.hovered_entity_color);
			}
		}
	}

	FORCE_INLINE auto get_camera_panning() const {
		return has_current_tab() ? tab().panning : vec2::zero;
	}
};