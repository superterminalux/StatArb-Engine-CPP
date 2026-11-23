#ifndef STATARB_ENGINE_BACKTESTER_H
#define STATARB_ENGINE_BACKTESTER_H

#include <vector>
#include <string>
#include "TradeFSM.h"

struct BacktestConfig {
    double initial_capital = 10000.0; // Başlangıç sermayesi ($)
    double capital_usage = 0.95;      // İşlem başına kullanılacak kasa oranı (%95)
    double commission_rate = 0.0005;  // Bacak başına işlem komisyonu (%0.05)
    size_t warmup_window = 5;         // Z-Skoru ısınma penceresi
};

struct BacktestReport {
    double initial_capital;
    double final_equity;
    double total_pnl;
    double return_pct;
    double max_drawdown_pct;
    int total_trades;
    int winning_trades;
};

class Backtester {
public:
    static BacktestReport run(const std::vector<std::string>& dates,
                              const std::vector<double>& x_prices,
                              const std::vector<double>& y_prices,
                              const std::vector<double>& z_scores,
                              double beta,
                              const RiskParams& risk_params,
                              const BacktestConfig& config);
};

#endif // STATARB_ENGINE_BACKTESTER_H