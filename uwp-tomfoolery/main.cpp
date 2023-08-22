#include <iostream>
#include <tuple>
#include <thread>
#include <string>
#include <string_view>
#include <format>
#include <Windows.h>

#include "console.hpp"
#include "execution.hpp"

using namespace std::literals;

auto thread_main(console::handle_tuple context, HMODULE module) noexcept -> void {
	console::set_title("uwp-tomfoolery");
	signature::register_signatures();

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
	console::write(std::format("get_scheduler_singleton: {:#X}", address::get_scheduler_singleton), console::message_type::info);
	console::write(std::format("get_global_state: {:#X}", address::get_global_state), console::message_type::info);
	console::write(std::format("vm_load: {:#X}", address::vm_load), console::message_type::info);
	console::write(std::format("task_defer: {:#X}", address::task_defer), console::message_type::info);
	console::write(std::format("TaskScheduler: {:#X}", scheduler.address), console::message_type::info);
	console::write(std::format("WaitingHybridScriptsJob: {:#X}", waiting_hybrid_scripts_job), console::message_type::info);
	console::write(std::format("ScriptContext: {:#X}", script_context), console::message_type::info);
	console::write(std::format("lua_State: {:#X}", lua_state), console::message_type::info);

	std::cout << std::endl;

	// auto whsj_vmt = memory::read<uintptr_t>(waiting_hybrid_scripts_job);
	// exec::whsj_step_original = memory::vmt_hook(whsj_vmt, 5, reinterpret_cast<uintptr_t>(exec::whsj_step_hook));
	// console::write(std::format("Hooked WaitingHybridScriptsJob.Stepped for script scheduling"), console::message_type::info);

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