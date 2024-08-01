#ifndef GATE_MAPPER_HPP
#define GATE_MAPPER_HPP

#include <vector>
#include <unordered_map>
#include <string>
#include "CellLibraryParser.hpp"

class GateMapper {
public:
    GateMapper(const std::vector<Cell>& cells);
    std::unordered_map<std::string, std::vector<std::string>> createGateMapping();

private:
    std::vector<Cell> cells;
};

#endif // GATE_MAPPER_HPP
