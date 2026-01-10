#ifndef IRATA2_MICROCODE_IR_CPU_PATH_RESOLVER_H
#define IRATA2_MICROCODE_IR_CPU_PATH_RESOLVER_H

#include "irata2/hdl/control_info.h"
#include "irata2/hdl/cpu.h"
#include "irata2/microcode/error.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace irata2::microcode::ir {

/// Resolves control paths to ControlInfo pointers.
/// This class owns the path-to-control index and is constructed once
/// during microcode compilation. Path resolution is only used during
/// DSL parsing; after that, all references are stored as ControlInfo*.
class CpuPathResolver {
 public:
  explicit CpuPathResolver(const hdl::Cpu& cpu);

  /// Find a control by path. Returns nullptr if not found.
  const hdl::ControlInfo* FindControl(std::string_view path) const;

  /// Require a control by path. Throws MicrocodeError if not found.
  const hdl::ControlInfo* RequireControl(std::string_view path,
                                         std::string_view context) const;

  /// Get all control paths (sorted alphabetically).
  const std::vector<std::string>& AllControlPaths() const {
    return control_paths_;
  }

 private:
  void IndexControls(const hdl::Cpu& cpu);

  std::unordered_map<std::string, const hdl::ControlInfo*> controls_by_path_;
  std::vector<std::string> control_paths_;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_CPU_PATH_RESOLVER_H
