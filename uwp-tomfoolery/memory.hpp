#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <vector>
#include <optional>
#include <Windows.h>

namespace memory {
	template<typename T>
	concept small_type = sizeof(T) <= sizeof(uintptr_t);

	inline auto rebase(uintptr_t address) noexcept -> uintptr_t {
		static auto module_base = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
		return module_base + address;
	}

	template<typename T>
	inline auto read(uintptr_t address) noexcept -> T {
		return *reinterpret_cast<T*>(address);
	}

	template<typename T> requires(!small_type<T>)
	inline auto write(uintptr_t address, T const& value) noexcept -> void {
		*reinterpret_cast<T*>(address) = value;
	}

	template<typename T> requires(small_type<T>)
	inline auto write(uintptr_t address, T value) noexcept -> void {
		*reinterpret_cast<T*>(address) = value;
	}

	static auto read_string(uintptr_t address) noexcept -> std::string {
		auto count = memory::read<size_t>(address + 0x10);
		if (count >= 16) { // small string optimization does not happen for larger strings
			address = memory::read<uintptr_t>(address);
		}
		std::string buffer{};
		for (auto i = 0; i < count; i++) {
			buffer.push_back(memory::read<char>(address + i));
		}
		return buffer;
	}

	static auto detour(uintptr_t original_func, uintptr_t jump_target, size_t length) noexcept -> void {
		DWORD old_protection{};
		DWORD dummy_{};
		VirtualProtect(reinterpret_cast<LPVOID>(original_func), length, PAGE_EXECUTE_READWRITE, &old_protection);
		auto relative_target = jump_target - original_func - 5;
		*reinterpret_cast<uint8_t*>(original_func) = 0xE9; // jmp
		*reinterpret_cast<uintptr_t*>(original_func + 1) = relative_target;
		for (auto offset = 5; offset < length; offset++) {
			*reinterpret_cast<uint8_t*>(original_func + offset) = 0x90; // nop
		}
		VirtualProtect(reinterpret_cast<LPVOID>(original_func), length, old_protection, &dummy_);
	}

	static auto vmt_hook(uintptr_t vmt, size_t index, uintptr_t hook) noexcept -> uintptr_t {
		auto func = *reinterpret_cast<uintptr_t*>(vmt + index);
		*reinterpret_cast<uintptr_t*>(vmt + index) = hook;
		return func;
	}

	static auto signature_scan(std::string_view pattern, std::optional<std::string_view> module_name = std::nullopt) noexcept -> uintptr_t {
		static auto handle = GetModuleHandle(module_name.has_value() ? module_name->data() : nullptr);
		auto base = reinterpret_cast<uintptr_t>(handle);
		auto dos_header = reinterpret_cast<IMAGE_DOS_HEADER*>(handle);
		if (!dos_header) {
			return 0;
		}
		auto nt_headers = reinterpret_cast<IMAGE_NT_HEADERS*>(base + dos_header->e_lfanew);
		auto optional_header = nt_headers->OptionalHeader;

		uintptr_t start_address = optional_header.BaseOfCode;
		uintptr_t end_address = optional_header.SizeOfCode;

		static auto module_base = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));
		if (end_address < start_address) {
			return 0;
		}

		auto end = module_base + end_address;
		auto start = module_base + start_address;
		std::vector<std::pair<bool, uint8_t>> byte_pattern{};
		for (auto it = pattern.begin(); it < pattern.end(); ++it) {
			switch (*it) {
			case ' ':
				continue;
			case '?':
				byte_pattern.emplace_back(std::make_pair<bool, uint8_t>(true, 0x00));
				continue;
			default:
				std::string cached_byte_string{ it - 1, (++it) + 1 };
				byte_pattern.emplace_back(std::make_pair<bool, uint8_t>(false, static_cast<uint8_t>(std::stoul(cached_byte_string, nullptr, 16))));
			}
		}

		while (start < end) {
			auto found_pattern = true;
			for (auto const& [wildcard, byte] : byte_pattern) {
				++start;
				if (wildcard)
					continue;

				if (*reinterpret_cast<uint8_t*>(start) != byte) {
					found_pattern = false;
					break;
				}
			}
			if (found_pattern) {
				return start - (byte_pattern.size() - 1);
			}
		}
		return 0;
	}
}
