#include "irata2/base/types.h"

#include <gtest/gtest.h>

using namespace irata2::base;

TEST(ByteTest, DefaultConstruction) {
  Byte b;
  EXPECT_EQ(b.value(), 0x00);
}

TEST(ByteTest, ValueConstruction) {
  Byte b(0x42);
  EXPECT_EQ(b.value(), 0x42);
}

TEST(ByteTest, Addition) {
  Byte a(0x10);
  Byte b(0x20);
  Byte c = a + b;
  EXPECT_EQ(c.value(), 0x30);
}

TEST(ByteTest, AdditionOverflow) {
  Byte a(0xFF);
  Byte b(0x01);
  Byte c = a + b;
  EXPECT_EQ(c.value(), 0x00);  // Wraps around
}

TEST(ByteTest, Subtraction) {
  Byte a(0x30);
  Byte b(0x10);
  Byte c = a - b;
  EXPECT_EQ(c.value(), 0x20);
}

TEST(ByteTest, SubtractionUnderflow) {
  Byte a(0x00);
  Byte b(0x01);
  Byte c = a - b;
  EXPECT_EQ(c.value(), 0xFF);  // Wraps around
}

TEST(ByteTest, BitwiseAnd) {
  Byte a(0b11110000);
  Byte b(0b10101010);
  Byte c = a & b;
  EXPECT_EQ(c.value(), 0b10100000);
}

TEST(ByteTest, BitwiseOr) {
  Byte a(0b11110000);
  Byte b(0b10101010);
  Byte c = a | b;
  EXPECT_EQ(c.value(), 0b11111010);
}

TEST(ByteTest, BitwiseXor) {
  Byte a(0b11110000);
  Byte b(0b10101010);
  Byte c = a ^ b;
  EXPECT_EQ(c.value(), 0b01011010);
}

TEST(ByteTest, BitwiseNot) {
  Byte a(0b11110000);
  Byte b = ~a;
  EXPECT_EQ(b.value(), 0b00001111);
}

TEST(ByteTest, LeftShift) {
  Byte a(0b00000011);
  Byte b = a << 2;
  EXPECT_EQ(b.value(), 0b00001100);
}

TEST(ByteTest, RightShift) {
  Byte a(0b00001100);
  Byte b = a >> 2;
  EXPECT_EQ(b.value(), 0b00000011);
}

TEST(ByteTest, Equality) {
  Byte a(0x42);
  Byte b(0x42);
  Byte c(0x43);

  EXPECT_TRUE(a == b);
  EXPECT_FALSE(a == c);
}

TEST(ByteTest, Inequality) {
  Byte a(0x42);
  Byte b(0x43);

  EXPECT_TRUE(a != b);
  EXPECT_FALSE(a != a);
}

TEST(ByteTest, LessThan) {
  Byte a(0x10);
  Byte b(0x20);

  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a < a);
}

TEST(ByteTest, LessThanOrEqual) {
  Byte a(0x10);
  Byte b(0x20);

  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a <= a);
  EXPECT_FALSE(b <= a);
}

TEST(ByteTest, GreaterThan) {
  Byte a(0x10);
  Byte b(0x20);

  EXPECT_TRUE(b > a);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(a > a);
}

TEST(ByteTest, GreaterThanOrEqual) {
  Byte a(0x10);
  Byte b(0x20);

  EXPECT_TRUE(b >= a);
  EXPECT_TRUE(a >= a);
  EXPECT_FALSE(a >= b);
}

TEST(ByteTest, ToString) {
  Byte a(0x42);
  EXPECT_EQ(a.to_string(), "0x42");

  Byte b(0x0A);
  EXPECT_EQ(b.to_string(), "0x0A");

  Byte c(0xFF);
  EXPECT_EQ(c.to_string(), "0xFF");
}
