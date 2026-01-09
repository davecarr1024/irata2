#ifndef IRATA2_SIM_INITIALIZATION_H
#define IRATA2_SIM_INITIALIZATION_H

#include <memory>

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/output/program.h"

namespace irata2::sim {

std::shared_ptr<const hdl::Cpu> DefaultHdl();
std::shared_ptr<const microcode::output::MicrocodeProgram> DefaultMicrocodeProgram();

}  // namespace irata2::sim

#endif  // IRATA2_SIM_INITIALIZATION_H
