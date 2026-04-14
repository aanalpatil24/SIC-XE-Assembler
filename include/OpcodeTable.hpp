#ifndef OPTAB_HPP
#define OPTAB_HPP

#include "Types.hpp"
#include <unordered_map>
#include <string>

namespace sicxe {

struct OpcodeEntry {
    uint8_t opcode;
    InstructionFormat format;
    int operands;  // Expected number of operands
};

class OpcodeTable {
private:
    std::unordered_map<std::string, OpcodeEntry> table; // Static map of mnemonics to machine codes
    void initTable();
    
public:
    OpcodeTable();
    
    bool exists(const std::string& mnemonic) const;
    OpcodeEntry get(const std::string& mnemonic) const;
    InstructionFormat getFormat(const std::string& mnemonic) const;
};

} // namespace sicxe
#endif