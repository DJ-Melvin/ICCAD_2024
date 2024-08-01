#include "NetlistWriter.hpp"
#include <fstream>
#include <iostream>
#include <set>
#include <unordered_set>

void NetlistWriter::writeNetlist(const Netlist& netlist, const std::unordered_map<std::string, std::string>& gateToCellMapping, const std::string& outputFilename) {
    std::ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilename << std::endl;
        return;
    }

    // Collect all inputs, outputs, and wires
    std::set<std::string> inputs(netlist.inputs.begin(), netlist.inputs.end());
    std::set<std::string> outputs(netlist.outputs.begin(), netlist.outputs.end());
    std::set<std::string> wires(netlist.wires.begin(), netlist.wires.end());

    // Remove inputs from wires to avoid duplication
    for (const auto& input : inputs) {
        wires.erase(input);
    }

    // Remove outputs from wires to avoid duplication
    for (const auto& output : outputs) {
        wires.erase(output);
    }

    // Ensure unique module name and avoid duplicate inputs/outputs
    std::unordered_set<std::string> uniquePorts(inputs.begin(), inputs.end());
    uniquePorts.insert(outputs.begin(), outputs.end());

    // Print the module declaration with inputs and outputs
    outFile << "module " << netlist.moduleName << " (";
    bool first = true;
    for (const auto& port : uniquePorts) {
        if (!first) outFile << ", ";
        outFile << port;
        first = false;
    }
    outFile << ");\n";

    // Print inputs
    outFile << " input ";
    first = true;
    for (const auto& input : inputs) {
        if (!first) outFile << ", ";
        outFile << input;
        first = false;
    }
    outFile << ";\n";

    // Print outputs
    outFile << " output ";
    first = true;
    for (const auto& output : outputs) {
        if (!first) outFile << ", ";
        outFile << output;
        first = false;
    }
    outFile << ";\n";

    // Print wires
    if (!wires.empty()) {
        outFile << " wire ";
        first = true;
        for (const auto& wire : wires) {
            if (!first) outFile << ", ";
            outFile << wire;
            first = false;
        }
        outFile << ";\n";
    }

    // Print gates and ensure the correct port order
    for (const auto& gate : netlist.gates) {
        auto it = gateToCellMapping.find(gate.name);
        if (it != gateToCellMapping.end()) {
            outFile << " " << it->second << " " << gate.name << " (";
            if (gate.inputs.size() == 2) {
                // For 2-input gates: (input1, input2, output)
                outFile << gate.inputs[0] << ", " << gate.inputs[1] << ", " << gate.output;
            } else if (gate.inputs.size() == 1) {
                // For 1-input gates: (input, output)
                outFile << gate.inputs[0] << ", " << gate.output;
            }
            outFile << ");\n";
        } else {
            std::cerr << "Error: Gate " << gate.name << " not found in mapping." << std::endl;
        }
    }

    outFile << "endmodule" << std::endl;
}
