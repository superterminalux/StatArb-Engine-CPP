#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include "DataLoader.h"
#include "OLSSolver.h"
#include "SignalEngine.h"
#include "TradeFSM.h"
#include "Backtester.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    if (!DataLoader::loadCSV("../data/sample_data.csv", dates, co_prices, pep_prices)) {
        return 1;
    }

    // 1. OLS Regresyonu
    OLSResult model = OLSSolver::fit(co_prices, pep_prices);

    // 2. Spread ve Hareketli Z-Skoru
    BacktestConfig config;
    config.initial_capital = 10000.0;
    config.warmup_window = 5;

    std::vector<double> spread = SignalEngine::computeSpread(co_prices, pep_prices, model.alpha, model.beta);
    std::vector<double> z_scores = SignalEngine::computeRollingZScore(spread, config.warmup_window);

    // 3. Risk Parametreleri ve Backtest Çalıştırma
    RiskParams risk{1.0, 0.2, 3.0, 0.10};
    BacktestReport report = Backtester::run(dates, co_prices, pep_prices, z_scores, model.beta, risk, config);

    // 4. Nihai Performans Özeti
    std::cout << "\n========================================\n";
    std::cout << "       STATARB ENGINE - SUMMARY         \n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "OLS Alpha / Beta  : " << model.alpha << " / " << model.beta << "\n";
    std::cout << "Initial Capital   : $" << report.initial_capital << "\n";
    std::cout << "Final Equity      : $" << report.final_equity << "\n";
    std::cout << "Net PnL           : $" << report.total_pnl << " (" << report.return_pct << "%)\n";
    std::cout << "Max Drawdown      : " << report.max_drawdown_pct << "%\n";
    std::cout << "Completed Trades  : " << report.total_trades
              << " (Wins: " << report.winning_trades << ")\n";
    std::cout << "========================================\n";

    return 0;
}