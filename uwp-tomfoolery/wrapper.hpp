#pragma once

#include <string_view>
#include <unordered_map>

#include "memory.hpp"
#include "address.hpp"
#include "scheduler.hpp"

namespace rbx {
	inline auto get_scheduler() noexcept -> scheduler {
		return scheduler{ reinterpret_cast<auto (__cdecl*)()->uintptr_t>(memory::rebase(address::get_scheduler_singleton))() };
	}

	inline auto get_lua_state(uintptr_t script_context) noexcept -> uintptr_t {
		static constexpr uint32_t identity = 8;
		static constexpr uint32_t environment = 0;

		auto get_global_state = reinterpret_cast<auto (__thiscall*)(uintptr_t, uintptr_t, uintptr_t)->uintptr_t>(memory::rebase(address::get_global_state));
		return get_global_state(script_context, reinterpret_cast<uintptr_t>(&identity), reinterpret_cast<uintptr_t>(&environment));
	}

	inline auto set_identity(uintptr_t lua_state, uint32_t identity) noexcept -> void {
		*reinterpret_cast<uint32_t*>(*reinterpret_cast<uintptr_t*>(lua_state + offset::state::thread) + offset::state::identity) = identity;
	}

	inline auto pop_stack(uintptr_t lua_state, uint8_t count) noexcept -> void {
		*reinterpret_cast<uintptr_t*>(lua_state + offset::state::stack_top) -= count * 0x10;
	}

	inline auto vm_load(uintptr_t lua_state, std::string* bytecode) noexcept -> int32_t {
		return reinterpret_cast<auto (__fastcall*)(uintptr_t, std::string*, const char*, uintptr_t)->int32_t>(memory::rebase(address::vm_load))(lua_state, bytecode, "", 0x0);
	}

	inline auto task_spawn(uintptr_t lua_state) noexcept -> int32_t {
		return reinterpret_cast<auto (__cdecl*)(uintptr_t)->int32_t>(memory::rebase(address::task_spawn))(lua_state);
	}

	inline auto task_defer(uintptr_t lua_state) noexcept -> uintptr_t {
		return reinterpret_cast<auto (__cdecl*)(uintptr_t)->uintptr_t>(memory::rebase(address::task_defer))(lua_state);
	}
}
