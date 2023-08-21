#pragma once

#include "memory.hpp"
#include "address.hpp"

#include <cstdint>
#include <vector>

namespace rbx {
	class job {
	public:
		uintptr_t address;

		inline auto get_name() const noexcept -> std::string {
			return memory::read_string(memory::read<uintptr_t>(this->address) + 0x10);
		}
	};

	class scheduler {
	public:
		uintptr_t address;

		auto get_jobs() const noexcept -> std::vector<job>;

		inline auto set_fps(double new_fps) noexcept -> void {
			memory::write<double>(this->address + offset::scheduler::fps, 1 / new_fps);
		}
		inline auto get_fps() const noexcept -> double {
			return 1 / memory::read<double>(this->address + offset::scheduler::fps);
		}
	};

}
