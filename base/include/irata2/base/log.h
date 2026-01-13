#pragma once

#include <absl/log/log.h>
#include <absl/log/globals.h>
#include <absl/log/initialize.h>

namespace irata2::base {

/**
 * @brief Logging facade for IRATA2.
 *
 * This provides a thin wrapper around absl::log to keep call sites stable
 * if the backend changes in the future. Use the macros below for all logging:
 *
 * - IRATA2_LOG_INFO: Informational messages (default level)
 * - IRATA2_LOG_WARNING: Warning messages
 * - IRATA2_LOG_ERROR: Error messages
 * - IRATA2_LOG_DEBUG: Debug messages (verbose, disabled by default)
 *
 * Example usage:
 *   IRATA2_LOG_INFO << "Simulation started at PC=" << pc;
 *   IRATA2_LOG_ERROR << "Failed to load cartridge: " << path;
 */

// Log level macros that wrap absl::log
#define IRATA2_LOG_INFO LOG(INFO)
#define IRATA2_LOG_WARNING LOG(WARNING)
#define IRATA2_LOG_ERROR LOG(ERROR)
#define IRATA2_LOG_DEBUG VLOG(1)

/**
 * @brief Initialize the logging system.
 *
 * This must be called once at program startup before any logging occurs.
 * It initializes the absl logging backend and sets INFO logs to stderr.
 */
inline void InitializeLogging() {
  absl::InitializeLog();
  absl::SetStderrThreshold(absl::LogSeverityAtLeast::kInfo);
}

/**
 * @brief Set the minimum log level.
 *
 * @param level Minimum severity level (0=INFO, 1=WARNING, 2=ERROR)
 */
inline void SetMinLogLevel(int level) {
  absl::SetMinLogLevel(static_cast<absl::LogSeverityAtLeast>(level));
}

/**
 * @brief Set the verbosity level for VLOG messages.
 *
 * @param verbosity Verbosity level (higher = more verbose, 0 = disabled)
 *
 * Use this to control debug logging. Set to 1 or higher to enable
 * IRATA2_LOG_DEBUG messages.
 */
inline void SetVerbosity(int verbosity) {
  absl::SetGlobalVLogLevel(verbosity);
}

/**
 * @brief Log level enum for user-facing configuration.
 */
enum class LogLevel {
  kInfo = 0,
  kWarning = 1,
  kError = 2,
  kDebug = -1  // Special: enables verbose logging
};

/**
 * @brief Set log level from LogLevel enum.
 *
 * @param level The desired log level
 */
inline void SetLogLevel(LogLevel level) {
  if (level == LogLevel::kDebug) {
    SetMinLogLevel(0);  // Show all levels
    SetVerbosity(1);    // Enable VLOG(1)
  } else {
    SetMinLogLevel(static_cast<int>(level));
    SetVerbosity(0);  // Disable VLOG
  }
}

}  // namespace irata2::base
