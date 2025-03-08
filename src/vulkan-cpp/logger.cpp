#include <vulkan-cpp/logger.hpp>
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace logger {

    std::string g_CurrentPatternForLogs = "Undefined Pattern Specified";
    std::unordered_map<std::string, ref<spdlog::logger>>
      console_log_manager::s_loggers;

    void console_log_manager::initialize_logger_manager(
      const std::string& pattern) {
        g_CurrentPatternForLogs = pattern;

        //! @note Setting up logs for different log stdout's
        //! @note Logs for p_tag is logs specific to the game.
        set_current_logger("renderer");
        create_new_logger("vulkan");
    }

    void console_log_manager::create_new_logger(const std::string& p_tag) {
        s_loggers[p_tag] = spdlog::stdout_color_mt(p_tag);
        s_loggers[p_tag]->set_level(spdlog::level::trace);
        s_loggers[p_tag]->set_pattern(g_CurrentPatternForLogs);
    }

    void console_log_manager::set_current_logger(const std::string& p_tag) {
        fmt::println("current p_tag = {}\n", p_tag);
        //! @note Setting up logs for different log stdout's
        //! @note Logs for p_tag is logs specific to the game
        s_loggers[p_tag] = spdlog::stdout_color_mt(p_tag);
        s_loggers[p_tag]->set_level(spdlog::level::trace);
        s_loggers[p_tag]->set_pattern(g_CurrentPatternForLogs);
    }

    ref<spdlog::logger> console_log_manager::get(const std::string& p_tag) {
        return s_loggers[p_tag];
    }
};