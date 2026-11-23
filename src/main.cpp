#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "TradeFSM.h"
#include "Backtester.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> x_prices, y_prices;

    const std::string path = "../data/jpm_bac_1h.csv";
    if (!DataLoader::loadCSV(path, dates, x_prices, y_prices)) {
        return 1;
    }

    BacktestConfig config;
    config.initial_capital = 10000.0;
    config.capital_usage   = 0.95;
    config.leverage        = 2.5;
    config.commission_rate = 0.0002;
    config.ols_window      = 200;
    config.adf_critical    = -2.57;
    config.max_half_life   = 60.0;
    config.enforce_adf     = true;
    config.use_kalman      = true;
    config.kalman_delta    = 1e-6;

    RiskParams risk{1.50, 0.10, 4.0, 0.10, 60};

    BacktestReport rep = Backtester::runWalkForward(dates, x_prices, y_prices, risk, config);

    double win_rate = (rep.total_trades > 0)
        ? (static_cast<double>(rep.winning_trades) / rep.total_trades) * 100.0
        : 0.0;

    std::cout << "========================================================\n";
    std::cout << "     STATARB ENGINE - PRODUCTION WALK-FORWARD REPORT    \n";
    std::cout << "========================================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Pair / Timeframe  : JPM / BAC (1H)\n";
    std::cout << "Dataset Range     : " << dates.front() << " -> " << dates.back() << "\n";
    std::cout << "Notional Leverage : " << config.leverage << "x\n";
    std::cout << "--------------------------------------------------------\n";
    std::cout << "Initial Capital   : $" << rep.initial_capital << "\n";
    std::cout << "Final Equity      : $" << rep.final_equity << "\n";
    std::cout << "Net PnL           : $" << rep.total_pnl << " (" << rep.return_pct << "%)\n";
    std::cout << "Max Drawdown      : " << rep.max_drawdown_pct << "%\n";
    std::cout << "Completed Trades  : " << rep.total_trades
              << " (Wins: " << rep.winning_trades << " | Win Rate: " << win_rate << "%)\n";
    std::cout << "Blocked by Gate   : " << rep.blocked_signals << " signals\n";
    std::cout << "========================================================\n";

    return 0;
}