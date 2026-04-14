#include "Pass2.hpp"
#include <regex>
#include <sstream>
#include <iostream>

namespace sicxe {

Pass2::Pass2(SymbolTable& st, LiteralTable& lt, ProgramBlockManager& bm, 
             ObjectWriter& ow, ErrorHandler& eh)
    : symtab(st), littab(lt), blockManager(bm), objWriter(ow), errorHandler(eh),
      baseSet(false), baseRegister(0) {}

bool Pass2::process(const std::vector<std::string>& sourceLines) {
    int lineNum = 0;
    Address loc = 0;
    Address startAddress = 0; // Declared startAddress to track the entry point
    
    for (const auto& line : sourceLines) {
        lineNum++;
        
        std::string trimmed = line;
        size_t commentPos = trimmed.find('.');
        if (commentPos != std::string::npos) {
            trimmed = trimmed.substr(0, commentPos);
        }
        
        if (trimmed.empty()) continue;
        
        std::istringstream iss(trimmed);
        std::string label, opcode, operand;
        iss >> label;
        
        bool extended = false;
        
        if (optab.exists(label) || label[0] == '+' || 
            label == "START" || label == "END" || label == "BYTE" || 
            label == "WORD" || label == "RESB" || label == "RESW" || 
            label == "USE" || label == "LTORG" || label == "BASE" || 
            label == "NOBASE" || label == "EQU") {
            opcode = label;
            label = "";
            iss >> operand;
        } else {
            iss >> opcode >> operand;
        }
        
        if (!opcode.empty() && opcode[0] == '+') {
            extended = true;
            opcode = opcode.substr(1);
        }
        
        if (opcode == "START") {
            // Capture the start address so it can be used for the End Record
            startAddress = std::stoul(operand, nullptr, 16); 
            
            objWriter.setHeader(label.empty() ? "PROG" : label, 
                              startAddress, 
                              blockManager.getTotalLength());
            continue;
        }
        
        if (opcode == "END") continue;
        if (opcode == "EQU") continue;
        if (opcode == "USE") {
            blockManager.setBlock(operand.empty() ? "DEFAULT" : operand);
            continue;
        }
        
        loc = blockManager.getCurrentLoc() + 
              blockManager.getBlockStart(blockManager.getCurrentBlockNumber());
        
        if (opcode == "BASE") {
            auto sym = symtab.lookup(operand);
            if (sym) {
                baseRegister = sym->value + 
                    blockManager.getBlockStart(sym->blockNumber);
                baseSet = true;
            }
            continue;
        }
        
        if (opcode == "NOBASE") {
            baseSet = false;
            continue;
        }
        
        if (opcode == "LTORG") continue; 
        
        std::vector<Byte> objCode;
        std::string objCodeStr;
        
        if (opcode == "BYTE") {
            if (operand[0] == 'X') {
                std::string hex = operand.substr(2, operand.length() - 3);
                for (size_t i = 0; i < hex.length(); i += 2) {
                    std::string byte = hex.substr(i, 2);
                    objCode.push_back(std::stoul(byte, nullptr, 16));
                }
            } else if (operand[0] == 'C') {
                std::string chars = operand.substr(2, operand.length() - 3);
                for (char c : chars) objCode.push_back(static_cast<Byte>(c));
            }
            blockManager.incrementLoc(objCode.size());
            
        } else if (opcode == "WORD") {
            int val = std::stoi(operand);
            objCode.push_back((val >> 16) & 0xFF);
            objCode.push_back((val >> 8) & 0xFF);
            objCode.push_back(val & 0xFF);
            blockManager.incrementLoc(3);
            
        } else if (opcode == "RESB" || opcode == "RESW") {
            int len = (opcode == "RESB") ? std::stoi(operand) : 3 * std::stoi(operand);
            blockManager.incrementLoc(len);
            objWriter.addListingLine(loc, line, "");
            continue; 
        } else if (optab.exists(opcode)) {
            auto entry = optab.get(opcode);
            Instruction inst{label, opcode, operand, line, lineNum, 
                           entry.format, loc, {}, false};
            
            if (extended) inst.format = InstructionFormat::FORMAT_4;
            
            objCode = generateObjectCode(inst);
            blockManager.incrementLoc(objCode.size());
            
            if (inst.format == InstructionFormat::FORMAT_4 && !label.empty()) {
                objWriter.addModificationRecord(loc + 1, 5); 
            }
        }
        
        if (!objCode.empty()) {
            objWriter.addObjectCode(loc, objCode);
            objCodeStr = bytesToHex(objCode);
        }
        
        objWriter.addListingLine(loc, line, objCodeStr);
    }
    
    // startAddress is now correctly scoped and passed to the End Record
    objWriter.addEndRecord(startAddress);
    return !errorHandler.hasErrors();
}

std::vector<Byte> Pass2::generateObjectCode(const Instruction& inst) {
    auto entry = optab.get(inst.opcode);
    uint8_t opcode = entry.opcode;
    
    switch (inst.format) {
        case InstructionFormat::FORMAT_1: return genFormat1(opcode);
        case InstructionFormat::FORMAT_2: return genFormat2(opcode, inst.operand);
        case InstructionFormat::FORMAT_3: return genFormat3(opcode, inst.operand, inst.location, Flags());
        case InstructionFormat::FORMAT_4: return genFormat4(opcode, inst.operand, inst.location, Flags());
        default: return {};
    }
}

std::vector<Byte> Pass2::genFormat1(uint8_t op) {
    return {op};
}

std::vector<Byte> Pass2::genFormat2(uint8_t op, const std::string& operands) {
    std::istringstream iss(operands);
    std::string r1, r2;
    iss >> r1 >> r2;
    
    uint8_t reg1 = getRegisterNumber(r1);
    uint8_t reg2 = r2.empty() ? 0 : getRegisterNumber(r2);
    
    return {op, static_cast<Byte>((reg1 << 4) | reg2)};
}

int Pass2::getRegisterNumber(const std::string& reg) {
    if (reg == "A") return 0; if (reg == "X") return 1; if (reg == "L") return 2;
    if (reg == "B") return 3; if (reg == "S") return 4; if (reg == "T") return 5;
    if (reg == "F") return 6; if (reg == "PC") return 8; if (reg == "SW") return 9;
    return 0;
}

std::vector<Byte> Pass2::genFormat3(uint8_t op, const std::string& operand, 
                                    Address loc, Flags flags) {
    flags.e = 0;
    
    std::string target = operand;
    if (target[0] == '#') {
        flags.n = 0; flags.i = 1;
        target = target.substr(1);
    } else if (target[0] == '@') {
        flags.n = 1; flags.i = 0;
        target = target.substr(1);
    }
    
    size_t xPos = target.find(",X");
    if (xPos != std::string::npos) {
        flags.x = 1;
        target = target.substr(0, xPos);
    }
    
    Address targetAddr = 0;
    bool isImmediateValue = false;
    
    if (flags.n == 0 && flags.i == 1 && std::isdigit(target[0])) {
        targetAddr = std::stoul(target);
        isImmediateValue = true;
    } else {
        if (target[0] == '=') {
            auto lit = littab.lookup(target.substr(3, target.length() - 4));
            if (lit) targetAddr = lit->address;
        } else {
            auto sym = symtab.lookup(target);
            if (sym) {
                targetAddr = sym->value + blockManager.getBlockStart(sym->blockNumber);
            }
        }
    }
    
    int displacement = 0;
    
    if (isImmediateValue && targetAddr <= 4095) {
        displacement = targetAddr;
        flags.n = 0; flags.p = 0; flags.b = 0;
    } else {
        // FIX: Cast to signed 32-bit integer to prevent catastrophic unsigned underflow
        int32_t pc = static_cast<int32_t>(loc) + 3;
        int32_t disp = static_cast<int32_t>(targetAddr) - pc;
        if (disp >= -2048 && disp <= 2047) {
            displacement = disp & 0xFFF;
            flags.p = 1;
        } else if (baseSet) {
            disp = static_cast<int32_t>(targetAddr) - static_cast<int32_t>(baseRegister);
            if (disp >= 0 && disp <= 4095) {
                displacement = disp;
                flags.b = 1;
            } else {
                errorHandler.error(loc, "Displacement out of range for " + operand);
            }
        } else {
            errorHandler.error(loc, "Cannot address " + operand + " without BASE");
        }
    }
    
    uint8_t byte1 = (op & 0xFC) | ((flags.n << 1) | flags.i);
    uint8_t byte2 = (flags.x << 7) | (flags.b << 6) | (flags.p << 5) | (flags.e << 4) | 
                    ((displacement >> 8) & 0x0F);
    uint8_t byte3 = displacement & 0xFF;
    
    return {byte1, byte2, byte3};
}

std::vector<Byte> Pass2::genFormat4(uint8_t op, const std::string& operand, 
                                    Address loc, Flags flags) {
    flags.e = 1;
    
    std::string target = operand;
    if (target[0] == '#') { flags.n = 0; flags.i = 1; target = target.substr(1); } 
    else if (target[0] == '@') { flags.n = 1; flags.i = 0; target = target.substr(1); }
    
    size_t xPos = target.find(",X");
    if (xPos != std::string::npos) { flags.x = 1; target = target.substr(0, xPos); }
    
    Address targetAddr = 0;
    if (flags.n == 0 && flags.i == 1 && std::isdigit(target[0])) {
        targetAddr = std::stoul(target);
    } else {
        if (target[0] == '=') {
            auto lit = littab.lookup(target.substr(3, target.length() - 4));
            if (lit) targetAddr = lit->address;
        } else {
            auto sym = symtab.lookup(target);
            if (sym) targetAddr = sym->value + blockManager.getBlockStart(sym->blockNumber);
        }
    }
    
    uint8_t byte1 = (op & 0xFC) | ((flags.n << 1) | flags.i);
    uint8_t byte2 = (flags.x << 7) | (flags.b << 6) | (flags.p << 5) | (flags.e << 4) | ((targetAddr >> 16) & 0x0F);
    uint8_t byte3 = (targetAddr >> 8) & 0xFF;
    uint8_t byte4 = targetAddr & 0xFF;
    
    return {byte1, byte2, byte3, byte4};
}

} // namespace sicxe