#ifndef IRATA2_HDL_COMPONENT_BASE_H
#define IRATA2_HDL_COMPONENT_BASE_H

#include <string>

namespace irata2::hdl {

class Cpu;

class ComponentBase {
 public:
  const std::string& name() const { return name_; }
  const std::string& path() const { return path_; }

  Cpu& cpu() { return *cpu_; }
  const Cpu& cpu() const { return *cpu_; }

  const ComponentBase* parent_ptr() const { return parent_; }

 protected:
  ComponentBase(std::string name, const ComponentBase* parent, Cpu* cpu);
  ComponentBase(std::string name, Cpu* cpu);

 private:
  const std::string name_;
  const std::string path_;
  Cpu* const cpu_;
  const ComponentBase* const parent_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COMPONENT_BASE_H
