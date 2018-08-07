#pragma once
#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/property_editor/commanding_property_editor_input.h"

template <
	class cmd_type,
	class Behaviour = default_edit_properties_behaviour,
	class T,
	class S = default_widget_provider,
	class D = default_sane_default_provider
>
void singular_edit_properties(
	const commanding_property_editor_input& in,
	T&& parent_altered,
	const std::string& property_location,
	S special_widget_provider = {},
	D sane_defaults = {},
	const int extra_columns = 0
) {
	using namespace augs::imgui;

	auto& cmd_in = in.command_in;
	auto& history = cmd_in.get_history();

	auto post_new_change = [&](
		const auto& description,
		const field_address& field,
		const auto& new_content
	) {
		cmd_type cmd;
		cmd.field = field;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, cmd_in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<cmd_type>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	general_edit_properties<Behaviour>(
		in.prop_in, 
		std::forward<T>(parent_altered),
		post_new_change,
		rewrite_last_change,
		true_returner(),
		special_widget_provider,
		sane_defaults,
		extra_columns
	);
}