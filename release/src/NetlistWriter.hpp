#ifndef NETLIST_WRITER_HPP
#define NETLIST_WRITER_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include "NetlistParser.hpp"

class NetlistWriter {
public:
    void writeNetlist(const Netlist& netlist, const std::unordered_map<std::string, std::string>& gateToCellMapping, const std::string& outputFilename);
};

#endif // NETLIST_WRITER_HPP
