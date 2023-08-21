#pragma once

#include <iostream>
#include <tuple>
#include <Windows.h>

namespace console {
	using handle_tuple = std::tuple<FILE*, FILE*, FILE*>;

	enum class message_type {
		message,
		info,
		error,
	};

	inline auto set_title(std::string_view title) noexcept -> void {
		SetConsoleTitle(title.data());
	}

	static auto write(std::string_view message, message_type type = message_type::message) noexcept -> void {
		auto stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi{};
		GetConsoleScreenBufferInfo(stdout_handle, &csbi);
		auto original{ csbi.wAttributes };

		std::cout << '[';
		switch (type) {
		case message_type::message:
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
			std::cout << '#';
			break;
		case message_type::error:
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_RED | FOREGROUND_INTENSITY);
			std::cout << '!';
			break;
		case message_type::info:
			SetConsoleTextAttribute(stdout_handle, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
			std::cout << '*';
			break;
		}
		SetConsoleTextAttribute(stdout_handle, original);
		std::cout << ']';
		std::cout << ' ';
		std::cout << message << std::endl;
	}

	[[nodiscard]] static auto alloc() noexcept -> handle_tuple {
		AllocConsole();
		auto window = GetConsoleWindow();
		SetWindowPos(window, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		auto menu = GetSystemMenu(window, FALSE);
		RemoveMenu(menu, SC_MAXIMIZE, MF_BYCOMMAND);
		RemoveMenu(menu, SC_SIZE, MF_BYCOMMAND);
		DrawMenuBar(window);
		FILE* stdin_file;
		FILE* stdout_file;
		FILE* stderr_file;
		freopen_s(&stdin_file, "CONIN$", "r", stdin);
		freopen_s(&stdout_file, "CONOUT$", "w", stdout);
		freopen_s(&stderr_file, "CONOUT$", "w", stderr);
		return std::make_tuple(stdin_file, stdout_file, stderr_file);
	}

	static auto free(handle_tuple handles) noexcept -> void {
		auto [in, out, err] = handles;
		fclose(in);
		fclose(out);
		fclose(err);
		FreeConsole();
	}
}
