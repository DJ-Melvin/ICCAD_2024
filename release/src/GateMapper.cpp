#include "GateMapper.hpp"
#include <iostream>
#include <unordered_map>

GateMapper::GateMapper(const std::vector<Cell>& cells) : cells(cells) {}

std::unordered_map<std::string, std::vector<std::string>> GateMapper::createGateMapping() {
    std::unordered_map<std::string, std::vector<std::string>> gateMapping;
    for (const auto& cell : cells) {
        gateMapping[cell.cell_type].push_back(cell.cell_name);
    }
    return gateMapping;
}
