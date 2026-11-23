#ifndef STATARB_ENGINE_BACKTESTER_H
#define STATARB_ENGINE_BACKTESTER_H

#include <vector>
#include <string>
#include <cstddef>
#include "TradeFSM.h"

struct BacktestConfig {
    double initial_capital = 10000.0;
    double capital_usage = 0.95;
    double leverage = 2.5;            // Volatilite hedefli pozisyon çarpanı
    double commission_rate = 0.0002;
    size_t ols_window = 200;
    double adf_critical = -2.57;
    double max_half_life = 60.0;
    bool enforce_adf = true;
    bool use_kalman = true;
    double kalman_delta = 1e-6;
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