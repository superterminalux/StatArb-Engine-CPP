#include <iostream>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "OLSSolver.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    if (!DataLoader::loadCSV("../data/sample_data.csv", dates, co_prices, pep_prices)) {
        return 1;
    }

    OLSResult model = OLSSolver::fit(co_prices, pep_prices);

    std::cout << "[Phase 2] OLS Regression Completed (" << dates.size() << " observations)\n";
    std::cout << "Alpha (Intercept)   : " << model.alpha << "\n";
    std::cout << "Beta (Hedge Ratio)  : " << model.beta << "\n";

    return 0;
}