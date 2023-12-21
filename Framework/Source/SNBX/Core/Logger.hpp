#pragma once

#include <spdlog/spdlog.h>

namespace SNBX::Log
{

	template<typename... Args>
	inline void Trace(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->trace(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void Debug(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->debug(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void Info(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->info(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void Warn(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->warn(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void Error(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->error(fmt, std::forward<Args>(args)...);
	}

	template<typename... Args>
	inline void Critical(spdlog::format_string_t<Args...> fmt, Args&& ...args)
	{
		spdlog::default_logger_raw()->critical(fmt, std::forward<Args>(args)...);
	}

}