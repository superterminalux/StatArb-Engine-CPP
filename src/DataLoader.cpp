#include "DataLoader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>

bool DataLoader::loadCSV(const std::string& filepath,
                         std::vector<std::string>& dates,
                         std::vector<double>& x_prices,
                         std::vector<double>& y_prices) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[DataLoader Error] Could not open file: " << filepath << "\n";
        return false;
    }

    dates.clear();
    x_prices.clear();
    y_prices.clear();

    std::string line;
    bool is_header = true;

    while (std::getline(file, line)) {
        if (line.empty()) continue;

        if (is_header) {
            is_header = false;
            continue;
        }

        std::stringstream ss(line);
        std::string date_str, x_str, y_str;

        if (std::getline(ss, date_str, ',') &&
            std::getline(ss, x_str, ',') &&
            std::getline(ss, y_str, ',')) {
            try {
                double x_val = std::stod(x_str);
                double y_val = std::stod(y_str);

                dates.push_back(date_str);
                x_prices.push_back(x_val);
                y_prices.push_back(y_val);
            } catch (const std::exception& e) {
                std::cerr << "[DataLoader Warning] Invalid numeric row skipped: " << line << "\n";
            }
            }
    }

    file.close();
    return !x_prices.empty();
}