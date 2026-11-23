#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "OLSSolver.h"
#include "SignalEngine.h"
#include "TradeFSM.h"

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

    // Test için entry_z = 1.0 (10 günlük örnek veride sinyal tetiklenmesini görmek için)
    RiskParams params{1.0, 0.2, 3.0, 0.10};
    double virtual_equity = 10000.0;
    TradeFSM fsm(params, virtual_equity);

    std::cout << "[Phase 4] Finite State Machine Execution Log\n";
    std::cout << std::fixed << std::setprecision(4);

    for (size_t i = 0; i < dates.size(); ++i) {
        // Isınma periyodunda (ilk window - 1 gün) işlem yapma
        if (i < window - 1) {
            std::cout << dates[i] << " | WARMUP | State: IDLE\n";
            continue;
        }

        TradeAction action = fsm.onTick(z_scores[i], virtual_equity);
        std::cout << dates[i]
                  << " | Z: " << std::setw(7) << z_scores[i]
                  << " | State: " << std::setw(12) << TradeFSM::stateToString(fsm.getState())
                  << " | Action: " << TradeFSM::actionToString(action) << "\n";
    }

    return 0;
}