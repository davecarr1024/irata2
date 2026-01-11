#include "irata2/sim/debug_trace.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(DebugTraceBufferTest, DisabledByDefaultDropsEntries) {
  DebugTraceBuffer buffer;
  DebugTraceEntry entry;
  entry.cycle = 1;

  buffer.Record(entry);
  EXPECT_EQ(buffer.size(), 0u);
}

TEST(DebugTraceBufferTest, RetainsMostRecentEntries) {
  DebugTraceBuffer buffer;
  buffer.Configure(2);

  DebugTraceEntry entry1;
  entry1.cycle = 1;
  DebugTraceEntry entry2;
  entry2.cycle = 2;
  DebugTraceEntry entry3;
  entry3.cycle = 3;

  buffer.Record(entry1);
  buffer.Record(entry2);
  buffer.Record(entry3);

  auto entries = buffer.entries();
  ASSERT_EQ(entries.size(), 2u);
  EXPECT_EQ(entries[0].cycle, 2u);
  EXPECT_EQ(entries[1].cycle, 3u);
}
