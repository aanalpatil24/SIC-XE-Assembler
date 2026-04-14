from python.assembler_wrapper import SICXEAssembler

# Simple high-level API script to test the assembler's end-to-end execution
asm = SICXEAssembler()
result = asm.assemble("source.asm", "output")
if result.success:
    print(f"Generated {result.program_length} bytes of object code")
else:    
    print("Assembly failed with errors:")
for error in result.errors:
    print(f" - {error}")