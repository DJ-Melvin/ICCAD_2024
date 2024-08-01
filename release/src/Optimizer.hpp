#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "NetlistParser.hpp"
#include <string>
#include <unordered_map>

class Optimizer {
public:
    Optimizer(const Netlist& netlist, const std::unordered_map<std::string, std::vector<std::string>>& gateMapping,
              const std::string& cellLibraryFile, const std::string& outputFile, const std::string& costEstimator);

    float optimize();

private:
    const Netlist& netlist;
    std::unordered_map<std::string, std::vector<std::string>> gateMapping;
    std::unordered_map<std::string, std::string> gateToCellMapping;
    std::string cellLibraryFile;
    std::string outputFile;
    std::string costEstimator;

    float runCostEstimator();
    void adjustNetlist();
    void updateCostFile(float bestCost);
    void getNeighbor(std::unordered_map<std::string, std::string>& neighborMapping);
    float calculateCost(const std::unordered_map<std::string, std::string>& mapping);
    void simulatedAnnealing();
};

#endif // OPTIMIZER_HPP
