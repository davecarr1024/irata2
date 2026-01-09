#ifndef IRATA2_BASE_TYPES_H
#define IRATA2_BASE_TYPES_H

#include <cstdint>
#include <string>

#include <fmt/format.h>

namespace irata2::base {

// 8-bit byte value (0x00 - 0xFF)
class Byte {
 public:
  constexpr Byte() : value_(0x00) {}
  constexpr explicit Byte(uint8_t value) : value_(value) {}

  constexpr uint8_t value() const { return value_; }

  // Arithmetic operations
  constexpr Byte operator+(const Byte& other) const {
    return Byte(value_ + other.value_);
  }

  constexpr Byte operator-(const Byte& other) const {
    return Byte(value_ - other.value_);
  }

  constexpr Byte operator&(const Byte& other) const {
    return Byte(value_ & other.value_);
  }

  constexpr Byte operator|(const Byte& other) const {
    return Byte(value_ | other.value_);
  }

  constexpr Byte operator^(const Byte& other) const {
    return Byte(value_ ^ other.value_);
  }

  constexpr Byte operator~() const { return Byte(~value_); }

  constexpr Byte operator<<(int shift) const {
    return Byte(value_ << shift);
  }

  constexpr Byte operator>>(int shift) const {
    return Byte(value_ >> shift);
  }

  // Comparison operations
  constexpr bool operator==(const Byte& other) const {
    return value_ == other.value_;
  }

  constexpr bool operator!=(const Byte& other) const {
    return value_ != other.value_;
  }

  constexpr bool operator<(const Byte& other) const {
    return value_ < other.value_;
  }

  constexpr bool operator<=(const Byte& other) const {
    return value_ <= other.value_;
  }

  constexpr bool operator>(const Byte& other) const {
    return value_ > other.value_;
  }

  constexpr bool operator>=(const Byte& other) const {
    return value_ >= other.value_;
  }

  // String representation
  std::string to_string() const {
    return fmt::format("0x{:02X}", value_);
  }

 private:
  uint8_t value_;
};

// 16-bit word value (0x0000 - 0xFFFF)
class Word {
 public:
  constexpr Word() : value_(0x0000) {}
  constexpr explicit Word(uint16_t value) : value_(value) {}
  constexpr Word(Byte high, Byte low)
      : value_((static_cast<uint16_t>(high.value()) << 8) | low.value()) {}

  constexpr uint16_t value() const { return value_; }

  constexpr Byte high() const { return Byte(value_ >> 8); }

  constexpr Byte low() const { return Byte(value_ & 0xFF); }

  // Arithmetic operations
  constexpr Word operator+(const Word& other) const {
    return Word(value_ + other.value_);
  }

  constexpr Word operator-(const Word& other) const {
    return Word(value_ - other.value_);
  }

  constexpr Word operator&(const Word& other) const {
    return Word(value_ & other.value_);
  }

  constexpr Word operator|(const Word& other) const {
    return Word(value_ | other.value_);
  }

  constexpr Word operator^(const Word& other) const {
    return Word(value_ ^ other.value_);
  }

  constexpr Word operator~() const { return Word(~value_); }

  constexpr Word operator<<(int shift) const {
    return Word(value_ << shift);
  }

  constexpr Word operator>>(int shift) const {
    return Word(value_ >> shift);
  }

  // Comparison operations
  constexpr bool operator==(const Word& other) const {
    return value_ == other.value_;
  }

  constexpr bool operator!=(const Word& other) const {
    return value_ != other.value_;
  }

  constexpr bool operator<(const Word& other) const {
    return value_ < other.value_;
  }

  constexpr bool operator<=(const Word& other) const {
    return value_ <= other.value_;
  }

  constexpr bool operator>(const Word& other) const {
    return value_ > other.value_;
  }

  constexpr bool operator>=(const Word& other) const {
    return value_ >= other.value_;
  }

  // String representation
  std::string to_string() const {
    return fmt::format("0x{:04X}", value_);
  }

 private:
  uint16_t value_;
};

}  // namespace irata2::base

#endif  // IRATA2_BASE_TYPES_H
