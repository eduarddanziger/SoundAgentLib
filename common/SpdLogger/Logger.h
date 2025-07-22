#pragma once

#include <spdlog/spdlog.h>
#include <filesystem>

#include "ClassDefHelper.h"

#include <string>

#include "LogBuffer.h"

#include <spdlog/sinks/dist_sink.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <utility>

// ReSharper disable CppClangTidyClangDiagnosticPragmaMessages
#ifndef RESOURCE_FILENAME_ATTRIBUTE
#define RESOURCE_FILENAME_ATTRIBUTE "UnknownAppFilename"
#pragma message( "RESOURCE_FILENAME_ATTRIBUTE was not defined while including " __FILE__ ". We defined and set it to " RESOURCE_FILENAME_ATTRIBUTE ".")
#else
#pragma message( "RESOURCE_FILENAME_ATTRIBUTE is " RESOURCE_FILENAME_ATTRIBUTE " while including " __FILE__ ".")
#endif

#ifndef ASSEMBLY_VERSION_ATTRIBUTE
#define ASSEMBLY_VERSION_ATTRIBUTE "UnknownAppVersion"
#pragma message( "ASSEMBLY_VERSION_ATTRIBUTE was not defined while including " __FILE__ ". We defined and set it to " ASSEMBLY_VERSION_ATTRIBUTE ".")
#else
#pragma message( "ASSEMBLY_VERSION_ATTRIBUTE is " ASSEMBLY_VERSION_ATTRIBUTE " while including " __FILE__ ".")
#endif
// ReSharper restore CppClangTidyClangDiagnosticPragmaMessages


namespace ed::model
{
	class Logger final
	{
	public:
		DISALLOW_COPY_MOVE(Logger);
	private:
		Logger();
	public:
		~Logger() = default;

		static Logger& Inst();

		Logger& ConfigureAppNameAndVersion(const std::string& appName, const std::string& appVersion);

		Logger& SetPathName(const std::filesystem::path& fileName);
		[[nodiscard]] std::filesystem::path GetPathName() const;
		[[nodiscard]] std::wstring GetDir() const;

		Logger& SetOutputToConsole(bool isOutputToConsole);
		[[nodiscard]] bool IsOutputToConsole() const { return isOutputToConsole_; }

		Logger& SetDelimiterBetweenDateAndTime(const std::string& delimiterBetweenDateAndTime = " ");
		[[nodiscard]] std::string GetDelimiterBetweenDateAndTime() const;

		void SetLogBuffer(std::shared_ptr<LogBuffer> logBuffer);
		[[nodiscard]] bool IsLogBufferSet() const { return spLogBuffer_ != nullptr; }

		void Free();
	private:
		void Reinit();
	private:
		std::shared_ptr<LogBuffer> spLogBuffer_;
		std::filesystem::path pathName_;
		bool isOutputToConsole_ = false;
		std::string delimiterBetweenDateAndTime_ = " ";
		std::shared_ptr<spdlog::details::thread_pool> threadPoolSmartPtr_;
		std::string appName_;
		std::string appVersion_;
	};
}


inline ed::model::Logger::Logger():
	appName_(RESOURCE_FILENAME_ATTRIBUTE),
	appVersion_(ASSEMBLY_VERSION_ATTRIBUTE)
{
}

inline ed::model::Logger& ed::model::Logger::Inst()
{
    static Logger logger;
    return logger;
}

inline ed::model::Logger& ed::model::Logger::ConfigureAppNameAndVersion(const std::string& appName,
    const std::string& appVersion)
{
    appName_ = appName;
    appVersion_ = appVersion;
    Reinit();
    return *this;
}

inline ed::model::Logger& ed::model::Logger::SetPathName(const std::filesystem::path& fileName)
{
    pathName_ = fileName;
	Reinit();
	return *this;
}

inline std::filesystem::path ed::model::Logger::GetPathName() const
{
	return pathName_;
}

inline std::wstring ed::model::Logger::GetDir() const
{
	const auto* pathNamePtr = pathName_.c_str();
	if
	(
		const wchar_t* found;
		(found = wcsrchr(pathNamePtr, L'\\')) != nullptr ||
		(found = wcsrchr(pathNamePtr, L'/')) != nullptr
	)
	{
		return {pathNamePtr, found};
	}

	return pathName_;
}

inline ed::model::Logger& ed::model::Logger::SetDelimiterBetweenDateAndTime(
	const std::string& delimiterBetweenDateAndTime)
{
    if (delimiterBetweenDateAndTime_ != delimiterBetweenDateAndTime)
    {
        delimiterBetweenDateAndTime_ = delimiterBetweenDateAndTime;
		Reinit();
	}
	return *this;
}

inline std::string ed::model::Logger::GetDelimiterBetweenDateAndTime() const
{
    return delimiterBetweenDateAndTime_;
}

inline void ed::model::Logger::SetLogBuffer(std::shared_ptr<LogBuffer> logBuffer)
{
    spLogBuffer_ = std::move(logBuffer);
	Reinit();
}

inline ed::model::Logger& ed::model::Logger::SetOutputToConsole(bool isOutputToConsole)
{
    if (isOutputToConsole_ != isOutputToConsole)
    {
        isOutputToConsole_ = isOutputToConsole;
		Reinit();
	}
	return *this;
}


inline void ed::model::Logger::Reinit()
{
	auto finalMessage = std::string();
	spdlog::shutdown();

	auto distributedSink = std::make_shared<spdlog::sinks::dist_sink_st>();
	if (!pathName_.empty())
	{
		const auto rotatingFileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			pathName_.string(), 1024 * 1024, 10);
		distributedSink->add_sink(rotatingFileSink);
		finalMessage += "Output to file ";
		finalMessage += pathName_.string();
	}
	else
	{
		finalMessage += "No output to file";
	}


	finalMessage += ", output to terminal (console)";
	if (isOutputToConsole_)
	{
		const auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		distributedSink->add_sink(consoleSink);
		finalMessage += " enabled";
	}
	else
	{
		finalMessage += " disabled";
	}

	finalMessage += ", output to log buffer";
	if (spLogBuffer_ != nullptr)
	{
		distributedSink->add_sink(spLogBuffer_);
		finalMessage += " enabled";
	}
	else
	{
		finalMessage += " disabled";
	}

	threadPoolSmartPtr_ = std::make_shared<
		spdlog::details::thread_pool>(65536, 2);

	// Create an async_logger using that custom thread pool
	const auto spdLogger = std::make_shared<spdlog::async_logger>(
		RESOURCE_FILENAME_ATTRIBUTE,
		distributedSink,
		threadPoolSmartPtr_,
		spdlog::async_overflow_policy::block
	);

	spdlog::register_logger(spdLogger);
	spdlog::set_default_logger(spdLogger);

	spdlog::set_pattern(std::string("%Y-%m-%d") + delimiterBetweenDateAndTime_ + "%H:%M:%S.%f %L [%t] %v");
	spdlog::set_level(spdlog::level::debug);
	spdlog::info("Application {}, version {}, logging [re-]initiated: {}", appName_, appVersion_, finalMessage);
}

inline void ed::model::Logger::Free()
{
	if (threadPoolSmartPtr_ == nullptr)
	{
		return;
	}
	spdlog::info("Log for {}, version {} is being freed.", RESOURCE_FILENAME_ATTRIBUTE, ASSEMBLY_VERSION_ATTRIBUTE);
	spdlog::shutdown();
	threadPoolSmartPtr_.reset(); // Explicitly reset the thread pool to free resources
}
