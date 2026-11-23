#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "OLSSolver.h"
#include "SignalEngine.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    if (!DataLoader::loadCSV("../data/sample_data.csv", dates, co_prices, pep_prices)) {
        return 1;
    }

    OLSResult model = OLSSolver::fit(co_prices, pep_prices);

    const size_t window = 5;
    std::vector<double> spread = SignalEngine::computeSpread(co_prices, pep_prices, model.alpha, model.beta);
    std::vector<double> z_scores = SignalEngine::computeRollingZScore(spread, window);

    std::cout << "[Phase 3] Spread & Rolling Z-Score (Window = " << window << ")\n";
    std::cout << std::fixed << std::setprecision(4);

    for (size_t i = 0; i < dates.size(); ++i) {
        std::cout << dates[i]
                  << " | Spread: " << std::setw(8) << spread[i]
                  << " | Z-Score: " << std::setw(8) << z_scores[i] << "\n";
    }

    return 0;
}