#pragma once

#include "common/ClassDefHelper.h"

#include <chrono>
#include <string>
#include <unordered_map>

class HttpRequestProcessorInterface
{
public:
    virtual void EnqueueRequest(
        bool postOrPut,
        const std::chrono::system_clock::time_point& time,
        const std::string& urlSuffix,
        const std::string& payload,
        const std::unordered_map<std::string, std::string>& header,
        const std::string& hint
    ) = 0;
    AS_INTERFACE(HttpRequestProcessorInterface);
    DISALLOW_COPY_MOVE(HttpRequestProcessorInterface);
};
