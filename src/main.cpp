#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "TradeFSM.h"
#include "Backtester.h"

int main() {
    std::vector<std::pair<std::string, std::string>> pairs = {
        {"KO / PEP", "../data/ko_pep_2y.csv"},
        {"V  / MA ", "../data/v_ma_2y.csv"},
        {"XOM/ CVX", "../data/xom_cvx_2y.csv"},
        {"JPM/ BAC", "../data/jpm_bac_2y.csv"},
        {"HD / LOW", "../data/hd_low_2y.csv"}
    };

    BacktestConfig config;
    config.initial_capital = 10000.0;
    config.ols_window = 120;
    config.adf_critical = -3.04;
    config.max_half_life = 20.0;
    config.enforce_adf = true;

    RiskParams risk{1.75, 0.2, 3.5, 0.10, 30};

    std::cout << "================================================================================\n";
    std::cout << "           ORNSTEIN-UHLENBECK ADAPTIVE WALK-FORWARD SCREENING                  \n";
    std::cout << "================================================================================\n";

    for (const auto& [name, path] : pairs) {
        std::vector<std::string> dates;
        std::vector<double> x_prices, y_prices;

        if (!DataLoader::loadCSV(path, dates, x_prices, y_prices)) continue;

        BacktestReport rep = Backtester::runWalkForward(dates, x_prices, y_prices, risk, config);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << name << " | Final: $" << std::setw(8) << rep.final_equity
                  << " | PnL: " << std::setw(6) << rep.return_pct << "%"
                  << " | MaxDD: " << std::setw(5) << rep.max_drawdown_pct << "%"
                  << " | Trades: " << std::setw(2) << rep.total_trades
                  << " (Win: " << rep.winning_trades << ")"
                  << " | Blocked: " << rep.blocked_signals << "\n";
    }

    return 0;
}