#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/word_bus.h"

#include <gtest/gtest.h>

#include <type_traits>

using namespace irata2::hdl;

TEST(BusTest, WidthMatchesValueType) {
  EXPECT_EQ(ByteBus::kWidth, 8u);
  EXPECT_EQ(WordBus::kWidth, 16u);
}

TEST(BusTest, ValueTypeAliases) {
  EXPECT_TRUE((std::is_same_v<ByteBus::value_type, irata2::base::Byte>));
  EXPECT_TRUE((std::is_same_v<WordBus::value_type, irata2::base::Word>));
}
