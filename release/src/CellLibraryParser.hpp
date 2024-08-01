#ifndef CELL_LIBRARY_PARSER_HPP
#define CELL_LIBRARY_PARSER_HPP

#include <string>
#include <vector>
#include "json.hpp"

struct Cell {
    std::string cell_name;
    std::string cell_type;
    std::vector<float> float_data;
    std::vector<int> int_data;
};

class CellLibraryParser {
public:
    CellLibraryParser(const std::string& filepath);
    void parse();
    std::vector<Cell> getCells() const;

private:
    std::vector<Cell> cells;
    std::string filepath;
};

#endif // CELL_LIBRARY_PARSER_HPP
