#ifndef IRATA2_SIM_COMPONENT_H
#define IRATA2_SIM_COMPONENT_H

#include <string>
#include <vector>

#include "irata2/base/tick_phase.h"

namespace irata2::sim {

// Forward declaration
class Cpu;

/**
 * @brief Abstract base class for all simulator components.
 *
 * Components form a tree hierarchy set up during construction:
 * - Parent/child relationships are immutable after construction
 * - RegisterChild() populates the children list during construction only
 * - Tick methods automatically propagate to all children
 *
 * @see ComponentWithParent for non-root components with a parent reference
 * @see Cpu for the root component
 */
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
  /**
   * @brief List of child components populated during construction.
   *
   * Structural invariant: This list is populated by RegisterChild() during
   * component construction and should not be modified after construction
   * completes. Tick methods automatically iterate this list.
   */
  std::vector<Component*> children_;

  /**
   * @brief Register a child component for tick propagation.
   *
   * This method should only be called during component construction to build
   * the component hierarchy. The hierarchy is immutable after construction.
   *
   * @param child Component to register as a child
   */
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

/**
 * @brief Base class for all non-root simulator components.
 *
 * ComponentWithParent provides a parent reference set during construction.
 * The parent reference is immutable - it cannot be rebound to point to a
 * different parent after construction.
 *
 * Structural invariants:
 * - parent_ is set in the constructor and never changes (C++ references
 *   cannot be rebound)
 * - name_ is const and set during construction
 * - The component hierarchy (parent/child relationships) is immutable
 *   after construction completes
 *
 * @see Component for the base class with tick propagation
 * @see Cpu for the root component (which has no parent)
 */
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
  Component& parent_;       // Immutable: reference cannot be rebound
  const std::string name_;  // Immutable: const member
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_COMPONENT_H
