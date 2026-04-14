#ifndef SYMTAB_HPP
#define SYMTAB_HPP

#include "Types.hpp"
#include <unordered_map>
#include <string>
#include <optional>

namespace sicxe {

class SymbolTable {
private:
    std::unordered_map<std::string, Symbol> symbols; // High speed hashmap for symbol lookups
    int currentBlock;
    
public:
    SymbolTable();
    
    void insert(const std::string& name, Address value, int block = 0, bool absolute = false);
    std::optional<Symbol> lookup(const std::string& name) const;
    bool contains(const std::string& name) const;
    void setBlock(int block) { currentBlock = block; }
    
    const auto& getAllSymbols() const { return symbols; }
    void clear() { symbols.clear(); }
};

} // namespace sicxe
#endif