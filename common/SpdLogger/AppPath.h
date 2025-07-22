#pragma once

#include <Windows.h>

#include <filesystem>
#include <sstream>

#include <ShlObj.h>

namespace fs = std::filesystem;


namespace std::filesystem  // NOLINT(cert-dcl58-cpp)
{
	class path;
}

namespace ed::utility
{
	class AppPath
	{
	public:
		static bool GetAndValidateLogFileInProgramData(std::filesystem::path& logFile, const std::string& appFileNameWoExt);
	protected:
		static void GetOwnProgramDataPath(std::filesystem::path& ownDataPath, const std::string& appFileNameWoExt);
	};
}

inline bool ed::utility::AppPath::GetAndValidateLogFileInProgramData(std::filesystem::path& logFile, const std::string& appFileNameWoExt)
{
    std::filesystem::path ownDataPath;
    GetOwnProgramDataPath(ownDataPath, appFileNameWoExt);

    auto logFileNameWoExt(appFileNameWoExt); // replace . via _
    while (logFileNameWoExt.find('.') != std::string::npos)
    {
        logFileNameWoExt.replace(logFileNameWoExt.find('.'), 1, "_");
    }
    if (exists(ownDataPath) || create_directories(ownDataPath))
    {
        fs::path logFilePathName;
        auto numberToIncrement = -1;
        do
        {
            if (++numberToIncrement > 99999)
            {
                return false;
            }
            std::ostringstream ossForFileName;
            ossForFileName << logFileNameWoExt << std::setfill('0') << std::setw(5) << numberToIncrement;
            logFilePathName = ownDataPath / ossForFileName.str();
            logFilePathName.replace_extension(".log");
        } while (std::filesystem::exists(logFilePathName));

        logFile.swap(logFilePathName);
        return true;
    }
    return false;
}

inline void ed::utility::AppPath::GetOwnProgramDataPath(std::filesystem::path& ownDataPath,
	const std::string& appFileNameWoExt)
{
    LPWSTR pProgramDataPath;
    std::wstring wideProgramDataPath;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &pProgramDataPath)))
    {
        wideProgramDataPath = pProgramDataPath;
        CoTaskMemFree(pProgramDataPath);
    }

    ownDataPath = fs::path(wideProgramDataPath) / appFileNameWoExt;
}

