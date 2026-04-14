#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>
#include <string>
#include <variant>
#include <optional>
#include <vector>
#include <sstream>
#include <iomanip>

namespace sicxe {

using Address = uint32_t;
using Byte = uint8_t;
using HalfWord = uint16_t;
using Word = uint32_t;

enum class InstructionFormat {
    FORMAT_1,   // 1 byte: opcode(8)
    FORMAT_2,   // 2 bytes: opcode(8), reg1(4), reg2(4)
    FORMAT_3,   // 3 bytes: opcode(6), flags(6), disp(12)
    FORMAT_4    // 4 bytes: opcode(6), flags(6), addr(20)
};

enum class AddressingMode {
    SIMPLE,      // n=1, i=1
    IMMEDIATE,   // n=0, i=1
    INDIRECT,    // n=1, i=0
    INDEXED      // x=1
};

struct Flags {
    bool n : 1;  // Indirect
    bool i : 1;  // Immediate
    bool x : 1;  // Indexed
    bool b : 1;  // Base relative
    bool p : 1;  // PC relative
    bool e : 1;  // Extended (Format 4)
    
    Flags() : n(1), i(1), x(0), b(0), p(0), e(0) {}
    
    // Packs flags into a single byte via bit-shifting to ensure cross-platform safety
    uint8_t toByte() const {
        return (n << 5) | (i << 4) | (x << 3) | (b << 2) | (p << 1) | e;
    }
};

struct Instruction {
    std::string label;
    std::string opcode;
    std::string operand;
    std::string rawLine;
    int lineNumber;
    
    InstructionFormat format;
    Address location;
    std::vector<Byte> objectCode;
    bool isDirective;
};

struct Symbol {
    std::string name;
    Address value;
    bool isAbsolute;
    bool isExternal;
    int blockNumber;
};

struct Literal {
    std::string name;
    std::string value;
    Address address;
    int blockNumber;
    int length;
    bool isHex;  // Differentiates =X'..' from =C'..'
};

struct ProgramBlock {
    std::string name;
    Address startAddress;
    Address currentLoc;
    int blockNumber;
};

// Exception class to safely unwind the stack upon fatal compilation errors
class AssemblyError : public std::exception {
private:
    std::string message;
    int line;
public:
    AssemblyError(const std::string& msg, int ln = -1) 
        : message(msg), line(ln) {}
    
    const char* what() const noexcept override {
        return message.c_str();
    }
    
    int getLine() const { return line; }
};

// Utility functions defined inline to avoid linker ODR (One Definition Rule) violations
inline std::string toHex(uint32_t value, int width = 6) {
    std::stringstream ss;
    ss << std::uppercase << std::hex << std::setfill('0') << std::setw(width) << (value & 0xFFFFFF);
    return ss.str();
}

inline std::string bytesToHex(const std::vector<Byte>& bytes) {
    std::stringstream ss;
    for (auto b : bytes) {
        ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (int)b;
    }
    return ss.str();
}

} // namespace sicxe
#endif