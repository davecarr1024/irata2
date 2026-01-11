#ifndef IRATA2_SIM_DEBUG_TRACE_H
#define IRATA2_SIM_DEBUG_TRACE_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <vector>

#include "irata2/base/types.h"

namespace irata2::sim {

struct DebugTraceEntry {
  uint64_t cycle = 0;
  base::Word instruction_address;
  base::Word pc;
  base::Byte ir;
  base::Byte sc;
  base::Byte a;
  base::Byte x;
  base::Byte status;
};

class DebugTraceBuffer {
 public:
  void Configure(size_t depth) {
    depth_ = depth;
    entries_.clear();
  }

  bool enabled() const { return depth_ > 0; }
  size_t depth() const { return depth_; }
  size_t size() const { return entries_.size(); }

  void Record(DebugTraceEntry entry) {
    if (!enabled()) {
      return;
    }
    if (entries_.size() >= depth_) {
      entries_.pop_front();
    }
    entries_.push_back(std::move(entry));
  }

  std::vector<DebugTraceEntry> entries() const {
    return std::vector<DebugTraceEntry>(entries_.begin(), entries_.end());
  }

 private:
  size_t depth_ = 0;
  std::deque<DebugTraceEntry> entries_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_DEBUG_TRACE_H
