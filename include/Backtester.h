#ifndef STATARB_ENGINE_BACKTESTER_H
#define STATARB_ENGINE_BACKTESTER_H

#include <vector>
#include <string>
#include <cstddef>
#include "TradeFSM.h"

struct BacktestConfig {
    double initial_capital = 10000.0;
    double capital_usage = 0.95;
    double commission_rate = 0.0005;
    size_t ols_window = 120;
    double adf_critical = -3.04;
    double max_half_life = 20.0;
    bool enforce_adf = true;
    bool verbose = false;
};

struct BacktestReport {
    double initial_capital;
    double final_equity;
    double total_pnl;
    double return_pct;
    double max_drawdown_pct;
    int total_trades;
    int winning_trades;
    int blocked_signals;
};

class Backtester {
public:
    static BacktestReport runWalkForward(const std::vector<std::string>& dates,
                                         const std::vector<double>& x_prices,
                                         const std::vector<double>& y_prices,
                                         const RiskParams& risk_params,
                                         const BacktestConfig& config);
};

#endif // STATARB_ENGINE_BACKTESTER_H