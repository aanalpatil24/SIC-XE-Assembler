#include "SymbolTable.hpp"

namespace sicxe {

SymbolTable::SymbolTable() : currentBlock(0) {}

void SymbolTable::insert(const std::string& name, Address value, int block, bool absolute) {
    // Saves parsed labels to resolve jump targets during Pass 2
    Symbol sym{name, value, absolute, false, block};
    symbols[name] = sym;
}

std::optional<Symbol> SymbolTable::lookup(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) return it->second;
    return std::nullopt;
}

bool SymbolTable::contains(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

} // namespace sicxe