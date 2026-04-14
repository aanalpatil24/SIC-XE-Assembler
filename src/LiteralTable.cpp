#include "LiteralTable.hpp"
#include <algorithm>

namespace sicxe {

LiteralTable::LiteralTable() : literalCounter(0) {}

std::string LiteralTable::generateName() {
    return "=*" + std::to_string(literalCounter++);
}

int LiteralTable::calculateLength(const std::string& value, bool isHex) {
    if (isHex) {
        return (value.length() + 1) / 2; // Hex: 2 chars = 1 byte
    } else {
        return value.length(); // ASCII: 1 char = 1 byte
    }
}

size_t LiteralTable::insert(const std::string& value, bool isHex) {
    auto it = indexMap.find(value);
    if (it != indexMap.end()) return it->second; // Return existing index if literal already recorded
    
    Literal lit;
    lit.name = generateName();
    lit.value = value;
    lit.length = calculateLength(value, isHex);
    lit.isHex = isHex;
    lit.address = 0; 
    lit.blockNumber = -1; // -1 signifies unassigned memory
    
    size_t index = literals.size();
    literals.push_back(lit);
    indexMap[value] = index;
    return index;
}

std::optional<Literal> LiteralTable::lookup(const std::string& value) const {
    auto it = indexMap.find(value);
    if (it != indexMap.end()) return literals[it->second];
    return std::nullopt;
}

void LiteralTable::assignAddresses(Address startAddr, int blockNum) {
    // Sequentially assigns memory to all pending literals upon hitting an LTORG
    Address current = startAddr;
    for (auto& lit : literals) {
        if (lit.blockNumber == -1) {
            lit.address = current;
            lit.blockNumber = blockNum;
            current += lit.length;
        }
    }
}

bool LiteralTable::hasUnassigned() const {
    return std::any_of(literals.begin(), literals.end(), 
                      [](const Literal& l) { return l.blockNumber == -1; });
}

Address LiteralTable::poolEndAddress() const {
    Address maxAddr = 0;
    for (const auto& lit : literals) {
        if (lit.blockNumber != -1) {
            Address end = lit.address + lit.length;
            if (end > maxAddr) maxAddr = end;
        }
    }
    return maxAddr;
}

} // namespace sicxe