#include "Pass1.hpp"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

namespace sicxe {

Pass1::Pass1(SymbolTable& st, LiteralTable& lt, ProgramBlockManager& bm, ErrorHandler& eh)
    : symtab(st), littab(lt), blockManager(bm), errorHandler(eh), 
      startAddress(0), programLength(0), started(false), ended(false) {}

bool Pass1::process(const std::vector<std::string>& sourceLines) {
    int lineNum = 0;
    
    for (const auto& line : sourceLines) {
        lineNum++;
        try {
            processLine(line, lineNum);
        } catch (const AssemblyError& e) {
            errorHandler.fatal(lineNum, e.what());
            return false;
        }
    }
    
    if (!ended) {
        errorHandler.error(lineNum, "Missing END directive");
    }
    
    // Flush remaining literal pool at end of program
    if (littab.hasUnassigned()) {
        Address poolStart = blockManager.getCurrentLoc();
        littab.assignAddresses(poolStart, blockManager.getCurrentBlockNumber());
        blockManager.setLoc(littab.poolEndAddress());
    }
    
    blockManager.organizeBlocks();
    programLength = blockManager.getTotalLength();
    
    return !errorHandler.hasErrors();
}

void Pass1::processLine(const std::string& line, int lineNum) {
    std::string trimmed = line;
    size_t commentPos = trimmed.find('.');
    if (commentPos != std::string::npos) {
        trimmed = trimmed.substr(0, commentPos);
    }
    
    // Ignore entirely blank lines
    if (trimmed.empty() || std::all_of(trimmed.begin(), trimmed.end(), [](unsigned char c){ return std::isspace(c); })) {
        return;
    }
    
    std::istringstream iss(trimmed);
    std::string label, opcode, operand;
    
    iss >> label;
    
    if (optab.exists(label) || label[0] == '+' || label == "START" || 
        label == "END" || label == "BYTE" || label == "WORD" || 
        label == "RESB" || label == "RESW" || label == "USE" || 
        label == "LTORG" || label == "EQU" || label == "ORG" || 
        label == "BASE" || label == "NOBASE") {
        opcode = label;
        label = "";
        iss >> operand;
    } else {
        iss >> opcode >> operand;
    }
    
    bool extended = false;
    if (!opcode.empty() && opcode[0] == '+') {
        extended = true;
        opcode = opcode.substr(1);
    }
    
    Address currentLoc = blockManager.getCurrentLoc();
    
    // Enter label into Symbol Table
    if (!label.empty()) {
        if (symtab.contains(label)) {
            errorHandler.error(lineNum, "Duplicate symbol: " + label);
        } else {
            symtab.insert(label, currentLoc, blockManager.getCurrentBlockNumber());
        }
    }
    
    if (opcode == "START") {
        if (started) {
            errorHandler.error(lineNum, "Multiple START directives");
        } else {
            started = true;
            programName = label;
            if (!operand.empty()) {
                // FIX: Added try/catch to protect against stoul crashing on bad input
                try {
                    startAddress = std::stoul(operand, nullptr, 16);
                    blockManager.setLoc(startAddress);
                } catch (...) {
                    errorHandler.error(lineNum, "Invalid START address format");
                }
            }
        }
    } else if (opcode == "END") {
        ended = true;
    } else if (opcode == "BYTE") {
        int len = 1;
        if (operand[0] == 'C' && operand[1] == '\'') {
            len = operand.length() - 3; 
        } else if (operand[0] == 'X' && operand[1] == '\'') {
            len = (operand.length() - 3 + 1) / 2; 
        }
        blockManager.incrementLoc(len);
    } else if (opcode == "WORD") {
        blockManager.incrementLoc(3);
    } else if (opcode == "RESB") {
        blockManager.incrementLoc(std::stoi(operand));
    } else if (opcode == "RESW") {
        blockManager.incrementLoc(3 * std::stoi(operand));
    } else if (opcode == "USE") {
        blockManager.setBlock(operand.empty() ? "DEFAULT" : operand);
    } else if (opcode == "LTORG") {
        if (littab.hasUnassigned()) {
            Address poolStart = blockManager.getCurrentLoc();
            littab.assignAddresses(poolStart, blockManager.getCurrentBlockNumber());
            blockManager.setLoc(littab.poolEndAddress());
        }
    } else if (opcode == "EQU") {
        if (!label.empty()) {
            Address value = 0;
            bool absolute = true;
            if (std::isdigit(operand[0])) {
                value = std::stoul(operand, nullptr, 10);
            } else {
                auto sym = symtab.lookup(operand);
                if (sym) {
                    value = sym->value;
                    absolute = sym->isAbsolute;
                }
            }
            symtab.insert(label, value, 0, absolute);
        }
    } else if (opcode == "BASE" || opcode == "NOBASE" || opcode == "ORG") {
        // Ignored in Pass 1
    } else if (optab.exists(opcode)) {
        int len = calculateInstructionLength(opcode, operand);
        if (extended) len = 4;
        blockManager.incrementLoc(len);
        
        // Scan operand for implicit literal declarations
        if (!operand.empty() && operand[0] == '=') {
            bool isHex = (operand[1] == 'X');
            size_t start = 3; 
            size_t len = operand.length() - start - 1; 
            std::string value = operand.substr(start, len);
            littab.insert(value, isHex);
        }
    } else {
        errorHandler.error(lineNum, "Unknown operation: " + opcode);
    }
}

int Pass1::calculateInstructionLength(const std::string& opcode, const std::string& operand) {
    auto entry = optab.get(opcode);
    if (entry.format == InstructionFormat::FORMAT_1) return 1;
    if (entry.format == InstructionFormat::FORMAT_2) return 2;
    
    // Checks if large immediate value forces Format 4
    if (!operand.empty() && operand[0] == '#') {
        std::string val = operand.substr(1);
        try {
            if (std::stoi(val) > 4095) return 4;
        } catch (...) {}
    }
    
    return 3; 
}

} // namespace sicxe