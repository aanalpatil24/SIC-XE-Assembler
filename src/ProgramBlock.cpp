#include "ProgramBlock.hpp"

namespace sicxe {

ProgramBlockManager::ProgramBlockManager() : currentBlockIndex(0) {
    // Initialize default block to handle code outside explicit USE directives
    blocks.push_back({"DEFAULT", 0, 0, 0});
    blockIndex["DEFAULT"] = 0;
    blockIndex[""] = 0;
}

void ProgramBlockManager::setBlock(const std::string& name) {
    auto it = blockIndex.find(name);
    if (it != blockIndex.end()) {
        currentBlockIndex = it->second;
    } else {
        int newNum = blocks.size();
        blocks.push_back({name, 0, 0, newNum});
        blockIndex[name] = newNum;
        currentBlockIndex = newNum;
    }
}

int ProgramBlockManager::getCurrentBlockNumber() const { return currentBlockIndex; }

Address ProgramBlockManager::getCurrentLoc() const { return blocks[currentBlockIndex].currentLoc; }

void ProgramBlockManager::incrementLoc(int bytes) { blocks[currentBlockIndex].currentLoc += bytes; }

void ProgramBlockManager::setLoc(Address addr) { blocks[currentBlockIndex].currentLoc = addr; }

void ProgramBlockManager::organizeBlocks() {
    // Calculates absolute start addresses by stacking blocks sequentially
    Address current = 0;
    for (auto& block : blocks) {
        block.startAddress = current;
        current += block.currentLoc;
    }
}

Address ProgramBlockManager::getTotalLength() const {
    Address total = 0;
    for (const auto& block : blocks) total += block.currentLoc;
    return total;
}

} // namespace sicxe