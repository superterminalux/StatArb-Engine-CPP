#include <iostream>
#include <vector>
#include <string>
#include "DataLoader.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    if (!DataLoader::loadCSV("../data/sample_data.csv", dates, co_prices, pep_prices)) {
        return 1;
    }

    std::cout << "[Phase 1] Loaded " << dates.size() << " rows.\n";
    for (size_t i = 0; i < dates.size(); ++i) {
        std::cout << dates[i] << " -> CO: " << co_prices[i]
                  << " | PEP: " << pep_prices[i] << "\n";
    }

    return 0;
}