#!/usr/bin/env python3
import argparse
import json
import os
import re
import subprocess
import tempfile


HEX_RE = re.compile(r"^0x[0-9a-fA-F]+$")


def die(message: str) -> None:
    raise SystemExit(message)


def load_json(path: str):
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def require(condition: bool, message: str) -> None:
    if not condition:
        die(message)


def validate_hex(value, label: str) -> None:
    require(isinstance(value, str), f"{label} must be a string")
    require(HEX_RE.match(value), f"{label} must be hex like 0x8000")


def validate_debug_json(data, schema):
    require(isinstance(schema, dict), "schema must be a JSON object")
    schema_version = schema.get("schema_version")
    require(isinstance(schema_version, str), "schema schema_version must be a string")

    require(isinstance(data, dict), "debug JSON must be a JSON object")
    version = data.get("version")
    require(version == schema_version,
            f"debug JSON version {version} does not match schema {schema_version}")

    for key in ("entry", "rom_size", "source_root", "source_files",
                "symbols", "pc_to_source", "records"):
        require(key in data, f"missing required field: {key}")

    validate_hex(data["entry"], "entry")
    require(isinstance(data["rom_size"], int), "rom_size must be an integer")
    require(isinstance(data["source_root"], str), "source_root must be a string")

    source_files = data["source_files"]
    require(isinstance(source_files, list), "source_files must be a list")
    for item in source_files:
        require(isinstance(item, str), "source_files entries must be strings")

    symbols = data["symbols"]
    require(isinstance(symbols, dict), "symbols must be an object")
    for name, value in symbols.items():
        require(isinstance(name, str), "symbols keys must be strings")
        validate_hex(value, f"symbols[{name}]")

    pc_to_source = data["pc_to_source"]
    require(isinstance(pc_to_source, dict), "pc_to_source must be an object")
    for address, entry in pc_to_source.items():
        validate_hex(address, "pc_to_source key")
        require(isinstance(entry, dict), "pc_to_source values must be objects")
        for field in ("file", "line", "column", "text"):
            require(field in entry, f"pc_to_source entry missing {field}")
        require(isinstance(entry["file"], str), "pc_to_source.file must be a string")
        require(entry["file"] in source_files,
                "pc_to_source.file must be listed in source_files")
        require(isinstance(entry["line"], int) and entry["line"] >= 1,
                "pc_to_source.line must be a positive integer")
        require(isinstance(entry["column"], int) and entry["column"] >= 1,
                "pc_to_source.column must be a positive integer")
        require(isinstance(entry["text"], str), "pc_to_source.text must be a string")

    records = data["records"]
    require(isinstance(records, list), "records must be a list")
    for record in records:
        require(isinstance(record, dict), "records entries must be objects")
        for field in ("address", "rom_offset", "file", "line", "column", "text"):
            require(field in record, f"record missing {field}")
        validate_hex(record["address"], "record.address")
        require(isinstance(record["rom_offset"], int) and record["rom_offset"] >= 0,
                "record.rom_offset must be a non-negative integer")
        require(isinstance(record["file"], str), "record.file must be a string")
        require(record["file"] in source_files, "record.file must be listed in source_files")
        require(isinstance(record["line"], int) and record["line"] >= 1,
                "record.line must be a positive integer")
        require(isinstance(record["column"], int) and record["column"] >= 1,
                "record.column must be a positive integer")
        require(isinstance(record["text"], str), "record.text must be a string")

    org_map = data.get("org_map")
    if org_map is not None:
        require(isinstance(org_map, list), "org_map must be a list when present")
        for segment in org_map:
            require(isinstance(segment, dict), "org_map entries must be objects")
            for field in ("start", "end"):
                require(field in segment, f"org_map entry missing {field}")
                validate_hex(segment[field], f"org_map.{field}")


def run_assembler(assembler: str, source: str, output_dir: str) -> str:
    output_bin = os.path.join(output_dir, "out.cart")
    output_debug = os.path.join(output_dir, "out.debug.json")
    subprocess.run([assembler, source, output_bin, output_debug],
                   check=True)
    return output_debug


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate irata2 debug JSON output.")
    parser.add_argument("--schema", required=True, help="Path to JSON schema file.")
    parser.add_argument("--input", help="Path to debug JSON to validate.")
    parser.add_argument("--assembler", help="Path to irata2_asm binary.")
    parser.add_argument("--source", help="ASM source file to assemble for validation.")
    args = parser.parse_args()

    schema = load_json(args.schema)

    if args.input:
        debug_path = args.input
        data = load_json(debug_path)
        validate_debug_json(data, schema)
        return 0

    if args.assembler and args.source:
        with tempfile.TemporaryDirectory() as tempdir:
            debug_path = run_assembler(args.assembler, args.source, tempdir)
            data = load_json(debug_path)
            validate_debug_json(data, schema)
        return 0

    die("Either --input or both --assembler and --source are required.")


if __name__ == "__main__":
    raise SystemExit(main())
