#pragma once

#include <cstdint>
#include <string>
#include <queue>
#include <mutex>

#include "Luau/Compiler.h"
#include "Luau/BytecodeBuilder.h"
#include "xxhash.h"
#include "zstd.h"

#include "wrapper.hpp"
#include "console.hpp"

namespace exec {
	using whsj_step_type = auto (__thiscall*)(uintptr_t, uintptr_t)->uintptr_t;

	extern std::mutex pending_bytecode_mutex;
	extern std::queue<std::string> pending_bytecode;
	extern uintptr_t whsj_step_original;

	class roblox_encoder : public Luau::BytecodeEncoder {
		inline auto encodeOp(uint8_t opcode) -> uint8_t override {
			return uint8_t(opcode * 0xE3);
		}
	};

	// TODO
	static auto __fastcall whsj_step_hook(uintptr_t whsj, uintptr_t stats_ref) -> uint32_t {
		std::lock_guard<std::mutex> lock{ pending_bytecode_mutex };
		while (!pending_bytecode.empty()) {
			auto code = pending_bytecode.front();
			pending_bytecode.pop();
		}
		if (!whsj_step_original) { // hilarious race condition bandaid fix
			return 0;
		}
		return reinterpret_cast<whsj_step_type>(whsj_step_original)(whsj, stats_ref);
	}

	static auto execute_bytecode(uintptr_t lua_state, std::string& bytecode) -> void {
		// thank you roblox for making this horrible for me :3
		auto max_compressed_size = ZSTD_compressBound(bytecode.size());
		auto* final_destination = new uint8_t[4 + 4 + max_compressed_size];
		std::memcpy(final_destination, "RSB1", 4);
		auto uncompressed_size = static_cast<uint32_t>(bytecode.size());
		std::memcpy(&final_destination[4], &uncompressed_size, 4);
		auto compressed_size = ZSTD_compress(&final_destination[8], max_compressed_size, bytecode.c_str(), bytecode.size(), ZSTD_maxCLevel());
		auto final_size = 4 + 4 + compressed_size;
		auto hash = XXH32(final_destination, final_size, 0x2A);
		uint8_t hash_key[4]{};
		std::memcpy(hash_key, &hash, 4);
		for (auto i = 0; i < final_size; i++) {
			final_destination[i] ^= hash_key[i % 4] + (i * 0x29); // close your eyes and pray it works
		}
		console::write("Compressed and hashed bytecode (ZSTD, XXH32, XOR)", console::message_type::info);

		rbx::set_identity(lua_state, 8); // we like to troll
		console::write("Identitiy was set to 8 for funnies", console::message_type::info);
		std::string compressed{ reinterpret_cast<const char*>(final_destination) };
		std::cout << std::hex;
		for (auto c : compressed) {
			std::cout << +c << "  ";
		}
		std::cout << std::dec;

		rbx::vm_load(lua_state, &compressed);
		console::write("Called vm_load on compressed bytecode", console::message_type::info);
		rbx::task_defer(lua_state);
		console::write("Spawned new proto via lua_State", console::message_type::info);
		rbx::pop_stack(lua_state, 1);
		console::write("Popped proto off the stack", console::message_type::info);
	}

	static auto execute_script(uintptr_t lua_state, std::string& script) -> void {
		static constinit roblox_encoder encoder{};

		// TODO: HOOK WaitingHybridScriptsJob.VMT[5] (Stepped), POP SCHEDULE QUEUE FRONT, LOCK WHEN ADDING NEW LUA PROTO
		auto bytecode = Luau::compile("task.spawn(function()\n" + script + "\nend)", { 0, 2, 0 }, {}, &encoder); // horrendously ugly hack, crashes like all the time
		console::write("Encoded bytecode (* 0xE3, uint8_t wrap-around)", console::message_type::info);
		// pending_bytecode.push(std::move(bytecode));
		if (bytecode[0] == 0) {
			console::write("Error while compiling into bytecode", console::message_type::error);
		} else {
			execute_bytecode(lua_state, bytecode);
		}
	}
}