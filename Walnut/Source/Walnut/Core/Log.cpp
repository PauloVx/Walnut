#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

#include <filesystem>

#define WL_HAS_CONSOLE !WL_DIST

namespace Walnut {

	std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
	std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

	void Log::Init(const std::filesystem::path& logDirectory)
	{
		std::vector<spdlog::sink_ptr> hazelSinks;
		std::vector<spdlog::sink_ptr> appSinks;

		if (!logDirectory.empty())
		{
			std::filesystem::create_directories(logDirectory);
			hazelSinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "WALNUT.log").string(), true));
			appSinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>((logDirectory / "APP.log").string(), true));
			hazelSinks.back()->set_pattern("[%T] [%l] %n: %v");
			appSinks.back()->set_pattern("[%T] [%l] %n: %v");
		}

#if WL_HAS_CONSOLE
		hazelSinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		appSinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
		hazelSinks.back()->set_pattern("%^[%T] %n: %v%$");
		appSinks.back()->set_pattern("%^[%T] %n: %v%$");
#endif

		s_CoreLogger = std::make_shared<spdlog::logger>("WALNUT", hazelSinks.begin(), hazelSinks.end());
		s_CoreLogger->set_level(spdlog::level::trace);

		s_ClientLogger = std::make_shared<spdlog::logger>("APP", appSinks.begin(), appSinks.end());
		s_ClientLogger->set_level(spdlog::level::trace);
	}

	void Log::Shutdown()
	{
		s_ClientLogger.reset();
		s_CoreLogger.reset();
		spdlog::drop_all();
	}

}
