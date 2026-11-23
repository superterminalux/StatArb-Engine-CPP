//
// Created by ertugrul on 23.11.2026.
//

#include "Backtester.h"
#include "Backtester.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

BacktestReport Backtester::run(const std::vector<std::string>& dates,
                               const std::vector<double>& x_prices,
                               const std::vector<double>& y_prices,
                               const std::vector<double>& z_scores,
                               double beta,
                               const RiskParams& risk_params,
                               const BacktestConfig& config) {
    TradeFSM fsm(risk_params, config.initial_capital);

    double cash = config.initial_capital;
    double peak_equity = config.initial_capital;
    double max_drawdown = 0.0;

    // Açık pozisyon takip değişkenleri
    double qty_x = 0.0;
    double qty_y = 0.0;
    double entry_x = 0.0;
    double entry_y = 0.0;

    int total_trades = 0;
    int winning_trades = 0;

    std::cout << "--- Backtest Execution Ledger ---\n";
    std::cout << std::fixed << std::setprecision(2);

    for (size_t i = config.warmup_window - 1; i < dates.size(); ++i) {
        // 1. Açık pozisyon varsa anlık gerçekleşmemiş kâr/zararı (Mark-to-Market) hesapla
        double unrealized_pnl = 0.0;
        if (qty_y != 0.0) {
            unrealized_pnl = qty_y * (y_prices[i] - entry_y) + qty_x * (x_prices[i] - entry_x);
        }
        double current_equity = cash + unrealized_pnl;

        // 2. Zirve sermaye ve Max Drawdown güncellemesi
        peak_equity = std::max(peak_equity, current_equity);
        double dd = (peak_equity - current_equity) / peak_equity;
        max_drawdown = std::max(max_drawdown, dd);

        // 3. FSM üzerinden karar al
        TradeAction action = fsm.onTick(z_scores[i], current_equity);

        if (action == TradeAction::ENTER_LONG_SPREAD || action == TradeAction::ENTER_SHORT_SPREAD) {
            // Bir birim Y ve Beta birim X paketinin toplam maliyeti
            double unit_basket_cost = y_prices[i] + std::abs(beta) * x_prices[i];
            double allocatable = cash * config.capital_usage;
            double units = allocatable / unit_basket_cost;

            entry_y = y_prices[i];
            entry_x = x_prices[i];

            if (action == TradeAction::ENTER_LONG_SPREAD) {
                qty_y = units;          // Long Y (PEP)
                qty_x = -units * beta;  // Short Beta*X (CO)
            } else {
                qty_y = -units;         // Short Y (PEP)
                qty_x = units * beta;   // Long Beta*X (CO)
            }

            double entry_fee = (std::abs(qty_y) * entry_y + std::abs(qty_x) * entry_x) * config.commission_rate;
            cash -= entry_fee;

            std::cout << dates[i] << " | OPEN  | " << TradeFSM::actionToString(action)
                      << " | Units Y: " << qty_y << ", X: " << qty_x
                      << " | Fee: -$" << entry_fee << "\n";
        }
        else if (action == TradeAction::EXIT_TAKE_PROFIT ||
                 action == TradeAction::EXIT_STOP_LOSS ||
                 action == TradeAction::KILL_SWITCH_HALT) {
            double realized_pnl = qty_y * (y_prices[i] - entry_y) + qty_x * (x_prices[i] - entry_x);
            double exit_fee = (std::abs(qty_y) * y_prices[i] + std::abs(qty_x) * x_prices[i]) * config.commission_rate;

            double net_trade_pnl = realized_pnl - exit_fee;
            cash += net_trade_pnl;

            total_trades++;
            if (net_trade_pnl > 0.0) winning_trades++;

            std::cout << dates[i] << " | CLOSE | " << TradeFSM::actionToString(action)
                      << " | Net PnL: $" << net_trade_pnl
                      << " | Equity: $" << cash << "\n";

            qty_y = 0.0;
            qty_x = 0.0;
        }
    }

    // Simülasyon sonunda açık pozisyon kaldıysa son fiyattan değerle
    double final_unrealized = (qty_y != 0.0)
        ? (qty_y * (y_prices.back() - entry_y) + qty_x * (x_prices.back() - entry_x))
        : 0.0;
    double final_equity = cash + final_unrealized;
    double total_pnl = final_equity - config.initial_capital;
    double return_pct = (total_pnl / config.initial_capital) * 100.0;

    return BacktestReport{
        config.initial_capital,
        final_equity,
        total_pnl,
        return_pct,
        max_drawdown * 100.0,
        total_trades,
        winning_trades
    };
}