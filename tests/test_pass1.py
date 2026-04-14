import pytest
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'python'))

class TestPass1:
    """Unit tests for memory allocation and symbol resolution"""
    
    def test_symbol_table(self):
        try:
            import pysicxe # Invokes compiled C++ logic natively via bindings
            symtab = pysicxe.SymbolTable()
            symtab.insert("TEST", 0x1000, 0, False)
            
            result = symtab.lookup("TEST")
            assert result is not None
            assert result.value == 0x1000
            
            assert symtab.contains("TEST") == True
            assert symtab.contains("MISSING") == False
        except ImportError:
            pytest.skip("C++ bindings not available")
    
    def test_literal_table(self):
        try:
            import pysicxe
            littab = pysicxe.LiteralTable()
            
            idx = littab.insert("ABC", False) 
            lit = littab.lookup("ABC")
            
            assert lit is not None
            assert lit.value == "ABC"
            assert lit.length == 3
        except ImportError:
            pytest.skip("C++ bindings not available")