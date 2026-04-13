#ifndef PASS1_HPP
#define PASS1_HPP

#include "Types.hpp"
#include "SymbolTable.hpp"
#include "LiteralTable.hpp"
#include "ProgramBlock.hpp"
#include "OpcodeTable.hpp"
#include "ErrorHandler.hpp"
#include <vector>
#include <string>
#include <memory>

namespace sicxe {

class Pass1 {
private:
    SymbolTable& symtab;
    LiteralTable& littab;
    ProgramBlockManager& blockManager;
    OpcodeTable optab;
    ErrorHandler& errorHandler;
    
    Address startAddress;
    Address programLength;
    std::string programName;
    bool started;
    bool ended;
    
    void processLine(const std::string& line, int lineNum);
    void handleDirective(const std::string& label, const std::string& opcode, 
                        const std::string& operand, int lineNum);
    void handleInstruction(const std::string& label, const std::string& opcode, 
                          const std::string& operand, int lineNum);
    
    int calculateInstructionLength(const std::string& opcode, const std::string& operand);
    
public:
    Pass1(SymbolTable& st, LiteralTable& lt, ProgramBlockManager& bm, ErrorHandler& eh);
    
    bool process(const std::vector<std::string>& sourceLines);
    Address getStartAddress() const { return startAddress; }
    Address getProgramLength() const { return programLength; }
    std::string getProgramName() const { return programName; }
};

} // namespace sicxe

#endif