#include "ObjectWriter.hpp"
#include <iostream>
#include <iomanip>

namespace sicxe {

ObjectWriter::ObjectWriter() : currentTextRecord(nullptr), startAddress(0), programLength(0) {}

void ObjectWriter::setHeader(const std::string& name, Address start, Address length) {
    programName = name;
    startAddress = start;
    programLength = length;
    while (programName.length() < 6) programName += ' ';
    if (programName.length() > 6) programName = programName.substr(0, 6);
}

void ObjectWriter::addObjectCode(Address loc, const std::vector<Byte>& code) {
    if (!currentTextRecord || !currentTextRecord->canFit(code.size())) {
        textRecords.emplace_back(loc);
        currentTextRecord = &textRecords.back();
    }
    currentTextRecord->add(code);
}

void ObjectWriter::addModificationRecord(Address loc, int halfBytes, bool external) {
    std::stringstream ss;
    ss << "M" << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << loc
       << std::setw(2) << halfBytes << (external ? "+" : "");
    modificationRecords.push_back(ss.str());
}

void ObjectWriter::addEndRecord(Address entryPoint) {
    // End record added during write
}

void ObjectWriter::addListingLine(Address loc, const std::string& source, 
                                 const std::string& objectCode, const std::string& errors) {
    listingStream << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << loc 
                  << "  " << std::left << std::setfill(' ') << std::setw(20) << objectCode 
                  << "  " << source << std::endl;
    if (!errors.empty()) {
        listingStream << "      *** " << errors << std::endl;
    }
}

void ObjectWriter::writeToFile(const std::string& filename) {
    std::ofstream out(filename);
    
    // Header Record
    out << "H" << programName 
        << std::uppercase << std::hex << std::setfill('0') << std::setw(6) << startAddress
        << std::setw(6) << programLength << std::endl;
    
    // Text Records
    for (const auto& rec : textRecords) {
        out << "T" << std::setw(6) << std::setfill('0') << rec.startAddr
            << std::setw(2) << rec.currentLength;
        for (const auto& code : rec.objectCodes) {
            for (auto b : code) {
                out << std::setw(2) << (int)b;
            }
        }
        out << std::endl;
    }
    
    // Modification Records
    for (const auto& mod : modificationRecords) {
        out << mod << std::endl;
    }
    
    // End Record
    out << "E" << std::setw(6) << std::setfill('0') << startAddress << std::endl;
    
    out.close();
}

} // namespace sicxe