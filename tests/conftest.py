import pytest
import os
import tempfile
import shutil

@pytest.fixture
def temp_dir():
    """Provide a temporary directory for test outputs"""
    dir_path = tempfile.mkdtemp()
    yield dir_path
    shutil.rmtree(dir_path)

@pytest.fixture
def assembler_exe():
    """Path to compiled assembler executable"""
    path = os.path.join(os.path.dirname(__file__), '..', 'build', 'sicxe_assembler')
    if not os.path.exists(path):
        pytest.skip("Assembler executable not built")
    return os.path.abspath(path)

@pytest.fixture
def sample_programs():
    """Dictionary of sample SIC/XE programs for testing suites"""
    return {
        'simple': '''
COPY    START   1000
FIRST   STL     RETADR
        LDB     #LENGTH
        BASE    LENGTH
CLOOP   +JSUB   RDREC
        LDA     LENGTH
        COMP    #0
        JEQ     ENDFIL
        +JSUB   WRREC
        J       CLOOP
ENDFIL  LDA     EOF
        STA     BUFFER
        LDA     #3
        STA     LENGTH
        +JSUB   WRREC
        J       @RETADR
EOF     BYTE    C'EOF'
RETADR  RESW    1
LENGTH  RESW    1
BUFFER  RESB    4096
        END     FIRST
''',
        'literals': '''
        START   0
        LDA     =C'ABC'
        LDB     =X'F1F2'
        LTORG
        LDA     =C'DEF'
        END
''',
        'progblocks': '''
        START   0
        USE     DEFAULT
        LDA     #10
        USE     DATA
        WORD    5
        USE
        ADD     #5
        END
'''
    }