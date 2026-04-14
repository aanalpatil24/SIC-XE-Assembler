#include "Pass1.hpp"
#include "Pass2.hpp"
#include "SymbolTable.hpp"
#include "LiteralTable.hpp"
#include "ProgramBlock.hpp"
#include "ObjectWriter.hpp"
#include "ErrorHandler.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace sicxe;

int main(int argc, char* argv[]) {
    // Basic argument parsing
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source.asm> [output.obj]" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = (argc > 2) ? argv[2] : "output.obj";
    
    std::ifstream inFile(inputFile);
    if (!inFile) {
        std::cerr << "Cannot open input file: " << inputFile << std::endl;
        return 1;
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(inFile, line)) {
        lines.push_back(line);
    }
    inFile.close();
    
    SymbolTable symtab;
    LiteralTable littab;
    ProgramBlockManager blockManager;
    ErrorHandler errorHandler;
    ObjectWriter objWriter;
    
    std::cout << "=== SIC/XE Assembler ===" << std::endl;
    std::cout << "Pass 1: Symbol Definition..." << std::endl;
    
    // Pass 1: Allocate memory and resolve labels
    Pass1 pass1(symtab, littab, blockManager, errorHandler);
    if (!pass1.process(lines)) {
        std::cerr << "Pass 1 failed." << std::endl;
        errorHandler.printReport();
        return 1;
    }
    
    std::cout << "Pass 1 complete. Program length: " 
              << std::hex << std::uppercase << pass1.getProgramLength() << std::endl;
    
    blockManager = ProgramBlockManager(); 
    errorHandler.clear();
    
    std::cout << "Pass 2: Code Generation..." << std::endl;
    
    // Pass 2: Generate final hex machine code
    Pass2 pass2(symtab, littab, blockManager, objWriter, errorHandler);
    if (!pass2.process(lines)) {
        std::cerr << "Pass 2 failed." << std::endl;
        errorHandler.printReport();
        return 1;
    }
    
    objWriter.writeToFile(outputFile);
    
    std::string listingFile = outputFile.substr(0, outputFile.find_last_of('.')) + ".lst";
    std::ofstream listOut(listingFile);
    listOut << objWriter.getListing();
    listOut.close();
    
    std::cout << "Assembly successful." << std::endl;
    std::cout << "Object code: " << outputFile << std::endl;
    std::cout << "Listing: " << listingFile << std::endl;
    
    errorHandler.printReport();
    return 0;
}