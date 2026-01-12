#include "irata2/base/log.h"

#include <gtest/gtest.h>

using namespace irata2::base;

TEST(LogTest, Initialization) {
  // Initialize should not crash
  EXPECT_NO_THROW(InitializeLogging());
}

TEST(LogTest, SetMinLogLevel) {
  // Setting log level should not crash
  EXPECT_NO_THROW(SetMinLogLevel(0));  // INFO
  EXPECT_NO_THROW(SetMinLogLevel(1));  // WARNING
  EXPECT_NO_THROW(SetMinLogLevel(2));  // ERROR
}

TEST(LogTest, SetVerbosity) {
  // Setting verbosity should not crash
  EXPECT_NO_THROW(SetVerbosity(0));
  EXPECT_NO_THROW(SetVerbosity(1));
  EXPECT_NO_THROW(SetVerbosity(2));
}

TEST(LogTest, SetLogLevelEnum) {
  // Setting log level with enum should not crash
  EXPECT_NO_THROW(SetLogLevel(LogLevel::kInfo));
  EXPECT_NO_THROW(SetLogLevel(LogLevel::kWarning));
  EXPECT_NO_THROW(SetLogLevel(LogLevel::kError));
  EXPECT_NO_THROW(SetLogLevel(LogLevel::kDebug));
}

TEST(LogTest, LogMacrosCompile) {
  // These tests verify that the macros compile and can be used
  // We don't verify output since that would require capturing stderr
  IRATA2_LOG_INFO << "Test info message";
  IRATA2_LOG_WARNING << "Test warning message";
  IRATA2_LOG_ERROR << "Test error message";
  IRATA2_LOG_DEBUG << "Test debug message";
}

TEST(LogTest, LogWithFormatting) {
  // Verify that stream operators work correctly
  int value = 42;
  std::string str = "test";

  IRATA2_LOG_INFO << "Value: " << value << ", String: " << str;
}
