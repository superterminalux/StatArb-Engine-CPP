#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

#include "DataLoader.h"
#include "OLSSolver.h"
#include "SignalEngine.h"
#include "CointegrationTester.h"
#include "TradeFSM.h"
#include "Backtester.h"

int main() {
    std::vector<std::string> dates;
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    // 1. VERİ YÜKLEME (2 yıllık veri yoksa test için "../data/sample_data.csv" yazabilirsin)
    const std::string data_path = "../data/ko_pep_2y.csv";
    if (!DataLoader::loadCSV(data_path, dates, co_prices, pep_prices)) {
        std::cerr << "[Main] Veri dosyasi okunamadi. Yol: " << data_path << "\n";
        return 1;
    }

    std::cout << "========================================\n";
    std::cout << "         STATARB ENGINE PIPELINE        \n";
    std::cout << "========================================\n";
    std::cout << "Dataset Range     : " << dates.front() << " -> " << dates.back()
              << " (" << dates.size() << " days)\n";

    // 2. OLS REGRESYONU (Alpha & Beta Hesabı)
    OLSResult model = OLSSolver::fit(co_prices, pep_prices);
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "OLS Alpha / Beta  : " << model.alpha << " / " << model.beta << "\n\n";

    // 3. SPREAD HESAPLAMA VE EŞBÜTÜNLEŞME KAPISI (ADF Testi)
    std::vector<double> spread = SignalEngine::computeSpread(co_prices, pep_prices, model.alpha, model.beta);
    ADFResult adf = CointegrationTester::testADF(spread, 1);

    std::cout << "--- Cointegration Gatekeeper (ADF) ---\n";
    std::cout << "ADF t-Statistic   : " << adf.t_stat << "\n";
    std::cout << "5% Critical Value : " << adf.critical_5pct << "\n";
    std::cout << "Result            : " << (adf.is_cointegrated ? "PASS (Stationary)" : "FAIL (Non-Stationary)") << "\n\n";

    // Eşbütünleşme yoksa gerçek piyasada işlem açılmaz; uyarı verip durdurabiliriz
    if (!adf.is_cointegrated) {
        std::cout << "[Warning] Seri %5 seviyesinde esbutunlesik degil. Simulasyon yine de test amacli surduruluyor...\n\n";
    }

    // 4. HAREKETLİ Z-SKORU VE BACKTEST (FSM Entegreli)
    BacktestConfig config;
    config.initial_capital = 10000.0;
    config.warmup_window = (dates.size() >= 20) ? 20 : 5; // Küçük veri setlerinde otomatik uyum

    std::vector<double> z_scores = SignalEngine::computeRollingZScore(spread, config.warmup_window);

    // Veri küçükse test sinyali görebilmek için 1.0, gerçek 2 yıllık veride standart 2.0 eşiği
    double entry_threshold = (dates.size() >= 20) ? 2.0 : 1.0;
    RiskParams risk{entry_threshold, 0.2, 3.5, 0.10};

    BacktestReport report = Backtester::run(dates, co_prices, pep_prices, z_scores, model.beta, risk, config);

    // 5. NİHAİ PERFORMANS ÖZETİ
    std::cout << "\n========================================\n";
    std::cout << "           BACKTEST SUMMARY             \n";
    std::cout << "========================================\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Initial Capital   : $" << report.initial_capital << "\n";
    std::cout << "Final Equity      : $" << report.final_equity << "\n";
    std::cout << "Net PnL           : $" << report.total_pnl << " (" << report.return_pct << "%)\n";
    std::cout << "Max Drawdown      : " << report.max_drawdown_pct << "%\n";
    std::cout << "Completed Trades  : " << report.total_trades
              << " (Wins: " << report.winning_trades << ")\n";
    std::cout << "========================================\n";

    return 0;
}