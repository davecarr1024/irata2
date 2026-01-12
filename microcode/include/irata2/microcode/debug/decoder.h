#ifndef IRATA2_MICROCODE_DEBUG_DECODER_H
#define IRATA2_MICROCODE_DEBUG_DECODER_H

#include "irata2/microcode/output/program.h"

#include <string>

namespace irata2::microcode::debug {

/**
 * @brief Decodes compiled microcode into human-readable format.
 *
 * Takes a compiled MicrocodeProgram and provides various views:
 * - Control word decoding (bit positions to control names)
 * - Per-instruction dumps (opcode -> steps -> controls)
 * - Full program dumps in readable format
 *
 * Example usage:
 *   MicrocodeDecoder decoder(program);
 *   std::string dump = decoder.DumpProgram();
 */
class MicrocodeDecoder {
 public:
  explicit MicrocodeDecoder(const output::MicrocodeProgram& program);

  /**
   * @brief Decode a single control word into control names.
   * @param control_word The encoded control word
   * @return Vector of control path strings for asserted bits
   */
  std::vector<std::string> DecodeControlWord(uint64_t control_word) const;

  /**
   * @brief Dump the entire program in readable format.
   * @return Multi-line string with all opcodes, steps, and controls
   */
  std::string DumpProgram() const;

  /**
   * @brief Dump a specific instruction.
   * @param opcode The instruction opcode
   * @return Multi-line string with all steps and controls for this opcode
   */
  std::string DumpInstruction(uint8_t opcode) const;

 private:
  const output::MicrocodeProgram& program_;

  std::string DecodeStatusBits(uint8_t status) const;
};

}  // namespace irata2::microcode::debug

#endif  // IRATA2_MICROCODE_DEBUG_DECODER_H
