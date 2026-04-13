#ifndef OBJWRITER_HPP
#define OBJWRITER_HPP

#include "Types.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

namespace sicxe {

struct TextRecord {
    Address startAddr;
    std::vector<std::vector<Byte>> objectCodes;
    size_t currentLength;
    
    TextRecord(Address sa) : startAddr(sa), currentLength(0) {}
    
    bool canFit(size_t bytes) const {
        return (currentLength + bytes) <= 30; // Max 30 bytes per text record
    }
    
    void add(const std::vector<Byte>& code) {
        objectCodes.push_back(code);
        currentLength += code.size();
    }
};

class ObjectWriter {
private:
    std::string programName;
    Address startAddress;
    Address programLength;
    
    std::vector<TextRecord> textRecords;
    std::vector<std::string> modificationRecords;
    std::stringstream listingStream;
    
    TextRecord* currentTextRecord;
    
    void flushTextRecord();
    
public:
    ObjectWriter();
    
    void setHeader(const std::string& name, Address start, Address length);
    void addObjectCode(Address loc, const std::vector<Byte>& code);
    void addModificationRecord(Address loc, int halfBytes, bool external = false);
    void addEndRecord(Address entryPoint = 0);
    
    void writeToFile(const std::string& filename);
    std::string getListing() const { return listingStream.str(); }
    
    void addListingLine(Address loc, const std::string& source, 
                       const std::string& objectCode, const std::string& errors = "");
};

} // namespace sicxe

#endif