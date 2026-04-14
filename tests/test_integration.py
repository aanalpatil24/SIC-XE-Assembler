import pytest
import subprocess
import os

class TestSICXEIntegration:
    """Integration tests validating standard and extended executable features"""
    
    def test_simple_assembly(self, temp_dir, assembler_exe, sample_programs):
        source_file = os.path.join(temp_dir, "test.asm")
        obj_file = os.path.join(temp_dir, "test.obj")
        
        with open(source_file, 'w') as f:
            f.write(sample_programs['simple'])
        
        result = subprocess.run(
            [assembler_exe, source_file, obj_file],
            capture_output=True,
            text=True
        )
        
        assert result.returncode == 0, f"Assembly failed: {result.stderr}"
        assert os.path.exists(obj_file), "Object file not created"
        
        with open(obj_file, 'r') as f:
            lines = f.readlines()
        
        assert lines[0].startswith('H')
        assert 'COPY' in lines[0]
        
        text_records = [l for l in lines if l.startswith('T')]
        assert len(text_records) > 0
        assert lines[-1].startswith('E')
    
    def test_literal_handling(self, temp_dir, assembler_exe, sample_programs):
        source_file = os.path.join(temp_dir, "lit.asm")
        obj_file = os.path.join(temp_dir, "lit.obj")
        
        with open(source_file, 'w') as f:
            f.write(sample_programs['literals'])
        
        result = subprocess.run(
            [assembler_exe, source_file, obj_file],
            capture_output=True,
            text=True
        )
        
        assert result.returncode == 0
        
        with open(obj_file, 'r') as f:
            content = f.read()
        
        assert '414243' in content or 'ABC' in content 
    
    def test_error_detection(self, temp_dir, assembler_exe):
        source = '''
        START   0
        LDA     UNDEFINED
        END
'''
        source_file = os.path.join(temp_dir, "err.asm")
        obj_file = os.path.join(temp_dir, "err.obj")
        
        with open(source_file, 'w') as f:
            f.write(source)
        
        result = subprocess.run(
            [assembler_exe, source_file, obj_file],
            capture_output=True,
            text=True
        )
        
        assert result.returncode != 0 or 'ERROR' in result.stdout

class TestPythonInterface:
    """Tests isolating the Python PLY grammar logic"""
    
    def test_parser_tokens(self):
        from python.sicxe_parser import parse_assembly
        
        source = "LABEL   LDA     #100"
        lines = parse_assembly(source)
        
        assert len(lines) == 1
        assert lines[0].label == "LABEL"
        assert lines[0].opcode == "LDA"
        assert lines[0].operand == "#100"
        assert lines[0].is_extended == False
    
    def test_extended_format_parsing(self):
        from python.sicxe_parser import parse_assembly
        
        source = "        +JSUB   SUBRTN"
        lines = parse_assembly(source)
        
        assert lines[0].is_extended == True
        assert lines[0].opcode == "JSUB"