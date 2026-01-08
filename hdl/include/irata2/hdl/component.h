#ifndef IRATA2_HDL_COMPONENT_H
#define IRATA2_HDL_COMPONENT_H

#include <string>

namespace irata2::hdl {

// Forward declaration
class Cpu;

// Abstract base class for all HDL components
// HDL components are immutable structural metadata
class Component {
 public:
  virtual ~Component() = default;

  // Every component can access the root CPU
  virtual const Cpu& cpu() const = 0;

  // Get component path for debugging
  virtual std::string path() const = 0;
};

// Base class for all non-root HDL components
// Non-root components have a parent reference
class ComponentWithParent : public Component {
 public:
  explicit ComponentWithParent(const Component& parent,
                               const std::string& name)
      : parent_(parent), name_(name) {}

  const Component& parent() const { return parent_; }
  const std::string& name() const { return name_; }

  const Cpu& cpu() const override { return parent_.cpu(); }

  std::string path() const override {
    return parent_.path() + "/" + name_;
  }

 private:
  const Component& parent_;
  const std::string name_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COMPONENT_H
