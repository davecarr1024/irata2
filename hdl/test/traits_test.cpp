#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/read_control.h"
#include "irata2/hdl/status.h"
#include "irata2/hdl/traits.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(TraitsTest, IdentifiesBusControlRegister) {
  EXPECT_TRUE(is_bus_v<ByteBus>);
  EXPECT_TRUE(is_control_v<ReadControl<irata2::base::Byte>>);
  EXPECT_TRUE(is_register_v<ByteRegister>);
  EXPECT_TRUE(is_status_v<Status>);
}
