#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <sstream>
#include <string>
#include <chrono>
#include <mutex>
#include <thread>
#include <iomanip>

#define LOG_INFO(...) \
  Logger::instance().log(eLogLevel::Info, __VA_ARGS__)

#define LOG_ERROR(...) \
  Logger::instance().log(eLogLevel::Error, __VA_ARGS__)

enum class eLogLevel
{
  Info,
  Warning,
  Error,
  Debug
};

void set_thread_name(const std::string &name);
const std::string &get_thread_name();

class Logger
{
public:
  static Logger &instance()
  {
    static Logger instance;
    return instance;
  }

  template <typename... Args>
  void log(eLogLevel level, Args &&...args)
  {
    std::lock_guard<std::mutex> lock(m_log_mutex);

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm local_time{};
#ifdef _WIN32
    localtime_s(&local_time, &time);  // Windows: parameters reversed
#else
    localtime_r(&time, &local_time);  // POSIX (Linux/Unix)
#endif

    std::ostringstream timestamp_stream;
    timestamp_stream << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
            << '.' << std::setfill('0') << std::setw(3) << ms.count();

    std::ostringstream message_stream;
    (message_stream << ... << args); // C++17 fold expression

    m_out_stream << "[" << timestamp_stream.str() << "] "
                 << "[" << log_level_to_string(level) << "] "
                 << "[Thread: " << get_thread_name() << "] "
                 << message_stream.str() << "\n";
  }

private:
  std::ostream &m_out_stream = std::cout;
  std::mutex m_log_mutex;

  Logger() = default;
  ~Logger() = default;
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;

  std::string log_level_to_string(eLogLevel level)
  {
    switch (level)
    {
    case eLogLevel::Info:
      return "INFO";
    case eLogLevel::Warning:
      return "WARNING";
    case eLogLevel::Error:
      return "ERROR";
    case eLogLevel::Debug:
      return "DEBUG";
    default:
      return "UNKNOWN";
    }
  }
};

#endif // __LOGGER_H__