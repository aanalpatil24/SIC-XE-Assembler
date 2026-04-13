#include "OpcodeTable.hpp"

namespace sicxe {

OpcodeTable::OpcodeTable() {
    initTable();
}

void OpcodeTable::initTable() {
    // Format 1
    table["FIX"] = {0xC4, InstructionFormat::FORMAT_1, 0};
    table["FLOAT"] = {0xC0, InstructionFormat::FORMAT_1, 0};
    table["HIO"] = {0xF4, InstructionFormat::FORMAT_1, 0};
    table["NORM"] = {0xC8, InstructionFormat::FORMAT_1, 0};
    table["SIO"] = {0xF0, InstructionFormat::FORMAT_1, 0};
    table["TIO"] = {0xF8, InstructionFormat::FORMAT_1, 0};
    
    // Format 2
    table["ADDR"] = {0x90, InstructionFormat::FORMAT_2, 2};
    table["CLEAR"] = {0xB4, InstructionFormat::FORMAT_2, 1};
    table["COMPR"] = {0xA0, InstructionFormat::FORMAT_2, 2};
    table["DIVR"] = {0x9C, InstructionFormat::FORMAT_2, 2};
    table["MULR"] = {0x98, InstructionFormat::FORMAT_2, 2};
    table["RMO"] = {0xAC, InstructionFormat::FORMAT_2, 2};
    table["SHIFTL"] = {0xA4, InstructionFormat::FORMAT_2, 2};
    table["SHIFTR"] = {0xA8, InstructionFormat::FORMAT_2, 2};
    table["SUBR"] = {0x94, InstructionFormat::FORMAT_2, 2};
    table["SVC"] = {0xB0, InstructionFormat::FORMAT_2, 1};
    table["TIXR"] = {0xB8, InstructionFormat::FORMAT_2, 1};
    
    // Format 3/4
    table["ADD"] = {0x18, InstructionFormat::FORMAT_3, 1};
    table["ADDF"] = {0x58, InstructionFormat::FORMAT_3, 1};
    table["AND"] = {0x40, InstructionFormat::FORMAT_3, 1};
    table["COMP"] = {0x28, InstructionFormat::FORMAT_3, 1};
    table["COMPF"] = {0x88, InstructionFormat::FORMAT_3, 1};
    table["DIV"] = {0x24, InstructionFormat::FORMAT_3, 1};
    table["DIVF"] = {0x64, InstructionFormat::FORMAT_3, 1};
    table["J"] = {0x3C, InstructionFormat::FORMAT_3, 1};
    table["JEQ"] = {0x30, InstructionFormat::FORMAT_3, 1};
    table["JGT"] = {0x34, InstructionFormat::FORMAT_3, 1};
    table["JLT"] = {0x38, InstructionFormat::FORMAT_3, 1};
    table["JSUB"] = {0x48, InstructionFormat::FORMAT_3, 1};
    table["LDA"] = {0x00, InstructionFormat::FORMAT_3, 1};
    table["LDB"] = {0x68, InstructionFormat::FORMAT_3, 1};
    table["LDCH"] = {0x50, InstructionFormat::FORMAT_3, 1};
    table["LDF"] = {0x70, InstructionFormat::FORMAT_3, 1};
    table["LDL"] = {0x08, InstructionFormat::FORMAT_3, 1};
    table["LDS"] = {0x6C, InstructionFormat::FORMAT_3, 1};
    table["LDT"] = {0x74, InstructionFormat::FORMAT_3, 1};
    table["LDX"] = {0x04, InstructionFormat::FORMAT_3, 1};
    table["LPS"] = {0xD0, InstructionFormat::FORMAT_3, 1};
    table["MUL"] = {0x20, InstructionFormat::FORMAT_3, 1};
    table["MULF"] = {0x60, InstructionFormat::FORMAT_3, 1};
    table["OR"] = {0x44, InstructionFormat::FORMAT_3, 1};
    table["RD"] = {0xD8, InstructionFormat::FORMAT_3, 1};
    table["RSUB"] = {0x4C, InstructionFormat::FORMAT_3, 0};
    table["SSK"] = {0xEC, InstructionFormat::FORMAT_3, 1};
    table["STA"] = {0x0C, InstructionFormat::FORMAT_3, 1};
    table["STB"] = {0x78, InstructionFormat::FORMAT_3, 1};
    table["STCH"] = {0x54, InstructionFormat::FORMAT_3, 1};
    table["STF"] = {0x80, InstructionFormat::FORMAT_3, 1};
    table["STI"] = {0xD4, InstructionFormat::FORMAT_3, 1};
    table["STL"] = {0x14, InstructionFormat::FORMAT_3, 1};
    table["STS"] = {0x7C, InstructionFormat::FORMAT_3, 1};
    table["STSW"] = {0xE8, InstructionFormat::FORMAT_3, 1};
    table["STT"] = {0x84, InstructionFormat::FORMAT_3, 1};
    table["STX"] = {0x10, InstructionFormat::FORMAT_3, 1};
    table["SUB"] = {0x1C, InstructionFormat::FORMAT_3, 1};
    table["SUBF"] = {0x5C, InstructionFormat::FORMAT_3, 1};
    table["TD"] = {0xE0, InstructionFormat::FORMAT_3, 1};
    table["TIX"] = {0x2C, InstructionFormat::FORMAT_3, 1};
    table["WD"] = {0xDC, InstructionFormat::FORMAT_3, 1};
}

bool OpcodeTable::exists(const std::string& mnemonic) const {
    return table.find(mnemonic) != table.end();
}

OpcodeEntry OpcodeTable::get(const std::string& mnemonic) const {
    auto it = table.find(mnemonic);
    if (it != table.end()) return it->second;
    return {0, InstructionFormat::FORMAT_3, 0}; // Default
}

InstructionFormat OpcodeTable::getFormat(const std::string& mnemonic) const {
    auto it = table.find(mnemonic);
    if (it != table.end()) return it->second.format;
    return InstructionFormat::FORMAT_3;
}

} // namespace sicxe