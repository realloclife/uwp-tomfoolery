#include <iostream>
#include <tuple>
#include <thread>
#include <string>
#include <format>
#include <Windows.h>

#include "console.hpp"
#include "execution.hpp"

auto thread_main(console::handle_tuple context, HMODULE module) noexcept -> void {
	console::set_title("uwp-tomfoolery");

	auto base = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
	auto scheduler = rbx::get_scheduler();

	uintptr_t waiting_hybrid_scripts_job{};
	for (const auto& job : scheduler.get_jobs()) {
		if (job.get_name() == "WaitingHybridScriptsJob") {
			waiting_hybrid_scripts_job = memory::read<uintptr_t>(job.address);
		}
		console::write(job.get_name());
	}

	std::cout << std::endl;

	auto script_context = memory::read<uintptr_t>(waiting_hybrid_scripts_job + offset::scheduler::whsj_script_context);
	auto lua_state = rbx::get_lua_state(script_context);

	console::write(std::format("Roblox base: {:#X}", base), console::message_type::info);
	console::write(std::format("TaskScheduler: {:#X}", scheduler.address), console::message_type::info);
	console::write(std::format("WaitingHybridScriptsJob: {:#X}", waiting_hybrid_scripts_job), console::message_type::info);
	console::write(std::format("ScriptContext: {:#X}", script_context), console::message_type::info);
	console::write(std::format("lua_State: {:#X}", lua_state), console::message_type::info);

	while (true) {
		std::string input{};
		input.clear();
		std::cout << "> ";
		std::getline(std::cin, input);
		if (input == "exit") {
			console::free(context);
			FreeLibraryAndExitThread(module, 0);
			return;
		}
		exec::execute_script(lua_state, input);
	}
}

auto DllMain(HMODULE module_handle, DWORD call_reason, LPVOID) -> BOOL APIENTRY {
	// DisableThreadLibraryCalls(module_handle);
	if (call_reason != DLL_PROCESS_ATTACH) {
		return TRUE;
	}
	std::thread{ thread_main, console::alloc(), module_handle }.detach();
	return TRUE;
}