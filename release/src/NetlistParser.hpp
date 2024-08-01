#ifndef NETLISTPARSER_HPP
#define NETLISTPARSER_HPP

#include <string>
#include <vector>
#include <fstream>

struct Gate {
    std::string type;
    std::string name;
    std::vector<std::string> inputs;
    std::string output;
};

struct Netlist {
    std::string moduleName;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<std::string> wires;
    std::vector<Gate> gates;
};

class NetlistParser {
public:
    NetlistParser(const std::string& filepath);
    void parse(const std::string& outputFilename);
    const Netlist& getNetlist() const;
    void printNetlist(std::ofstream& outFile, const Netlist& netlist);

private:
    std::string filepath;
    Netlist netlist;

    void parseModule(const std::string& line);
    void parseInputOutput(const std::string& line, const std::string& type);
    void parseWireLine(const std::string& line);
    void parseGate(const std::string& line);
};

#endif // NETLISTPARSER_HPP
