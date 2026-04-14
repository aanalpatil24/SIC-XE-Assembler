cmake_minimum_required(VERSION 3.14)
project(SICXEAssembler VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17) # Enforces modern C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Python COMPONENTS Interpreter Development)

# Groups core logic into a static library to allow sharing between CLI and Python bindings
add_library(sicxe_core STATIC
    src/OpcodeTable.cpp
    src/SymbolTable.cpp
    src/LiteralTable.cpp
    src/ProgramBlock.cpp
    src/Pass1.cpp
    src/Pass2.cpp
    src/ObjectWriter.cpp
    src/ErrorHandler.cpp
)

target_include_directories(sicxe_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(sicxe_assembler src/main.cpp)
target_link_libraries(sicxe_assembler PRIVATE sicxe_core)

if(Python_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11
        GIT_TAG        v2.11.1
    )
    FetchContent_MakeAvailable(pybind11)
    
    # Compiles the Python shared object library (.so)
    pybind11_add_module(pysicxe bindings/sicxe_bindings.cpp)
    target_link_libraries(pysicxe PRIVATE sicxe_core)
endif()

enable_testing()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
    add_test(NAME AssemblerTests 
             COMMAND pytest ${CMAKE_CURRENT_SOURCE_DIR}/tests -v
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()