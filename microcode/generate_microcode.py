#!/usr/bin/env python3

import argparse
import textwrap
import yaml


def parse_yaml(path):
    with open(path, "r", encoding="utf-8") as handle:
        return yaml.safe_load(handle)


def emit_controls(controls):
    if not controls:
        return "{}"
    quoted = ", ".join(f'"{c}"' for c in controls)
    return "{" + quoted + "}"


def emit_step(stage, controls, context):
    return textwrap.dedent(
        f"""
        {{
          irata2::microcode::ir::Step step;
          step.stage = {stage};
          step.controls = builder.RequireControls({emit_controls(controls)}, "{context}");
          variant.steps.push_back(std::move(step));
        }}
        """
    )


def emit_variant(variant_index, variant):
    lines = []
    lines.append("{")
    lines.append("  irata2::microcode::ir::InstructionVariant variant;")

    when = variant.get("when") or {}
    if when:
        lines.append("  variant.status_conditions = {")
        for key, value in when.items():
            val = "true" if value else "false"
            lines.append(f"    {{\"{key.lower()}\", {val}}},")
        lines.append("  };")

    stages = variant.get("stages") or []
    for stage_index, stage in enumerate(stages):
        steps = stage.get("steps") or []
        for step_index, step in enumerate(steps):
            controls = step or []
            context = f"variant[{variant_index}].stage[{stage_index}].step[{step_index}]"
            lines.append(textwrap.indent(emit_step(stage_index, controls, context).strip(), "  "))

    lines.append("  instruction.variants.push_back(std::move(variant));")
    lines.append("}")
    return "\n".join(lines)


def emit_instruction(name, definition):
    name_upper = name.upper()
    lines = []
    lines.append("{")
    lines.append("  irata2::microcode::ir::Instruction instruction;")
    opcode_name = name_upper if "_" in name_upper else f"{name_upper}_IMP"
    lines.append(f"  instruction.opcode = irata2::isa::Opcode::{opcode_name};")

    if "variants" in definition:
        variants = definition["variants"] or []
        for idx, variant in enumerate(variants):
            lines.append(textwrap.indent(emit_variant(idx, variant), "  "))
    else:
        variant = {"when": {}, "stages": definition.get("stages") or []}
        lines.append(textwrap.indent(emit_variant(0, variant), "  "))

    lines.append("  instruction_set.instructions.push_back(std::move(instruction));")
    lines.append("}")
    return "\n".join(lines)


def emit_fetch_preamble(fetch_preamble):
    lines = []
    for index, step in enumerate(fetch_preamble or []):
        controls = step or []
        context = f"fetch_preamble[{index}]"
        snippet = textwrap.dedent(
            f"""
            {{
              irata2::microcode::ir::Step step;
              step.stage = 0;
              step.controls = builder.RequireControls({emit_controls(controls)}, "{context}");
              instruction_set.fetch_preamble.push_back(std::move(step));
            }}
            """
        ).strip()
        lines.append(snippet)
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("yaml_path")
    parser.add_argument("output_cpp")
    args = parser.parse_args()

    data = parse_yaml(args.yaml_path) or {}
    fetch_preamble = data.get("fetch_preamble") or []
    instructions = data.get("instructions") or {}

    lines = []
    lines.append("#include \"irata2/microcode/ir/irata_instruction_set.h\"")
    lines.append("#include \"irata2/microcode/ir/builder.h\"")
    lines.append("#include \"irata2/microcode/ir/cpu_path_resolver.h\"")
    lines.append("#include \"irata2/isa/isa.h\"")
    lines.append("")
    lines.append("namespace irata2::microcode::ir {")
    lines.append("")
    lines.append("InstructionSet BuildIrataInstructionSet(const hdl::Cpu& cpu) {")
    lines.append("  CpuPathResolver resolver(cpu);")
    lines.append("  Builder builder(resolver);")
    lines.append("  InstructionSet instruction_set;")
    lines.append(emit_fetch_preamble(fetch_preamble))

    for name, definition in instructions.items():
        lines.append(emit_instruction(name, definition))

    lines.append("  return instruction_set;")
    lines.append("}")
    lines.append("")
    lines.append("}  // namespace irata2::microcode::ir")
    lines.append("")

    with open(args.output_cpp, "w", encoding="utf-8") as handle:
        handle.write("\n".join(line for line in lines if line is not None))


if __name__ == "__main__":
    main()
