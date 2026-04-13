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

// --- Boost Injection ---
#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace sicxe;

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string outputFile;
    bool verbose = false;

    // Define supported options using Boost.Program_options
    po::options_description desc("SIC/XE Assembler Options");
    desc.add_options()
        ("help,h", "Produce help message")
        ("input,i", po::value<std::string>(&inputFile), "Source assembly file (.asm)")
        ("output,o", po::value<std::string>(&outputFile)->default_value("output.obj"), "Output object file (.obj)")
        ("verbose,v", po::bool_switch(&verbose), "Enable verbose diagnostic output");

    // Positional arguments (allows: ./sicxe_assembler input.asm output.obj)
    po::positional_options_description p;
    p.add("input", 1);
    p.add("output", 1);

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv).options(desc).positional(p).run(), vm);
        po::notify(vm);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    // Handle Help
    if (vm.count("help") || inputFile.empty()) {
        std::cout << "=== SIC/XE Assembler (Boost Edition) ===" << std::endl;
        std::cout << desc << std::endl;
        return 0;
    }

    // Read source file
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

    // Initialize components
    SymbolTable symtab;
    LiteralTable littab;
    ProgramBlockManager blockManager;
    ErrorHandler errorHandler;
    ObjectWriter objWriter;

    if (verbose) std::cout << "[INFO] Starting Pass 1: Symbol Definition..." << std::endl;
    
    // Pass 1
    Pass1 pass1(symtab, littab, blockManager, errorHandler);
    if (!pass1.process(lines)) {
        std::cerr << "[FAIL] Pass 1 failed." << std::endl;
        errorHandler.printReport();
        return 1;
    }
    
    std::cout << "Pass 1 complete. Program length: " 
              << std::hex << std::uppercase << pass1.getProgramLength() << " bytes." << std::endl;
    
    // Reset for Pass 2
    blockManager = ProgramBlockManager(); 
    errorHandler.clear();
    
    if (verbose) std::cout << "[INFO] Starting Pass 2: Code Generation..." << std::endl;
    
    // Pass 2
    Pass2 pass2(symtab, littab, blockManager, objWriter, errorHandler);
    if (!pass2.process(lines)) {
        std::cerr << "[FAIL] Pass 2 failed." << std::endl;
        errorHandler.printReport();
        return 1;
    }
    
    // Write output
    objWriter.writeToFile(outputFile);
    
    // Write listing file automatically
    std::string listingFile = outputFile.substr(0, outputFile.find_last_of('.')) + ".lst";
    std::ofstream listOut(listingFile);
    listOut << objWriter.getListing();
    listOut.close();
    
    std::cout << "Assembly successful." << std::endl;
    std::cout << "Object code: " << outputFile << std::endl;
    std::cout << "Listing:     " << listingFile << std::endl;
    
    if (verbose) errorHandler.printReport();
    return 0;
}