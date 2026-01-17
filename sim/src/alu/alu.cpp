#include "irata2/sim/alu/alu.h"

namespace irata2::sim::alu {

Alu::Alu(std::string name,
         Component& parent,
         Bus<base::Byte>& data_bus,
         StatusRegister& status)
    : ComponentWithParent(parent, std::move(name)),
      lhs_("lhs", *this, data_bus),
      rhs_("rhs", *this, data_bus),
      result_("result", *this, data_bus),
      opcode_bit_0_("opcode_bit_0", *this),
      opcode_bit_1_("opcode_bit_1", *this),
      opcode_bit_2_("opcode_bit_2", *this),
      opcode_bit_3_("opcode_bit_3", *this),
      carry_(status.carry()),
      overflow_(status.overflow()) {}

bool Alu::CarryIn() const {
  return carry_.value();
}

void Alu::SetCarryOut(bool value) {
  carry_.Set(value);
}

void Alu::TickProcess() {
  uint8_t opcode = 0;
  if (opcode_bit_0_.asserted()) {
    opcode |= 1U << 0;
  }
  if (opcode_bit_1_.asserted()) {
    opcode |= 1U << 1;
  }
  if (opcode_bit_2_.asserted()) {
    opcode |= 1U << 2;
  }
  if (opcode_bit_3_.asserted()) {
    opcode |= 1U << 3;
  }

  if (opcode == 0) {
    return;
  }

  const uint16_t lhs = lhs_.value().value();
  const uint16_t rhs = rhs_.value().value();
  const uint16_t borrow = CarryIn() ? 0U : 1U;

  switch (opcode) {
    case 0x1: {  // ADD (with carry in)
      const uint16_t carry_in = CarryIn() ? 1U : 0U;
      const uint16_t result = static_cast<uint16_t>(lhs + rhs + carry_in);
      result_.set_value(base::Byte{static_cast<uint8_t>(result & 0xFFu)});
      SetCarryOut(result > 0xFFu);

      // Calculate overflow: occurs if sign bit changes incorrectly
      // Overflow happens when: pos + pos = neg OR neg + neg = pos
      const bool lhs_sign = (lhs & 0x80u) != 0;
      const bool rhs_sign = (rhs & 0x80u) != 0;
      const bool result_sign = (result & 0x80u) != 0;
      const bool overflow = (lhs_sign == rhs_sign) && (lhs_sign != result_sign);
      overflow_.Set(overflow);
      break;
    }
    case 0x2: {  // SUB (MVP: used for CMP)
      const uint16_t subtrahend = static_cast<uint16_t>(rhs + borrow);
      const uint16_t result = static_cast<uint16_t>(lhs - subtrahend);
      result_.set_value(base::Byte{static_cast<uint8_t>(result & 0xFFu)});
      SetCarryOut(lhs >= subtrahend);
      break;
    }
    case 0x3: {  // INC (increment, ignores carry)
      const uint16_t result = static_cast<uint16_t>(lhs + 1u);
      result_.set_value(base::Byte{static_cast<uint8_t>(result & 0xFFu)});
      // INC doesn't affect carry or overflow flags
      break;
    }
    case 0x4: {  // AND (bitwise and)
      const uint8_t result = static_cast<uint8_t>(lhs & rhs);
      result_.set_value(base::Byte{result});
      // Logic operations clear carry and overflow
      SetCarryOut(false);
      overflow_.Set(false);
      break;
    }
    case 0x5: {  // OR (bitwise or)
      const uint8_t result = static_cast<uint8_t>(lhs | rhs);
      result_.set_value(base::Byte{result});
      // Logic operations clear carry and overflow
      SetCarryOut(false);
      overflow_.Set(false);
      break;
    }
    case 0x6: {  // XOR (bitwise exclusive or)
      const uint8_t result = static_cast<uint8_t>(lhs ^ rhs);
      result_.set_value(base::Byte{result});
      // Logic operations clear carry and overflow
      SetCarryOut(false);
      overflow_.Set(false);
      break;
    }
    case 0x7: {  // ASL (arithmetic shift left)
      const uint8_t lhs_byte = static_cast<uint8_t>(lhs);
      const bool bit7 = (lhs_byte & 0x80u) != 0;
      const uint8_t result = static_cast<uint8_t>(lhs_byte << 1);
      result_.set_value(base::Byte{result});
      SetCarryOut(bit7);  // Bit 7 goes to carry
      overflow_.Set(false);  // Shifts clear overflow
      break;
    }
    case 0x8: {  // LSR (logical shift right)
      const uint8_t lhs_byte = static_cast<uint8_t>(lhs);
      const bool bit0 = (lhs_byte & 0x01u) != 0;
      const uint8_t result = static_cast<uint8_t>(lhs_byte >> 1);
      result_.set_value(base::Byte{result});
      SetCarryOut(bit0);  // Bit 0 goes to carry
      overflow_.Set(false);  // Shifts clear overflow
      break;
    }
    case 0x9: {  // ROL (rotate left through carry)
      const uint8_t lhs_byte = static_cast<uint8_t>(lhs);
      const bool bit7 = (lhs_byte & 0x80u) != 0;
      const bool carry_in = CarryIn();
      const uint8_t result = static_cast<uint8_t>((lhs_byte << 1) | (carry_in ? 1u : 0u));
      result_.set_value(base::Byte{result});
      SetCarryOut(bit7);  // Bit 7 goes to carry
      overflow_.Set(false);  // Rotates clear overflow
      break;
    }
    case 0xA: {  // ROR (rotate right through carry)
      const uint8_t lhs_byte = static_cast<uint8_t>(lhs);
      const bool bit0 = (lhs_byte & 0x01u) != 0;
      const bool carry_in = CarryIn();
      const uint8_t result = static_cast<uint8_t>((lhs_byte >> 1) | (carry_in ? 0x80u : 0u));
      result_.set_value(base::Byte{result});
      SetCarryOut(bit0);  // Bit 0 goes to carry
      overflow_.Set(false);  // Rotates clear overflow
      break;
    }
    default:
      break;
  }
}

}  // namespace irata2::sim::alu
