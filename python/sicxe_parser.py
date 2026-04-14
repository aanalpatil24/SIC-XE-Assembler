#!/usr/bin/env python3
"""
SIC/XE Assembly Language Parser
Using PLY (Python Lex-Yacc)
"""

import ply.lex as lex
import ply.yacc as yacc
import re
from dataclasses import dataclass
from typing import List, Optional, Union

# Lexical tokens expected by the parser
tokens = (
    'LABEL', 'MNEMONIC', 'OPERAND', 'COMMENT',
    'PLUS', 'COMMA', 'HASH', 'AT', 'QUOTE',
    'NUMBER', 'STRING', 'HEX', 'REGISTER',
)

t_PLUS = r'\+'
t_COMMA = r','
t_HASH = r'\#'
t_AT = r'@'
t_QUOTE = r"'"
t_ignore = ' \t'

registers = {
    'A': 'REGISTER', 'X': 'REGISTER', 'L': 'REGISTER',
    'B': 'REGISTER', 'S': 'REGISTER', 'T': 'REGISTER',
    'F': 'REGISTER', 'PC': 'REGISTER', 'SW': 'REGISTER'
}

directives = {
    'START', 'END', 'BYTE', 'WORD', 'RESB', 'RESW',
    'USE', 'LTORG', 'EQU', 'ORG', 'BASE', 'NOBASE',
    'EXTDEF', 'EXTREF', 'CSECT'
}

instructions = {
    'ADD', 'ADDF', 'ADDR', 'AND', 'CLEAR', 'COMP', 'COMPF', 'COMPR',
    'DIV', 'DIVF', 'DIVR', 'FIX', 'FLOAT', 'HIO', 'J', 'JEQ', 'JGT',
    'JLT', 'JSUB', 'LDA', 'LDB', 'LDCH', 'LDF', 'LDL', 'LDS', 'LDT',
    'LDX', 'LPS', 'MUL', 'MULF', 'MULR', 'NORM', 'OR', 'RD', 'RMO',
    'RSUB', 'SHIFTL', 'SHIFTR', 'SIO', 'SSK', 'STA', 'STB', 'STCH',
    'STF', 'STI', 'STL', 'STS', 'STSW', 'STT', 'STX', 'SUB', 'SUBF',
    'SUBR', 'SVC', 'TD', 'TIO', 'TIX', 'TIXR', 'WD'
}

@dataclass
class ParsedLine:
    line_num: int
    label: Optional[str]
    opcode: str
    operand: Optional[str]
    is_extended: bool
    is_directive: bool
    raw_line: str
    comment: Optional[str] = None

def t_COMMENT(t):
    r'\..*'
    t.value = t.value[1:].strip()
    return t

def t_MNEMONIC(t):
    r'[A-Z]+'
    upper_val = t.value.upper()
    if upper_val in registers:
        t.type = 'REGISTER'
    elif upper_val in directives:
        t.type = 'MNEMONIC'
        t.value = upper_val
    elif upper_val in instructions:
        t.type = 'MNEMONIC'
        t.value = upper_val
    else:
        t.type = 'LABEL' 
    return t

def t_NUMBER(t):
    r'\d+'
    t.value = int(t.value)
    return t

def t_HEX(t):
    r'X\'[0-9A-Fa-f]+\''
    t.value = t.value[2:-1] 
    return t

def t_STRING(t):
    r'C\'[^\']*\''
    t.value = t.value[2:-1] 
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print(f"Illegal character '{t.value[0]}' at line {t.lineno}")
    t.lexer.skip(1)

lexer = lex.lex()

# Defines grammar rules mapping assembly format constructs
def p_program(p):
    '''program : statements'''
    p[0] = p[1]

def p_statements(p):
    '''statements : statement
                  | statements statement'''
    if len(p) == 2:
        p[0] = [p[1]]
    else:
        p[0] = p[1] + [p[2]]

def p_statement(p):
    '''statement : line COMMENT
                 | line'''
    line_obj = p[1]
    if len(p) == 3:
        line_obj.comment = p[2]
    p[0] = line_obj

def p_line(p):
    '''line : LABEL MNEMONIC operand
            | LABEL PLUS MNEMONIC operand
            | MNEMONIC operand
            | PLUS MNEMONIC operand
            | LABEL MNEMONIC
            | MNEMONIC'''
    
    if len(p) == 2: 
        p[0] = ParsedLine(
            line_num=p.lineno(1),
            label=None,
            opcode=p[1],
            operand=None,
            is_extended=False,
            is_directive=p[1] in directives,
            raw_line=p.value
        )
    elif len(p) == 3: 
        if isinstance(p[1], str) and p[1].upper() in instructions or p[1].upper() in directives:
            p[0] = ParsedLine(
                line_num=p.lineno(1),
                label=None,
                opcode=p[1],
                operand=p[2],
                is_extended=False,
                is_directive=p[1] in directives,
                raw_line=p.value
            )
        else:
            p[0] = ParsedLine(
                line_num=p.lineno(1),
                label=p[1],
                opcode=p[2],
                operand=None,
                is_extended=False,
                is_directive=p[2] in directives,
                raw_line=p.value
            )
    elif len(p) == 4:
        if p[2] == '+': 
            p[0] = ParsedLine(
                line_num=p.lineno(1),
                label=p[1],
                opcode=p[3],
                operand=None,
                is_extended=True,
                is_directive=False,
                raw_line=p.value
            )
        else:
            p[0] = ParsedLine(
                line_num=p.lineno(1),
                label=p[1],
                opcode=p[2],
                operand=p[3],
                is_extended=False,
                is_directive=p[2] in directives,
                raw_line=p.value
            )
    elif len(p) == 5: 
        p[0] = ParsedLine(
            line_num=p.lineno(1),
            label=p[1],
            opcode=p[3],
            operand=p[4],
            is_extended=True,
            is_directive=False,
            raw_line=p.value
        )

def p_operand(p):
    '''operand : expression
               | HASH expression
               | AT expression
               | literal'''
    if len(p) == 2:
        p[0] = p[1]
    else:
        p[0] = p[1] + p[2] if p[1] in ['#', '@'] else p[2]

def p_literal(p):
    '''literal : EQUAL HEX
               | EQUAL STRING
               | EQUAL X QUOTE STRING QUOTE
               | EQUAL C QUOTE STRING QUOTE'''
    if len(p) == 3:
        p[0] = '=' + p[2]
    else:
        type_char = 'X' if p[2].upper() == 'X' else 'C'
        p[0] = f'={type_char}\'{p[4]}\''

def p_expression(p):
    '''expression : term
                  | expression PLUS term
                  | expression COMMA REGISTER'''
    if len(p) == 2:
        p[0] = str(p[1])
    elif p[2] == '+':
        p[0] = str(p[1]) + '+' + str(p[3])
    else:
        p[0] = str(p[1]) + ',' + str(p[3])

def p_term(p):
    '''term : LABEL
            | NUMBER
            | REGISTER
            | HEX'''
    p[0] = p[1]

def p_error(p):
    if p:
        print(f"Syntax error at line {p.lineno}: {p.value}")
    else:
        print("Syntax error at EOF")

parser = yacc.yacc()

class SICXEParser:
    """High-level parser interface for SIC/XE assembly"""
    
    def __init__(self):
        self.lines = []
    
    def parse_file(self, filename: str) -> List[ParsedLine]:
        with open(filename, 'r') as f:
            content = f.read()
        return self.parse_string(content)
    
    def parse_string(self, content: str) -> List[ParsedLine]:
        lines = []
        line_num = 0
        
        for line in content.split('\n'):
            line_num += 1
            if not line.strip() or line.strip().startswith('.'):
                continue
                
            try:
                result = parser.parse(line, lexer=lexer)
                if result:
                    lines.append(result)
            except Exception as e:
                print(f"Error parsing line {line_num}: {line}")
                print(f"Exception: {e}")
        
        return lines

def parse_assembly(source: Union[str, List[str]]) -> List[ParsedLine]:
    parser = SICXEParser()
    if isinstance(source, str) and not source.endswith('.asm'):
        return parser.parse_string(source)
    elif isinstance(source, str):
        return parser.parse_file(source)
    else:
        return parser.parse_string('\n'.join(source))