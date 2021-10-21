#define WEBCROWN_LOG_DEBUG 1

#ifdef WEBCROWN_LOG_DEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG // All DEBUG/TRACE statements will be removed by the pre-processor
#include <spdlog/spdlog.h>
#endif
