#include "NetlistParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

// Helper function to trim whitespace from a string
std::string trim(const std::string &s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start)) {
        start++;
    }

    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

NetlistParser::NetlistParser(const std::string& filepath) : filepath(filepath) {}

const Netlist& NetlistParser::getNetlist() const {
    return netlist;
}

void NetlistParser::parse(const std::string& outputFilename) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filepath << std::endl;
        exit(1);
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open()) {
        std::cerr << "Could not open the output file: " << outputFilename << std::endl;
        exit(1);
    }

    std::string line;
    std::string collectedLine;
    bool collecting = false;
    std::string collectType;

    while (std::getline(file, line)) {
        // Remove leading and trailing whitespace
        line = trim(line);

        if (line.empty() || line[0] == '/') {
            // Skip empty lines and comments
            continue;
        }

        if (line.find("module") != std::string::npos && line.find("//") == std::string::npos) {
            collectedLine = line;
            while (std::getline(file, line)) {
                line = trim(line);
                if (line.empty() || line[0] == '/') {
                    continue; // Skip empty lines and comments
                }
                collectedLine += " " + line;
                if (line.find('(') != std::string::npos) {
                    break;
                }
            }
            parseModule(collectedLine);
        } else if (line.find("input") != std::string::npos) {
            collecting = true;
            collectType = "input";
            collectedLine = line;
            if (line.find(';') != std::string::npos) {
                parseInputOutput(collectedLine, collectType);
                collecting = false;
                collectType = "";
                collectedLine.clear();
            }
        } else if (line.find("output") != std::string::npos) {
            collecting = true;
            collectType = "output";
            collectedLine = line;
            if (line.find(';') != std::string::npos) {
                parseInputOutput(collectedLine, collectType);
                collecting = false;
                collectType = "";
                collectedLine.clear();
            }
        } else if (line.find("wire") != std::string::npos) {
            collecting = true;
            collectType = "wire";
            collectedLine = line;
            if (line.find(';') != std::string::npos) {
                parseWireLine(collectedLine);
                collecting = false;
                collectType = "";
                collectedLine.clear();
            }
        } else if (collecting && (collectType == "input" || collectType == "output")) {
            collectedLine += " " + line;
            if (line.find(';') != std::string::npos) {
                parseInputOutput(collectedLine, collectType);
                collecting = false;
                collectType = "";
                collectedLine.clear();
            }
        } else if (collecting && collectType == "wire") {
            collectedLine += " " + line;
            if (line.find(';') != std::string::npos) {
                parseWireLine(collectedLine);
                collecting = false;
                collectType = "";
                collectedLine.clear();
            }
        } else if (line.find("endmodule") != std::string::npos) {
            // Handle endmodule
            // No action needed
        } else if (line.find(";") != std::string::npos) {
            parseGate(line);
        }
    }

    file.close();

    // Write the parsed netlist to the output file
    printNetlist(outFile, netlist);
    outFile.close();
}

void NetlistParser::parseModule(const std::string& line) {
    auto start = line.find("module") + 6;
    auto end = line.find('(');
    if (end == std::string::npos) {
        // std::cerr << "Error: Invalid module line: " << line << std::endl;
        return;
    }
    std::string moduleName = trim(line.substr(start, end - start));
    netlist.moduleName = moduleName;
}

void NetlistParser::parseInputOutput(const std::string& line, const std::string& type) {
    auto start = line.find(type) + type.length();
    auto end = line.find(';', start);
    std::string ios = line.substr(start, end - start);
    std::istringstream iss(ios);
    std::string io;
    while (std::getline(iss, io, ',')) {
        io = trim(io);
        if (type == "input") {
            netlist.inputs.push_back(io);
        } else if (type == "output") {
            netlist.outputs.push_back(io);
        }
    }
}

void NetlistParser::parseWireLine(const std::string& line) {
    auto start = line.find("wire") + 4;
    auto end = line.find(';', start);
    std::string wires = line.substr(start, end - start);
    std::istringstream iss(wires);
    std::string wire;
    while (std::getline(iss, wire, ',')) {
        wire = trim(wire);
        netlist.wires.push_back(wire);
    }
}

void NetlistParser::parseGate(const std::string& line) {
    auto start = line.find(' ') + 1;
    auto end = line.find('(', start);
    if (end == std::string::npos) {
        // Invalid gate line, skip it
        return;
    }
    std::string gateType = trim(line.substr(0, start - 1));
    std::string gateName = trim(line.substr(start, end - start));

    start = end + 1;
    end = line.find(')', start);
    if (end == std::string::npos) {
        // Invalid gate line, skip it
        return;
    }
    std::string connections = line.substr(start, end - start);
    std::istringstream iss(connections);
    std::vector<std::string> connectionsList;
    std::string conn;
    while (std::getline(iss, conn, ',')) {
        conn = trim(conn);
        connectionsList.push_back(conn);
    }

    Gate gate;
    gate.type = gateType;
    gate.name = gateName;

    // Swap format to (input1, input2, output) or (input, output)
    if (connectionsList.size() > 1) {
        gate.output = connectionsList[0];
        connectionsList.erase(connectionsList.begin());
        gate.inputs = connectionsList;
    } else if (connectionsList.size() == 1) {
        gate.output = connectionsList[0];
        connectionsList.clear();
    }

    netlist.gates.push_back(gate);
}

void NetlistParser::printNetlist(std::ofstream& outFile, const Netlist& netlist) {
    outFile << "Module Name: " << netlist.moduleName << std::endl;
    outFile << "Inputs: ";
    for (const auto& input : netlist.inputs) {
        outFile << input << " ";
    }
    outFile << std::endl;
    outFile << "Outputs: ";
    for (const auto& output : netlist.outputs) {
        outFile << output << " ";
    }
    outFile << std::endl;
    outFile << "Wires: ";
    for (const auto& wire : netlist.wires) {
        outFile << wire << " ";
    }
    outFile << std::endl;
    outFile << "Gates: " << std::endl;
    for (const auto& gate : netlist.gates) {
        outFile << gate.type << " " << gate.name << " (";
        for (const auto& input : gate.inputs) {
            outFile << input << ", ";
        }
        outFile << gate.output << ");" << std::endl;
    }
}
