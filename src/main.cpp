#include <iostream>
#include <clocale>

#include "augs/log.h"
#include "augs/log_path_getters.h"
#include "augs/filesystem/file.h"
#include "augs/window_framework/shell.h"

#include "cmd_line_params.h"
#include "build_info.h"

#if PLATFORM_WINDOWS
#include <Windows.h>
#undef MIN
#undef MAX
#endif

#include "augs/window_framework/create_process.h"
#include "application/main/new_and_old_hypersomnia_path.h"
#include "work_result.h"

work_result work(const int argc, const char* const * const argv);

#if PLATFORM_WINDOWS
#if BUILD_IN_CONSOLE_MODE
int main(const int argc, const char* const * const argv) {
#else

HINSTANCE g_myhinst;

int __stdcall WinMain(HINSTANCE myhinst, HINSTANCE, char*, int) {
	g_myhinst = myhinst;
	const auto argc = __argc;
	const auto argv = __argv;
#endif
#elif PLATFORM_UNIX
int main(const int argc, const char* const * const argv) {
#else
#error "Unsupported platform!"
#endif
	/* 
		At least on Linux, 
		we need to call this in order to be able to write non-English characters. 
	*/

	std::setlocale(LC_ALL, "");
	std::setlocale(LC_NUMERIC, "C");

	const auto params = cmd_line_params(argc, argv);

	::current_app_type = params.type;

	if (params.help_only) {
		std::cout << get_help_section() << std::endl;
		
		return EXIT_SUCCESS;
	}

	if (params.version_only) {
		std::cout << get_version_section() << std::endl;

		return EXIT_SUCCESS;
	}

	const auto completed_work_result = work(argc, argv);
	LOG_NVPS(completed_work_result);

	{
		auto save_success_logs = [&]() {
			const auto logs = program_log::get_current().get_complete(); 
			augs::save_as_text(get_exit_success_path(), logs); 
		};

		auto save_failure_logs = [&]() {
			const auto logs = program_log::get_current().get_complete(); 
			const auto failure_log_path = get_exit_failure_path();

			augs::save_as_text(failure_log_path, logs);
			augs::open_text_editor(failure_log_path);
			mark_as_controlled_crash();
		};

		switch (completed_work_result) {
			case work_result::SUCCESS: 
				save_success_logs();
				return EXIT_SUCCESS;

			case work_result::FAILURE: {
				save_failure_logs();
				return EXIT_FAILURE;
			}

			case work_result::RELAUNCH: {
				LOG("main: Application requested relaunch.");
				save_success_logs();

				return augs::restart_application(params.exe_path.string(), "");
			}

			case work_result::RELAUNCH_UPGRADED: {
				LOG("main: Application requested relaunch due to a successful upgrade.");
				save_success_logs();

				return augs::restart_application(params.exe_path.string(), "--upgraded-successfully");
			}

			default: 
				break;
		}
	}

	return EXIT_FAILURE;
}

