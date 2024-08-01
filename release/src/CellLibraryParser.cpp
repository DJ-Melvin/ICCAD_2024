#include "CellLibraryParser.hpp"
#include <fstream>
#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

CellLibraryParser::CellLibraryParser(const std::string& filepath) : filepath(filepath) {}

void CellLibraryParser::parse() {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open the file: " << filepath << std::endl;
        exit(1);
    }

    json j;
    file >> j;

    try {
        for (const auto& cell : j["cells"]) {
            Cell c;
            c.cell_name = cell["cell_name"].get<std::string>();
            c.cell_type = cell["cell_type"].get<std::string>();

            for (const auto& attribute : cell.items()) {
                if (attribute.key().find("_f") != std::string::npos) {
                    c.float_data.push_back(std::stof(attribute.value().get<std::string>()));
                } else if (attribute.key().find("_i") != std::string::npos) {
                    c.int_data.push_back(std::stoi(attribute.value().get<std::string>()));
                }
            }
            this->cells.push_back(c);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        exit(1);
    }

    std::cout << "Parsed " << this->cells.size() << " cells from the library." << std::endl;
}

std::vector<Cell> CellLibraryParser::getCells() const {
    return cells;
}
