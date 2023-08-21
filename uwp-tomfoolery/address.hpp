#pragma once

namespace address {
	constexpr auto get_scheduler_singleton{ 0x7D15B0 };
	constexpr auto get_global_state{ 0x47F1C0 };
	constexpr auto vm_load{ 0x970FE0 };
	constexpr auto task_spawn{ 0x580270 };
}

namespace offset {
	namespace scheduler {
		constexpr auto fps{ 0x118 };
		constexpr auto whsj_script_context{ 0x138 };
	}

	// TODO: FIGURE THIS SHIT OUT
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
