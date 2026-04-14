#include <pybind11/pybind11.h>
#include <pybind11/stl.h> // Auto-converts C++ std::vector to Python lists
#include <pybind11/functional.h>
#include "Types.hpp"
#include "SymbolTable.hpp"
#include "Pass1.hpp"
#include "Pass2.hpp"
#include "ObjectWriter.hpp"

namespace py = pybind11;
using namespace sicxe;

PYBIND11_MODULE(pysicxe, m) {
    m.doc() = "SIC/XE Assembler Python bindings";
    
    // Bind C++ Structs as read-only Python classes
    py::class_<Symbol>(m, "Symbol")
        .def_readonly("name", &Symbol::name)
        .def_readonly("value", &Symbol::value)
        .def_readonly("isAbsolute", &Symbol::isAbsolute)
        .def_readonly("blockNumber", &Symbol::blockNumber);
    
    py::class_<Literal>(m, "Literal")
        .def_readonly("name", &Literal::name)
        .def_readonly("value", &Literal::value)
        .def_readonly("address", &Literal::address)
        .def_readonly("length", &Literal::length);
    
    py::class_<Diagnostic>(m, "Diagnostic")
        .def("toString", &Diagnostic::toString)
        .def_readonly("line", &Diagnostic::line)
        .def_readonly("message", &Diagnostic::message);
    
    // Expose core Tables and Handlers
    py::class_<SymbolTable>(m, "SymbolTable")
        .def(py::init<>())
        .def("insert", &SymbolTable::insert)
        .def("lookup", &SymbolTable::lookup)
        .def("contains", &SymbolTable::contains);
    
    py::class_<LiteralTable>(m, "LiteralTable")
        .def(py::init<>())
        .def("insert", &LiteralTable::insert)
        .def("lookup", &LiteralTable::lookup)
        .def("getAllLiterals", &LiteralTable::getAllLiterals);
    
    py::class_<ErrorHandler>(m, "ErrorHandler")
        .def(py::init<>())
        .def("hasErrors", &ErrorHandler::hasErrors)
        .def("getErrorCount", &ErrorHandler::getErrorCount)
        .def("getDiagnostics", &ErrorHandler::getDiagnostics)
        .def("error", &ErrorHandler::error)
        .def("warning", &ErrorHandler::warning);
    
    py::class_<ObjectWriter>(m, "ObjectWriter")
        .def(py::init<>())
        .def("setHeader", &ObjectWriter::setHeader)
        .def("addObjectCode", &ObjectWriter::addObjectCode)
        .def("writeToFile", &ObjectWriter::writeToFile)
        .def("getListing", &ObjectWriter::getListing);
    
    m.def("toHex", &toHex, "Convert value to hex string");
    m.def("bytesToHex", &bytesToHex, "Convert byte vector to hex string");
}