#!/usr/bin/env python3
"""
Python wrapper for C++ SIC/XE Assembler
Handles preprocessing, parsing, and calls C++ core via pybind11
"""

import subprocess
import tempfile
import os
from typing import List, Tuple, Optional
from dataclasses import dataclass
from sicxe_parser import parse_assembly, ParsedLine

@dataclass
class AssemblyResult:
    success: bool
    object_file: Optional[str]
    listing_file: Optional[str]
    errors: List[str]
    warnings: List[str]
    symbol_table: dict
    program_length: int

class SICXEAssembler:
    """High-level assembler interface"""
    
    def __init__(self, cpp_executable: str = "./sicxe_assembler"):
        self.cpp_exe = cpp_executable
        self.temp_dir = tempfile.mkdtemp()
        
    def assemble(self, source: str, output_name: Optional[str] = None) -> AssemblyResult:
        """
        Assemble SIC/XE source code.
        
        Args:
            source: Assembly source code or filename
            output_name: Base name for output files
        """
        # Determine input type
        if os.path.exists(source):
            input_file = source
            with open(source, 'r') as f:
                source_content = f.read()
        else:
            source_content = source
            input_file = os.path.join(self.temp_dir, "input.asm")
            with open(input_file, 'w') as f:
                f.write(source_content)
        
        # Parse with Python PLY first (syntax validation)
        try:
            parsed_lines = parse_assembly(source_content)
            print(f"Parsed {len(parsed_lines)} lines successfully")
        except Exception as e:
            return AssemblyResult(
                success=False,
                object_file=None,
                listing_file=None,
                errors=[f"Parser error: {str(e)}"],
                warnings=[],
                symbol_table={},
                program_length=0
            )
        
        # Generate output paths
        if output_name is None:
            output_name = os.path.join(self.temp_dir, "output")
        
        obj_file = output_name + ".obj"
        lst_file = output_name + ".lst"
        
        # Call C++ assembler
        try:
            result = subprocess.run(
                [self.cpp_exe, input_file, obj_file],
                capture_output=True,
                text=True,
                timeout=30
            )
            
            errors = []
            warnings = []
            success = (result.returncode == 0)
            
            # Parse output for diagnostics
            for line in result.stdout.split('\n'):
                if '[ERROR]' in line:
                    errors.append(line)
                elif '[WARNING]' in line:
                    warnings.append(line)
            
            # Also check stderr
            if result.stderr:
                errors.extend(result.stderr.split('\n'))
            
            # Extract symbol table from listing if successful
            symtab = {}
            if os.path.exists(lst_file):
                symtab = self._extract_symtab_from_listing(lst_file)
            
            # Get program length
            prog_len = 0
            if os.path.exists(obj_file):
                prog_len = self._get_program_length(obj_file)
            
            return AssemblyResult(
                success=success and len(errors) == 0,
                object_file=obj_file if success else None,
                listing_file=lst_file if success else None,
                errors=errors,
                warnings=warnings,
                symbol_table=symtab,
                program_length=prog_len
            )
            
        except subprocess.TimeoutExpired:
            return AssemblyResult(
                success=False,
                object_file=None,
                listing_file=None,
                errors=["Assembly timeout"],
                warnings=[],
                symbol_table={},
                program_length=0
            )
        except Exception as e:
            return AssemblyResult(
                success=False,
                object_file=None,
                listing_file=None,
                errors=[f"Execution error: {str(e)}"],
                warnings=[],
                symbol_table={},
                program_length=0
            )
    
    def _extract_symtab_from_listing(self, lst_file: str) -> dict:
        """Extract symbol table from listing file"""
        symtab = {}
        # Implementation depends on listing format
        return symtab
    
    def _get_program_length(self, obj_file: str) -> int:
        """Extract program length from object file header"""
        try:
            with open(obj_file, 'r') as f:
                header = f.readline()
                if header.startswith('H') and len(header) >= 19:
                    length_hex = header[13:19]
                    return int(length_hex, 16)
        except:
            pass
        return 0

# CLI interface
if __name__ == "__main__":
    import argparse
    
    parser = argparse.ArgumentParser(description='SIC/XE Assembler')
    parser.add_argument('source', help='Source assembly file')
    parser.add_argument('-o', '--output', help='Output object file')
    parser.add_argument('--cpp-path', default='./sicxe_assembler', 
                       help='Path to C++ assembler executable')
    
    args = parser.parse_args()
    
    asm = SICXEAssembler(cpp_executable=args.cpp_path)
    result = asm.assemble(args.source, args.output)
    
    if result.success:
        print(f"Assembly successful!")
        print(f"Object file: {result.object_file}")
        print(f"Program length: {result.program_length} bytes")
    else:
        print("Assembly failed!")
        for error in result.errors:
            print(f"  ERROR: {error}")
        for warning in result.warnings:
            print(f"  WARNING: {warning}")