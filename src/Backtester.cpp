#include "Backtester.h"
#include "OLSSolver.h"
#include "SignalEngine.h"
#include "CointegrationTester.h"
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

BacktestReport Backtester::runWalkForward(const std::vector<std::string>& dates,
                                         const std::vector<double>& x_prices,
                                         const std::vector<double>& y_prices,
                                         const RiskParams& risk_params,
                                         const BacktestConfig& config) {
    TradeFSM fsm(risk_params, config.initial_capital);

    double cash = config.initial_capital;
    double peak_equity = config.initial_capital;
    double max_drawdown = 0.0;

    double qty_x = 0.0;
    double qty_y = 0.0;
    double entry_x = 0.0;
    double entry_y = 0.0;

    int total_trades = 0;
    int winning_trades = 0;
    int blocked_signals = 0;

    for (size_t i = config.ols_window; i < dates.size(); ++i) {
        double unrealized_pnl = 0.0;
        if (qty_y != 0.0) {
            unrealized_pnl = qty_y * (y_prices[i] - entry_y) + qty_x * (x_prices[i] - entry_x);
        }
        double current_equity = cash + unrealized_pnl;

        peak_equity = std::max(peak_equity, current_equity);
        double dd = (peak_equity - current_equity) / peak_equity;
        max_drawdown = std::max(max_drawdown, dd);

        std::vector<double> win_x(x_prices.begin() + (i - config.ols_window), x_prices.begin() + i);
        std::vector<double> win_y(y_prices.begin() + (i - config.ols_window), y_prices.begin() + i);

        OLSResult model = OLSSolver::fit(win_x, win_y);
        std::vector<double> win_spread = SignalEngine::computeSpread(win_x, win_y, model.alpha, model.beta);

        // ADF ve Ornstein-Uhlenbeck Half-Life hesabı
        ADFResult adf = CointegrationTester::testADF(win_spread, 1);

        // Dinamik Z-Skoru penceresi: Half-Life süresine eşit (min 5, max 30 gün)
        size_t dynamic_z_win = static_cast<size_t>(std::clamp(std::round(adf.half_life), 5.0, 30.0));
        size_t z_start = win_spread.size() - dynamic_z_win;

        double sum = 0.0;
        for (size_t k = z_start; k < win_spread.size(); ++k) sum += win_spread[k];
        double mean = sum / static_cast<double>(dynamic_z_win);

        double sq_sum = 0.0;
        for (size_t k = z_start; k < win_spread.size(); ++k) {
            double diff = win_spread[k] - mean;
            sq_sum += diff * diff;
        }
        double std_dev = std::sqrt(sq_sum / static_cast<double>(dynamic_z_win - 1));

        double current_spread = y_prices[i] - (model.beta * x_prices[i] + model.alpha);
        double z_score = (std_dev > 1e-12) ? (current_spread - mean) / std_dev : 0.0;

        // Rejim geçerliliği: ADF eşiği + hızlı ortalamaya dönüş (Half-Life <= max_half_life) + pozitif Beta
        bool regime_valid = (!config.enforce_adf || (adf.t_stat < config.adf_critical && adf.half_life > 1.0 && adf.half_life <= config.max_half_life))
                            && (model.beta > 0.1);

        TradeAction action = fsm.onTick(z_score, current_equity);

        if (action == TradeAction::ENTER_LONG_SPREAD || action == TradeAction::ENTER_SHORT_SPREAD) {
            if (!regime_valid) {
                blocked_signals++;
                fsm.reset();
                qty_x = 0.0;
                qty_y = 0.0;
                continue;
            }

            // Pozisyona özel dinamik Time-Stop: 2.5 * Half-Life (min 10, max 45 gün)
            int dynamic_time_stop = static_cast<int>(std::clamp(std::round(adf.half_life * 2.5), 10.0, 45.0));
            fsm.setMaxHoldingDays(dynamic_time_stop);

            double unit_basket_cost = y_prices[i] + std::abs(model.beta) * x_prices[i];
            double allocatable = cash * config.capital_usage;
            double units = allocatable / unit_basket_cost;

            entry_y = y_prices[i];
            entry_x = x_prices[i];

            if (action == TradeAction::ENTER_LONG_SPREAD) {
                qty_y = units;
                qty_x = -units * model.beta;
            } else {
                qty_y = -units;
                qty_x = units * model.beta;
            }

            double entry_fee = (std::abs(qty_y) * entry_y + std::abs(qty_x) * entry_x) * config.commission_rate;
            cash -= entry_fee;

            if (config.verbose) {
                std::cout << dates[i] << " | OPEN  | HL: " << adf.half_life << "d | "
                          << TradeFSM::actionToString(action) << " | Fee: -$" << entry_fee << "\n";
            }
        }
        else if ((action == TradeAction::EXIT_TAKE_PROFIT ||
                  action == TradeAction::EXIT_STOP_LOSS ||
                  action == TradeAction::EXIT_TIME_STOP ||
                  action == TradeAction::KILL_SWITCH_HALT) && qty_y != 0.0) {
            double realized_pnl = qty_y * (y_prices[i] - entry_y) + qty_x * (x_prices[i] - entry_x);
            double exit_fee = (std::abs(qty_y) * y_prices[i] + std::abs(qty_x) * x_prices[i]) * config.commission_rate;

            double net_trade_pnl = realized_pnl - exit_fee;
            cash += net_trade_pnl;

            total_trades++;
            if (net_trade_pnl > 0.0) winning_trades++;

            if (config.verbose) {
                std::cout << dates[i] << " | CLOSE | " << TradeFSM::actionToString(action)
                          << " | Net PnL: $" << net_trade_pnl << " | Equity: $" << cash << "\n";
            }

            qty_y = 0.0;
            qty_x = 0.0;
        }
    }

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
        winning_trades,
        blocked_signals
    };
}