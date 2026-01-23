#include "irata2/sim/io/input_device.h"

#include <gtest/gtest.h>

#include "irata2/sim.h"
#include "irata2/sim/memory/memory.h"
#include "irata2/sim/memory/region.h"
#include "test_helpers.h"

using namespace irata2::sim;
using namespace irata2::sim::io;
using namespace irata2::base;

namespace {

// Helper to create a CPU with an InputDevice at the standard MMIO address
// Returns a pair of (Cpu, InputDevice*) where the InputDevice pointer
// is valid for the lifetime of the Cpu.
std::pair<Cpu, InputDevice*> MakeCpuWithInputDevice() {
  InputDevice* device_ptr = nullptr;

  std::vector<memory::Memory::RegionFactory> factories;
  factories.push_back([&device_ptr](memory::Memory& m) -> std::unique_ptr<memory::Region> {
    return std::make_unique<memory::Region>(
        "input_device", m, Word{io::INPUT_DEVICE_BASE},
        [&device_ptr](memory::Region& r) -> std::unique_ptr<memory::Module> {
          auto device = std::make_unique<InputDevice>("input", r);
          device_ptr = device.get();
          return device;
        });
  });

  Cpu cpu(DefaultHdl(), test::MakeNoopProgram(), {}, std::move(factories));
  return {std::move(cpu), device_ptr};
}

}  // namespace

class InputDeviceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto [cpu, dev] = MakeCpuWithInputDevice();
    cpu_ = std::make_unique<Cpu>(std::move(cpu));
    device_ = dev;
  }

  std::unique_ptr<Cpu> cpu_;
  InputDevice* device_ = nullptr;
};

// --- Queue behavior tests ---

TEST_F(InputDeviceTest, StartsEmpty) {
  EXPECT_TRUE(device_->empty());
  EXPECT_FALSE(device_->full());
  EXPECT_EQ(device_->count(), 0);
}

TEST_F(InputDeviceTest, InjectKeyAddsToQueue) {
  device_->inject_key(0x41);  // 'A'
  EXPECT_FALSE(device_->empty());
  EXPECT_EQ(device_->count(), 1);
}

TEST_F(InputDeviceTest, InjectMultipleKeys) {
  device_->inject_key(0x01);  // Up
  device_->inject_key(0x02);  // Down
  device_->inject_key(0x03);  // Left

  EXPECT_EQ(device_->count(), 3);
  EXPECT_FALSE(device_->full());
}

TEST_F(InputDeviceTest, QueueBecomesFull) {
  for (size_t i = 0; i < InputDevice::QUEUE_SIZE; ++i) {
    device_->inject_key(static_cast<uint8_t>(i));
  }

  EXPECT_TRUE(device_->full());
  EXPECT_EQ(device_->count(), InputDevice::QUEUE_SIZE);
}

TEST_F(InputDeviceTest, DropsInputWhenFull) {
  // Fill the queue
  for (size_t i = 0; i < InputDevice::QUEUE_SIZE; ++i) {
    device_->inject_key(static_cast<uint8_t>(i));
  }

  // Try to add one more
  device_->inject_key(0xFF);

  // Should still be full with same count (dropped)
  EXPECT_TRUE(device_->full());
  EXPECT_EQ(device_->count(), InputDevice::QUEUE_SIZE);
}

TEST_F(InputDeviceTest, FifoOrder) {
  device_->inject_key(0x01);
  device_->inject_key(0x02);
  device_->inject_key(0x03);

  // Read DATA register (pops)
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x01);
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x02);
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x03);
  EXPECT_TRUE(device_->empty());
}

TEST_F(InputDeviceTest, CircularBufferWraparound) {
  // Fill and drain several times to test wraparound
  for (int cycle = 0; cycle < 5; ++cycle) {
    for (size_t i = 0; i < InputDevice::QUEUE_SIZE; ++i) {
      device_->inject_key(static_cast<uint8_t>(i + cycle * 16));
    }
    EXPECT_TRUE(device_->full());

    for (size_t i = 0; i < InputDevice::QUEUE_SIZE; ++i) {
      uint8_t expected = static_cast<uint8_t>(i + cycle * 16);
      EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), expected);
    }
    EXPECT_TRUE(device_->empty());
  }
}

// --- Register read tests ---

TEST_F(InputDeviceTest, StatusRegisterWhenEmpty) {
  uint8_t status = device_->Read(Word{input_reg::STATUS}).value();
  EXPECT_EQ(status & input_status::NOT_EMPTY, 0);
  EXPECT_EQ(status & input_status::FULL, 0);
  EXPECT_EQ(status & input_status::IRQ_PENDING, 0);
}

TEST_F(InputDeviceTest, StatusRegisterNotEmpty) {
  device_->inject_key(0x41);
  uint8_t status = device_->Read(Word{input_reg::STATUS}).value();
  EXPECT_NE(status & input_status::NOT_EMPTY, 0);
  EXPECT_EQ(status & input_status::FULL, 0);
}

TEST_F(InputDeviceTest, StatusRegisterFull) {
  for (size_t i = 0; i < InputDevice::QUEUE_SIZE; ++i) {
    device_->inject_key(static_cast<uint8_t>(i));
  }

  uint8_t status = device_->Read(Word{input_reg::STATUS}).value();
  EXPECT_NE(status & input_status::NOT_EMPTY, 0);
  EXPECT_NE(status & input_status::FULL, 0);
}

TEST_F(InputDeviceTest, DataRegisterReturnsZeroWhenEmpty) {
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x00);
}

TEST_F(InputDeviceTest, DataRegisterPops) {
  device_->inject_key(0x42);
  device_->inject_key(0x43);

  EXPECT_EQ(device_->count(), 2);
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x42);
  EXPECT_EQ(device_->count(), 1);
  EXPECT_EQ(device_->Read(Word{input_reg::DATA}).value(), 0x43);
  EXPECT_EQ(device_->count(), 0);
}

TEST_F(InputDeviceTest, PeekRegisterDoesNotPop) {
  device_->inject_key(0x44);

  EXPECT_EQ(device_->Read(Word{input_reg::PEEK}).value(), 0x44);
  EXPECT_EQ(device_->count(), 1);  // Still there
  EXPECT_EQ(device_->Read(Word{input_reg::PEEK}).value(), 0x44);  // Same value
  EXPECT_EQ(device_->count(), 1);
}

TEST_F(InputDeviceTest, PeekRegisterReturnsZeroWhenEmpty) {
  EXPECT_EQ(device_->Read(Word{input_reg::PEEK}).value(), 0x00);
}

TEST_F(InputDeviceTest, CountRegister) {
  EXPECT_EQ(device_->Read(Word{input_reg::COUNT}).value(), 0);

  device_->inject_key(0x01);
  EXPECT_EQ(device_->Read(Word{input_reg::COUNT}).value(), 1);

  device_->inject_key(0x02);
  EXPECT_EQ(device_->Read(Word{input_reg::COUNT}).value(), 2);

  device_->Read(Word{input_reg::DATA});  // Pop one
  EXPECT_EQ(device_->Read(Word{input_reg::COUNT}).value(), 1);
}

TEST_F(InputDeviceTest, ReservedRegistersReturnZero) {
  for (uint8_t offset = 5; offset < InputDevice::MMIO_SIZE; ++offset) {
    EXPECT_EQ(device_->Read(Word{offset}).value(), 0x00);
  }
}

// --- Register write tests ---

TEST_F(InputDeviceTest, ControlRegisterEnablesIrq) {
  EXPECT_FALSE(device_->irq_pending());

  // Enable IRQ
  device_->Write(Word{input_reg::CONTROL}, Byte{input_control::IRQ_ENABLE});

  // Still no IRQ (queue empty)
  EXPECT_FALSE(device_->irq_pending());

  // Add input - now IRQ should be pending
  device_->inject_key(0x41);
  EXPECT_TRUE(device_->irq_pending());
}

TEST_F(InputDeviceTest, ControlRegisterDisablesIrq) {
  device_->Write(Word{input_reg::CONTROL}, Byte{input_control::IRQ_ENABLE});
  device_->inject_key(0x41);
  EXPECT_TRUE(device_->irq_pending());

  // Disable IRQ
  device_->Write(Word{input_reg::CONTROL}, Byte{0x00});
  EXPECT_FALSE(device_->irq_pending());
}

TEST_F(InputDeviceTest, IrqClearsWhenQueueEmpties) {
  device_->Write(Word{input_reg::CONTROL}, Byte{input_control::IRQ_ENABLE});
  device_->inject_key(0x41);
  EXPECT_TRUE(device_->irq_pending());

  // Read (pop) the input
  device_->Read(Word{input_reg::DATA});
  EXPECT_FALSE(device_->irq_pending());
}

TEST_F(InputDeviceTest, WritesToReadOnlyRegistersIgnored) {
  device_->inject_key(0x41);

  // Try to write to STATUS (read-only)
  device_->Write(Word{input_reg::STATUS}, Byte{0xFF});

  // Should still have data
  EXPECT_EQ(device_->count(), 1);
}

TEST_F(InputDeviceTest, WritesToReservedRegistersIgnored) {
  device_->inject_key(0x41);

  // Write to reserved registers
  for (uint8_t offset = 5; offset < InputDevice::MMIO_SIZE; ++offset) {
    device_->Write(Word{offset}, Byte{0xFF});
  }

  // Should still work normally
  EXPECT_EQ(device_->count(), 1);
}

// --- IRQ status bit tests ---

TEST_F(InputDeviceTest, StatusShowsIrqPending) {
  device_->Write(Word{input_reg::CONTROL}, Byte{input_control::IRQ_ENABLE});
  device_->inject_key(0x41);

  uint8_t status = device_->Read(Word{input_reg::STATUS}).value();
  EXPECT_NE(status & input_status::IRQ_PENDING, 0);
}

TEST_F(InputDeviceTest, StatusClearsIrqPendingWhenDisabled) {
  device_->Write(Word{input_reg::CONTROL}, Byte{input_control::IRQ_ENABLE});
  device_->inject_key(0x41);

  // Disable IRQ
  device_->Write(Word{input_reg::CONTROL}, Byte{0x00});

  uint8_t status = device_->Read(Word{input_reg::STATUS}).value();
  EXPECT_EQ(status & input_status::IRQ_PENDING, 0);
}

// --- Module interface tests ---

TEST_F(InputDeviceTest, SizeReturns16) {
  EXPECT_EQ(device_->size(), InputDevice::MMIO_SIZE);
  EXPECT_EQ(device_->size(), 16);
}

// --- MMIO routing tests (via CPU memory map) ---

TEST_F(InputDeviceTest, MmioReadStatusViaMemory) {
  // Inject a key so status is non-zero
  device_->inject_key(0x41);

  // Read status via memory map
  Byte status = cpu_->memory().ReadAt(Word{io::INPUT_DEVICE_BASE + input_reg::STATUS});
  EXPECT_NE(status.value() & input_status::NOT_EMPTY, 0);
}

TEST_F(InputDeviceTest, MmioReadDataViaMemory) {
  device_->inject_key(0x42);
  device_->inject_key(0x43);

  // Read data via memory map - should pop
  Byte data1 = cpu_->memory().ReadAt(Word{io::INPUT_DEVICE_BASE + input_reg::DATA});
  EXPECT_EQ(data1.value(), 0x42);

  Byte data2 = cpu_->memory().ReadAt(Word{io::INPUT_DEVICE_BASE + input_reg::DATA});
  EXPECT_EQ(data2.value(), 0x43);

  EXPECT_TRUE(device_->empty());
}

TEST_F(InputDeviceTest, MmioWriteControlViaMemory) {
  // Write control register via memory map to enable IRQ
  cpu_->memory().WriteAt(Word{io::INPUT_DEVICE_BASE + input_reg::CONTROL},
                         Byte{input_control::IRQ_ENABLE});

  device_->inject_key(0x41);
  EXPECT_TRUE(device_->irq_pending());
}

TEST_F(InputDeviceTest, MmioReadCountViaMemory) {
  device_->inject_key(0x01);
  device_->inject_key(0x02);
  device_->inject_key(0x03);

  Byte count = cpu_->memory().ReadAt(Word{io::INPUT_DEVICE_BASE + input_reg::COUNT});
  EXPECT_EQ(count.value(), 3);
}
