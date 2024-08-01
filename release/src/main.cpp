#include <iostream>
#include <fstream>
#include "CellLibraryParser.hpp"
#include "NetlistParser.hpp"
#include "GateMapper.hpp"
#include "NetlistWriter.hpp"
#include "Optimizer.hpp"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <netlist> <cell_library> <output> <cost_estimator>" << std::endl;
        return 1;
    }

    std::string netlistFile = argv[1];
    std::string cellLibraryFile = argv[2];
    std::string outputFile = argv[3];
    std::string costEstimator = argv[4];

    // Parse the cell library
    CellLibraryParser cellLibraryParser(cellLibraryFile);
    cellLibraryParser.parse();
    const std::vector<Cell>& cells = cellLibraryParser.getCells();

    // Create the gate mapper
    GateMapper gateMapper(cells);
    std::unordered_map<std::string, std::vector<std::string>> gateMapping = gateMapper.createGateMapping();

    // Parse the netlist
    NetlistParser netlistParser(netlistFile);
    netlistParser.parse(outputFile);
    const Netlist& netlist = netlistParser.getNetlist();

    // Convert gateMapping to the required format for NetlistWriter
    std::unordered_map<std::string, std::string> gateToCellMapping;
    for (const auto& gate : netlist.gates) {
        auto it = gateMapping.find(gate.type);
        if (it != gateMapping.end() && !it->second.empty()) {
            gateToCellMapping[gate.name] = it->second[0]; // Assign the first possible cell for initial mapping
        } else {
            std::cerr << "Warning: No mapping found for gate type " << gate.type << std::endl;
        }
    }

    // Write the initial netlist with mapped gates
    NetlistWriter netlistWriter;
    netlistWriter.writeNetlist(netlist, gateToCellMapping, outputFile);

    // Optimize the netlist
    Optimizer optimizer(netlist, gateMapping, cellLibraryFile, outputFile, costEstimator);
    optimizer.optimize();

    return 0;
}
