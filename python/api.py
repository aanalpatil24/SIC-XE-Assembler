from python.assembler_wrapper import SICXEAssembler

asm = SICXEAssembler()
result = asm.assemble("source.asm", "output")
if result.success:
    print(f"Generated {result.program_length} bytes of object code")