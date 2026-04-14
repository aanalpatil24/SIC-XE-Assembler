#ifndef PASS2_HPP
#define PASS2_HPP

#include "Types.hpp"
#include "SymbolTable.hpp"
#include "LiteralTable.hpp"
#include "ProgramBlock.hpp"
#include "OpcodeTable.hpp"
#include "ObjectWriter.hpp"
#include "ErrorHandler.hpp"
#include <vector>
#include <string>

namespace sicxe {

// Pass 2 performs actual machine code generation and displacement calculations
class Pass2 {
private:
    SymbolTable& symtab;
    LiteralTable& littab;
    ProgramBlockManager& blockManager;
    OpcodeTable optab;
    ObjectWriter& objWriter;
    ErrorHandler& errorHandler;
    
    Address baseRegister;
    bool baseSet; // Tracks if LDB has been called
    
    std::vector<Instruction> intermediate;
    
    void processLine(const std::string& line, int lineNum, Address loc);
    std::vector<Byte> generateObjectCode(const Instruction& inst);
    
    // Format specific generators
    std::vector<Byte> genFormat1(uint8_t opcode);
    std::vector<Byte> genFormat2(uint8_t opcode, const std::string& operands);
    std::vector<Byte> genFormat3(uint8_t opcode, const std::string& operand, 
                                 Address loc, Flags flags);
    std::vector<Byte> genFormat4(uint8_t opcode, const std::string& operand, 
                                 Address loc, Flags flags);
    
    Address parseOperand(const std::string& operand, Flags& flags, bool& isImmediateValue);
    int getRegisterNumber(const std::string& reg);
    
public:
    Pass2(SymbolTable& st, LiteralTable& lt, ProgramBlockManager& bm, 
          ObjectWriter& ow, ErrorHandler& eh);
    
    bool process(const std::vector<std::string>& sourceLines);
    const auto& getIntermediate() const { return intermediate; }
};

} // namespace sicxe
#endif