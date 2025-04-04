#pragma once
#include <fmt/os.h>
#include <fmt/ostream.h>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace logger {
    //! @note Some aliases for the smart pointers
    template<typename T>
    using ref = std::shared_ptr<T>;

    template<typename T, typename... Args>
    constexpr ref<T> create_ref(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using scope = std::unique_ptr<T>;

    template<typename T, typename... Args>
    constexpr scope<T> create_scope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T>
    using weak_ptr = std::weak_ptr<T>;

    template<typename T, typename... Args>
    constexpr weak_ptr<T> create_weak_ptr(Args&&... args) {
        return std::weak_ptr<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename U>
    weak_ptr<T> to_weak_ptr(const U& p_value) {
        return CreateWeakPtr<T>(p_value);
    }
    /**
     * @name console_log_manager
     * @note renderer's console logger to write to the console instead of
     * specifying the UI-type of widget to upload logging information to
     * @note TODO --- Adding a way to indicate to the logger to supply a type of
     * ui identifier
     * @note Reason - is if users wanted have a way of knowing to t
     */
    class console_log_manager {
    public:
        //! @note Used for initiating this console logger across renderer
        //! supplying the pattern and application to dedicate log messages to
        static void initialize_logger_manager(
          const std::string& pattern = "%^[%T] %n: %v%$");

        static void set_current_logger(
          const std::string& p_tag = "Undefined g_Tag in console_logger");
        static void create_new_logger(
          const std::string& p_tag = "Undefined Tag");
        static ref<spdlog::logger> get(const std::string& p_tag);

    private:
        //! @note std::string is the tag of the log's location.
        static std::unordered_map<std::string, ref<spdlog::logger>> s_loggers;
    };
};

//! @note Console Loggers (These are loggers that write specifically to the
//! console, terminal console)
//! @note TODO --- Specify that renderer will have it's own console terminal
//! that these will be written to.
template<typename... T>
inline void
console_log_trace(spdlog::format_string_t<T...> fmt, T&&... args) {
    logger::console_log_manager::get("renderer")
      ->trace(fmt, std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_warn(spdlog::format_string_t<T...> fmt, T&&... args) {
    logger::console_log_manager::get("renderer")
      ->warn(fmt, std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_info(spdlog::format_string_t<T...> fmt, T&&... args) {
    logger::console_log_manager::get("renderer")
      ->info(fmt, std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_error(spdlog::format_string_t<T...> fmt, T&&... args) {
    logger::console_log_manager::get("renderer")
      ->error(fmt, std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_fatal(spdlog::format_string_t<T...> fmt, T&&... args) {
    logger::console_log_manager::get("renderer")
      ->critical(fmt, std::forward<T>(args)...);
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
template<typename... T>
inline void
console_log_trace_tagged(const std::string& p_tag,
                         spdlog::format_string_t<T...> fmt,
                         T&&... args) {
    logger::console_log_manager::get(p_tag)->trace(fmt,
                                                   std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_info_tagged(const std::string& p_tag,
                        spdlog::format_string_t<T...> fmt,
                        T&&... args) {
    logger::console_log_manager::get(p_tag)->info(fmt,
                                                  std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_warn_tagged(const std::string& p_tag,
                        spdlog::format_string_t<T...> fmt,
                        T&&... args) {
    logger::console_log_manager::get(p_tag)->warn(fmt,
                                                  std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_error_tagged(const std::string& p_tag,
                         spdlog::format_string_t<T...> fmt,
                         T&&... args) {
    logger::console_log_manager::get(p_tag)->error(fmt,
                                                   std::forward<T>(args)...);
}

template<typename... T>
inline void
console_log_fatal_tagged(const std::string& p_tag,
                         spdlog::format_string_t<T...> fmt,
                         T&&... args) {
    logger::console_log_manager::get(p_tag)->critical(fmt,
                                                      std::forward<T>(args)...);
}