#include "irata2/sim/debug_symbols.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace irata2::sim;

namespace {
std::string WriteTempDebugJson(const std::string& content) {
  std::filesystem::path path =
      std::filesystem::temp_directory_path() / "irata2_debug_symbols.json";
  std::ofstream out(path, std::ios::trunc);
  out << content;
  out.close();
  return path.string();
}
}  // namespace

TEST(DebugSymbolsTest, LoadsDebugSymbolsFromJson) {
  const std::string json = R"({
    "version": "v1",
    "cartridge_version": 1,
    "entry": "0x8000",
    "rom_size": 32768,
    "source_root": ".",
    "source_files": ["test.asm"],
    "symbols": {"start": "0x8000"},
    "pc_to_source": {
      "0x8000": {"file": "test.asm", "line": 1, "column": 1, "text": "HLT"}
    },
    "records": [{
      "address": "0x8000",
      "rom_offset": 0,
      "file": "test.asm",
      "line": 1,
      "column": 1,
      "text": "HLT"
    }]
  })";

  const std::string path = WriteTempDebugJson(json);
  DebugSymbols symbols = LoadDebugSymbols(path);

  EXPECT_EQ(symbols.version, "v1");
  EXPECT_EQ(symbols.entry.value(), 0x8000);
  EXPECT_EQ(symbols.rom_size, 32768u);
  EXPECT_EQ(symbols.source_root, ".");
  ASSERT_EQ(symbols.source_files.size(), 1u);
  EXPECT_EQ(symbols.source_files[0], "test.asm");

  auto it = symbols.symbols.find("start");
  ASSERT_NE(it, symbols.symbols.end());
  EXPECT_EQ(it->second.value(), 0x8000);

  auto location = symbols.Lookup(irata2::base::Word{0x8000});
  ASSERT_TRUE(location.has_value());
  EXPECT_EQ(location->file, "test.asm");
  EXPECT_EQ(location->line, 1);
  EXPECT_EQ(location->column, 1);
  EXPECT_EQ(location->text, "HLT");
}
