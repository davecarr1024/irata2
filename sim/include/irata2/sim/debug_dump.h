#ifndef IRATA2_SIM_DEBUG_DUMP_H
#define IRATA2_SIM_DEBUG_DUMP_H

#include <string>
#include <string_view>

namespace irata2::sim {

class Cpu;

std::string FormatDebugDump(const Cpu& cpu, std::string_view reason);

}  // namespace irata2::sim

#endif  // IRATA2_SIM_DEBUG_DUMP_H
