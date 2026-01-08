#include "irata2/base/types.h"

#include <gtest/gtest.h>

using namespace irata2::base;

TEST(WordTest, DefaultConstruction) {
  Word w;
  EXPECT_EQ(w.value(), 0x0000);
}

TEST(WordTest, ValueConstruction) {
  Word w(0x1234);
  EXPECT_EQ(w.value(), 0x1234);
}

TEST(WordTest, ByteConstruction) {
  Byte high(0x12);
  Byte low(0x34);
  Word w(high, low);
  EXPECT_EQ(w.value(), 0x1234);
}

TEST(WordTest, HighByte) {
  Word w(0x1234);
  EXPECT_EQ(w.high().value(), 0x12);
}

TEST(WordTest, LowByte) {
  Word w(0x1234);
  EXPECT_EQ(w.low().value(), 0x34);
}

TEST(WordTest, Addition) {
  Word a(0x1000);
  Word b(0x2000);
  Word c = a + b;
  EXPECT_EQ(c.value(), 0x3000);
}

TEST(WordTest, AdditionOverflow) {
  Word a(0xFFFF);
  Word b(0x0001);
  Word c = a + b;
  EXPECT_EQ(c.value(), 0x0000);  // Wraps around
}

TEST(WordTest, Subtraction) {
  Word a(0x3000);
  Word b(0x1000);
  Word c = a - b;
  EXPECT_EQ(c.value(), 0x2000);
}

TEST(WordTest, SubtractionUnderflow) {
  Word a(0x0000);
  Word b(0x0001);
  Word c = a - b;
  EXPECT_EQ(c.value(), 0xFFFF);  // Wraps around
}

TEST(WordTest, BitwiseAnd) {
  Word a(0xFF00);
  Word b(0xAAAA);
  Word c = a & b;
  EXPECT_EQ(c.value(), 0xAA00);
}

TEST(WordTest, BitwiseOr) {
  Word a(0xFF00);
  Word b(0x00AA);
  Word c = a | b;
  EXPECT_EQ(c.value(), 0xFFAA);
}

TEST(WordTest, BitwiseXor) {
  Word a(0xFFFF);
  Word b(0xAAAA);
  Word c = a ^ b;
  EXPECT_EQ(c.value(), 0x5555);
}

TEST(WordTest, BitwiseNot) {
  Word a(0xFF00);
  Word b = ~a;
  EXPECT_EQ(b.value(), 0x00FF);
}

TEST(WordTest, LeftShift) {
  Word a(0x0003);
  Word b = a << 4;
  EXPECT_EQ(b.value(), 0x0030);
}

TEST(WordTest, RightShift) {
  Word a(0x0030);
  Word b = a >> 4;
  EXPECT_EQ(b.value(), 0x0003);
}

TEST(WordTest, Equality) {
  Word a(0x1234);
  Word b(0x1234);
  Word c(0x1235);

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
}

TEST(WordTest, Inequality) {
  Word a(0x1234);
  Word b(0x1235);

  EXPECT_TRUE(a != b);
  EXPECT_FALSE(a != a);
}

TEST(WordTest, LessThan) {
  Word a(0x1000);
  Word b(0x2000);

  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a < a);
}

TEST(WordTest, LessThanOrEqual) {
  Word a(0x1000);
  Word b(0x2000);

  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a <= a);
  EXPECT_FALSE(b <= a);
}

TEST(WordTest, GreaterThan) {
  Word a(0x1000);
  Word b(0x2000);

  EXPECT_TRUE(b > a);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a > a);
}

TEST(WordTest, GreaterThanOrEqual) {
  Word a(0x1000);
  Word b(0x2000);

  EXPECT_TRUE(b >= a);
  EXPECT_TRUE(a >= a);
  EXPECT_FALSE(a >= b);
}

TEST(WordTest, ToString) {
  Word a(0x1234);
  EXPECT_EQ(a.to_string(), "0x1234");

  Word b(0x00AB);
  EXPECT_EQ(b.to_string(), "0x00AB");

  Word c(0xFFFF);
  EXPECT_EQ(c.to_string(), "0xFFFF");
}
