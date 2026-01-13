#ifndef IRATA2_SIM_COMPONENT_H
#define IRATA2_SIM_COMPONENT_H

#include <string>
#include <vector>

#include "irata2/base/tick_phase.h"

namespace irata2::sim {

// Forward declaration
class Cpu;

// Abstract base class for all simulator components
// Simulator components have runtime state and tick behavior
class Component {
 public:
  virtual ~Component() = default;  // LCOV_EXCL_LINE

  // Every component can access the root CPU
  virtual Cpu& cpu() = 0;
  virtual const Cpu& cpu() const = 0;

  // Get component path for debugging
  virtual std::string path() const = 0;

  // Current tick phase - public for component hierarchy access
  virtual base::TickPhase current_phase() const = 0;

 protected:
  // Children registered during construction - populated by RegisterChild()
  std::vector<Component*> children_;

  // Register a child component with the root CPU for ticking.
  // Protected: only called during component construction
  virtual void RegisterChild(Component& child) { children_.push_back(&child); }

  // Five-phase tick model
  // Base implementations automatically propagate to children
  // Components override these to add their own behavior
  virtual void TickControl() {
    for (auto* child : children_) {
      child->TickControl();
    }
  }

  virtual void TickWrite() {
    for (auto* child : children_) {
      child->TickWrite();
    }
  }

  virtual void TickRead() {
    for (auto* child : children_) {
      child->TickRead();
    }
  }

  virtual void TickProcess() {
    for (auto* child : children_) {
      child->TickProcess();
    }
  }

  virtual void TickClear() {
    for (auto* child : children_) {
      child->TickClear();
    }
  }
};

// Base class for all non-root simulator components
// Non-root components have a parent reference
class ComponentWithParent : public Component {
 public:
  explicit ComponentWithParent(Component& parent, const std::string& name)
      : parent_(parent), name_(name) {}

  Component& parent() { return parent_; }
  const Component& parent() const { return parent_; }

  const std::string& name() const { return name_; }

  Cpu& cpu() override { return parent_.cpu(); }
  const Cpu& cpu() const override { return parent_.cpu(); }

  base::TickPhase current_phase() const override {
    return parent_.current_phase();
  }

  std::string path() const override {
    if (parent_.path().empty()) {
      return name_;
    }
    return parent_.path() + "." + name_;
  }

 private:
  Component& parent_;
  const std::string name_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_COMPONENT_H
