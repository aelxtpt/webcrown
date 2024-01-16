#pragma once
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#define SPDLOG_FMT_EXTERNAL

#include <memory>
#include <vector>
#include <spdlog/logger.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace webcrown {

static const std::string logger_name = "webcrown_logger";

inline
std::shared_ptr<spdlog::logger>
setup_logger(std::vector<spdlog::sink_ptr> const& sinks)
{
    auto logger = spdlog::get(logger_name);
    if(not logger)
    {
        if(sinks.size() > 0)
        {
            logger = std::make_shared<spdlog::logger>(logger_name,
                                                      std::begin(sinks),
                                                      std::end(sinks));

            spdlog::register_logger(logger);
        }
        else
        {
            logger = spdlog::stdout_color_mt(logger_name);
        }
    }

    return logger;
}

}
