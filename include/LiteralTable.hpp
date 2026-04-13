#ifndef LITTAB_HPP
#define LITTAB_HPP

#include "Types.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <optional>

namespace sicxe {

class LiteralTable {
private:
    std::vector<Literal> literals;
    std::unordered_map<std::string, size_t> indexMap;
    int literalCounter;
    
    std::string generateName();
    int calculateLength(const std::string& value, bool isHex);
    
public:
    LiteralTable();
    
    // Returns index of literal
    size_t insert(const std::string& value, bool isHex);
    std::optional<Literal> lookup(const std::string& value) const;
    std::optional<Literal> lookupByAddress(Address addr) const;
    
    void assignAddresses(Address startAddr, int blockNum);
    void clearUnassigned();
    
    const auto& getAllLiterals() const { return literals; }
    bool hasUnassigned() const;
    
    Address poolEndAddress() const;
};

} // namespace sicxe

#endif