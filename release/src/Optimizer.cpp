#include "Optimizer.hpp"
#include "NetlistWriter.hpp"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <unordered_map>
#include <algorithm>
#include <cmath>
#include <chrono>

// Constructor definition
Optimizer::Optimizer(const Netlist& netlist, const std::unordered_map<std::string, std::vector<std::string>>& gateMapping,
                     const std::string& cellLibraryFile, const std::string& outputFile, const std::string& costEstimator)
    : netlist(netlist), gateMapping(gateMapping), cellLibraryFile(cellLibraryFile), outputFile(outputFile), costEstimator(costEstimator) {
    // Initialize random seed
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize the gate to cell mapping
    for (const auto& gate : netlist.gates) {
        auto it = gateMapping.find(gate.type);
        if (it != gateMapping.end() && !it->second.empty()) {
            gateToCellMapping[gate.name] = it->second[0]; // Assign the first possible cell for initial mapping
        } else {
            std::cerr << "Warning: No mapping found for gate type " << gate.type << std::endl;
        }
    }
}

// Function to generate a random neighbor with domain-specific knowledge
void Optimizer::getNeighbor(std::unordered_map<std::string, std::string>& neighborMapping) {
    neighborMapping = gateToCellMapping;
    int index = std::rand() % netlist.gates.size();
    const auto& gate = netlist.gates[index];
    auto it = gateMapping.find(gate.type);
    if (it != gateMapping.end() && !it->second.empty()) {
        const std::vector<std::string>& possibleCells = it->second;
        if (possibleCells.size() > 1) {
            std::string currentCell = neighborMapping[gate.name];
            std::string newCell;
            size_t attempts = 0;
            do {
                newCell = possibleCells[std::rand() % possibleCells.size()];
                attempts++;
            } while (newCell == currentCell && attempts < possibleCells.size());
            if (newCell != currentCell) {
                neighborMapping[gate.name] = newCell;
            }
        }
    }
}

// Function to calculate the cost of the current netlist
float Optimizer::calculateCost(const std::unordered_map<std::string, std::string>& mapping) {
    // Write the current netlist to the output file with the given mapping
    NetlistWriter netlistWriter;
    netlistWriter.writeNetlist(netlist, mapping, outputFile);
    return runCostEstimator();
}

// Enhanced Simulated Annealing function
void Optimizer::simulatedAnnealing() {
    float initialTemp = 1000.0f;
    float alpha = 0.95f;  // Slower cooling rate initially

    float currentTemp = initialTemp;

    // Initial solution
    float bestCost = calculateCost(gateToCellMapping);
    std::unordered_map<std::string, std::string> bestMapping = gateToCellMapping;

    // Write the initial best cost to the cost_output.txt file
    updateCostFile(bestCost);

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    int iteration = 0;
    int resetCounter = 0;
    
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = startTime + std::chrono::hours(3);

    while (std::chrono::steady_clock::now() < endTime) {
        iteration++;
        std::unordered_map<std::string, std::string> neighborMapping;
        getNeighbor(neighborMapping);
        float currentCost = calculateCost(gateToCellMapping);
        float neighborCost = calculateCost(neighborMapping);

        // Increase the acceptance probability for worse solutions at higher temperatures
        if (neighborCost < currentCost || std::exp((currentCost - neighborCost) / currentTemp) > (static_cast<float>(std::rand()) / RAND_MAX)) {
            gateToCellMapping = neighborMapping;
            currentCost = neighborCost;
        }

        if (currentCost < bestCost) {
            bestCost = currentCost;
            bestMapping = gateToCellMapping;
            // Update the cost_output.txt with the best cost
            updateCostFile(bestCost);
            // Save the best netlist periodically
            NetlistWriter netlistWriter;
            netlistWriter.writeNetlist(netlist, bestMapping, outputFile);
        }

        // Adjust alpha dynamically
        if (iteration % 100 == 0) {  // Output progress every 100 iterations
            std::cout << "Iteration " << iteration << ": Current cost = " << currentCost << ", Best cost = " << bestCost << std::endl;
            // Adaptive cooling: Reduce alpha if no improvement
            if (currentCost == bestCost) {
                alpha = std::max(alpha * 0.99f, 0.85f);  // Slow down cooling if stuck
            } else {
                alpha = 0.95f;  // Reset to the original cooling rate if improvement
                resetCounter = 0;
            }
        }

        // Occasionally reset the temperature to escape local minima
        if (resetCounter > 500) {
            currentTemp = initialTemp;
            resetCounter = 0;
        } else {
            resetCounter++;
        }

        currentTemp *= alpha;
    }

    // Restore the best mapping
    gateToCellMapping = bestMapping;

    // Save the final best netlist
    NetlistWriter netlistWriter;
    netlistWriter.writeNetlist(netlist, bestMapping, outputFile);
}

// Function to update the cost_output.txt file with the best cost
void Optimizer::updateCostFile(float bestCost) {
    std::ofstream costFile("cost_output.txt", std::ios::trunc);
    if (costFile.is_open()) {
        costFile << "cost =" << bestCost << std::endl;
        costFile.close();
    } else {
        std::cerr << "Error: Could not open cost_output.txt file to write best cost." << std::endl;
    }
}

float Optimizer::optimize() {
    // Redirect cout to optimizer.txt
    std::ofstream outFile("optimizer.txt");
    std::streambuf* coutbuf = std::cout.rdbuf(); // Save old buf
    std::cout.rdbuf(outFile.rdbuf()); // Redirect cout to optimizer.txt

    simulatedAnnealing();

    // Restore cout back to standard output
    std::cout.rdbuf(coutbuf);

    // Write the best solution to the output file
    NetlistWriter netlistWriter;
    netlistWriter.writeNetlist(netlist, gateToCellMapping, outputFile);

    float finalCost = calculateCost(gateToCellMapping);
    return finalCost;
}

float Optimizer::runCostEstimator() {
    std::string command = costEstimator + " -library " + cellLibraryFile + " -netlist " + outputFile + " -output temp_cost_output.txt 2>> optimizer.txt";
    std::cout << "Running command: " << command << std::endl;
    system(command.c_str());

    std::ifstream costFile("temp_cost_output.txt");
    float cost = std::numeric_limits<float>::max();
    if (costFile.is_open()) {
        std::string line;
        std::getline(costFile, line);
        try {
            size_t pos = line.find('=');
            if (pos != std::string::npos) {
                cost = std::stof(line.substr(pos + 1));
            } else {
                std::cerr << "Error: Could not find '=' in cost output." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Exception parsing cost: " << e.what() << std::endl;
        }
        costFile.close();
    } else {
        std::cerr << "Error: Could not open cost file." << std::endl;
    }

    return cost;
}

void Optimizer::adjustNetlist() {
    std::cout << "Adjusting netlist:" << std::endl;

    for (auto& gate : netlist.gates) {
        std::cout << "Processing gate " << gate.name << " of type " << gate.type << std::endl;

        // Collect possible cells of the same type
        auto it = gateMapping.find(gate.type);
        if (it != gateMapping.end() && !it->second.empty()) {
            const std::vector<std::string>& possibleCells = it->second;

            if (possibleCells.size() > 1) {
                std::string currentCell = gateToCellMapping[gate.name];
                std::string newCell;
                size_t attempts = 0;
                do {
                    newCell = possibleCells[std::rand() % possibleCells.size()];
                    attempts++;
                } while (newCell == currentCell && attempts < possibleCells.size());

                if (newCell != currentCell) {
                    std::cout << "Changing gate " << gate.name << " of type " << gate.type << " from " << currentCell << " to " << newCell << std::endl;
                    gateToCellMapping[gate.name] = newCell;
                } else {
                    std::cout << "No different cell found for gate type " << gate.type << " after " << attempts << " attempts." << std::endl;
                }
            } else {
                std::cout << "No different cell found for gate type " << gate.type << std::endl;
            }
        } else {
            std::cout << "No possible cells found for gate type " << gate.type << std::endl;
        }
    }
}
