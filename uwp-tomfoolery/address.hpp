#pragma once

#include <string_view>

#include "memory.hpp"

using namespace std::literals;

namespace address {
	extern uintptr_t get_scheduler_singleton;
	extern uintptr_t get_global_state;
	extern uintptr_t vm_load;
	extern uintptr_t task_defer;
}

namespace signature {
	constexpr auto get_scheduler_singleton{ "E8 ? ? ? ? 8B 00 90 33 C9"sv };
	constexpr auto get_global_state{ "E8 ? ? ? ? 8B F0 8D 8D A8 FD FF FF"sv };
	constexpr auto vm_load{ "8B C8 6A 00 83 C2 10 68 ? ? ? ? E8 ? ? ? ?"sv };
	constexpr auto task_defer{ "55 8B EC 6A FF 68 ? ? ? ? 64 A1 00 00 00 00 50 83 EC ? A1 ? ? ? ? 33 C5 89 45 EC 53 56 57 50 8D 45 F4 64 A3 00 00 00 00 8B 7D ? C7 45 E8"sv };

	static auto register_signatures() noexcept -> void {
		{
			auto sig = memory::signature_scan(get_scheduler_singleton);
			auto offset = memory::read<uint32_t>(sig + 1);
			address::get_scheduler_singleton = sig + 5 + offset;
		}

		{
			auto sig = memory::signature_scan(get_global_state);
			auto offset = memory::read<uint32_t>(sig + 1);
			address::get_global_state = sig + 5 + offset;
		}

		{
			auto sig = memory::signature_scan(vm_load);
			auto offset = memory::read<uint32_t>(sig + 13);
			address::vm_load = sig + 17 + offset;
		}

		{
			address::task_defer = memory::signature_scan(task_defer);
		}
	}
}

namespace offset {
	namespace scheduler {
		constexpr auto fps{ 0x118 };
		constexpr auto whsj_script_context{ 0x138 };
	}

	namespace state {
		constexpr auto thread{ 0x48 };
		constexpr auto identity{ 0x18 };
		constexpr auto stack_top{ 0x14 };
		constexpr auto stack_base{ 0x8 };
	}

	namespace job {
		constexpr auto start{ 0x134 };
		constexpr auto end{ 0x138 };
	}

	namespace instance {
		constexpr auto name{ 0x2C };
		constexpr auto class_name{ 0xC };
		constexpr auto character{ 0x8C };
		constexpr auto children{ 0x30 };
		constexpr auto parent{ 0x38 };
	}
}
