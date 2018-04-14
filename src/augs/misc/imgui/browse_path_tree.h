#pragma once
#include "augs/filesystem/path.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_settings.h"

template <class F, class C>
void browse_path_tree(
	path_tree_settings& settings,
	const C& all_paths,
	F path_callback
) {
	using namespace augs::imgui;

	settings.do_tweakers();
	ImGui::Separator();

	const auto prettify_filenames = settings.prettify_filenames;

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	auto prettify = [prettify_filenames](const std::string& filename) {
		return prettify_filenames ? format_field_name(augs::path_type(filename).stem()) : filename;
	};

	if (settings.linear_view) {
		ImGui::Columns(3);

		text_disabled(prettify_filenames ? "Name" : "Filename");
		ImGui::NextColumn();
		text_disabled("Location");
		ImGui::NextColumn();
		text_disabled("Details");
		ImGui::NextColumn();

		ImGui::Separator();

		for (const auto& l : all_paths) {
			const auto prettified = prettify(l.get_filename());

			auto displayed_dir = l.get_directory();
			cut_preffix(displayed_dir, "content/");

			if (!filter.PassFilter(prettified.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				continue;
			}

			path_callback(l, prettified);
			ImGui::NextColumn();

			text_disabled(displayed_dir);

			ImGui::NextColumn();
			ImGui::NextColumn();
		}
	}
	else {

	}
}