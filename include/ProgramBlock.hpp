#ifndef PROGBLOCK_HPP
#define PROGBLOCK_HPP

#include "Types.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace sicxe {

class ProgramBlockManager {
private:
    std::vector<ProgramBlock> blocks;
    std::unordered_map<std::string, int> blockIndex;
    int currentBlockIndex;
    
public:
    ProgramBlockManager();
    
    void setBlock(const std::string& name); // Handles USE directive to switch active blocks
    int getCurrentBlockNumber() const;
    Address getCurrentLoc() const;
    void incrementLoc(int bytes);
    void setLoc(Address addr);
    
    Address getBlockStart(int blockNum) const;
    Address getTotalLength() const;
    void organizeBlocks();  // Condenses parallel blocks into a continuous linear address space
    
    const auto& getBlocks() const { return blocks; }
};

} // namespace sicxe
#endif