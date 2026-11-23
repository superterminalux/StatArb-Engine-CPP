#include "Backtester.h"
#include "OLSSolver.h"
#include "SignalEngine.h"
#include "CointegrationTester.h"
#include "KalmanFilter.h"
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

    std::vector<double> log_x(x_prices.size());
    std::vector<double> log_y(y_prices.size());
    for (size_t idx = 0; idx < x_prices.size(); ++idx) {
        log_x[idx] = std::log(x_prices[idx]);
        log_y[idx] = std::log(y_prices[idx]);
    }

    KalmanFilter kf(config.kalman_delta, 1e-3);

    // EWMA (30 bar) ile inovasyon ortalaması ve varyansı için ısınma (warmup)
    const double ewma_alpha = 2.0 / (30.0 + 1.0);
    double ewma_mean = 0.0;
    double ewma_var = 1e-6;

    for (size_t w = 0; w < config.ols_window && w < dates.size(); ++w) {
        KalmanEstimate est = kf.update(log_x[w], log_y[w]);
        if (w > 5) { // İlk 5 barlık başlangıç sıçramasını atla
            double diff = est.spread - ewma_mean;
            ewma_mean += ewma_alpha * diff;
            ewma_var = (1.0 - ewma_alpha) * (ewma_var + ewma_alpha * diff * diff);
        }
    }

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

    const double round_trip_fee_pct = 2.0 * config.commission_rate;

    for (size_t i = config.ols_window; i < dates.size(); ++i) {
        double unrealized_pnl = 0.0;
        if (qty_y != 0.0) {
            unrealized_pnl = qty_y * (y_prices[i] - entry_y) + qty_x * (x_prices[i] - entry_x);
        }
        double current_equity = cash + unrealized_pnl;

        peak_equity = std::max(peak_equity, current_equity);
        double dd = (peak_equity - current_equity) / peak_equity;
        max_drawdown = std::max(max_drawdown, dd);

        // 1. Kalman güncellemesi ve gerçek inovasyon Z-skoru (Zero Look-Ahead)
        KalmanEstimate est = kf.update(log_x[i], log_y[i]);
        double innovation_std = std::sqrt(std::max(ewma_var, 1e-12));
        double z_score = (est.spread - ewma_mean) / innovation_std;

        // EWMA istatistiklerini bir sonraki bar için güncelle
        double diff = est.spread - ewma_mean;
        ewma_mean += ewma_alpha * diff;
        ewma_var = (1.0 - ewma_alpha) * (ewma_var + ewma_alpha * diff * diff);

        // 2. Son ols_window bar üzerinden rejim (ADF & Half-Life) kontrolü
        std::vector<double> win_lx(log_x.begin() + (i - config.ols_window), log_x.begin() + i);
        std::vector<double> win_ly(log_y.begin() + (i - config.ols_window), log_y.begin() + i);
        std::vector<double> win_spread = SignalEngine::computeSpread(win_lx, win_ly, est.alpha, est.beta);

        ADFResult adf = CointegrationTester::testADF(win_spread, 1);

        // Beklenen brüt getiri gidiş-dönüş komisyonunun en az 2 katı olmalı
        double expected_move_pct = risk_params.entry_z * innovation_std;
        bool clears_fee_hurdle = expected_move_pct >= (2.0 * round_trip_fee_pct);

        bool regime_valid = (!config.enforce_adf || (adf.t_stat < config.adf_critical && adf.half_life > 2.0 && adf.half_life <= config.max_half_life))
                            && (est.beta > 0.2)
                            && clears_fee_hurdle;

        TradeAction action = fsm.onTick(z_score, current_equity);

        if (action == TradeAction::ENTER_LONG_SPREAD || action == TradeAction::ENTER_SHORT_SPREAD) {
            if (!regime_valid) {
                blocked_signals++;
                fsm.reset();
                qty_x = 0.0;
                qty_y = 0.0;
                continue;
            }

            int dynamic_time_stop = static_cast<int>(std::clamp(std::round(adf.half_life * 2.5), 12.0, 72.0));
            fsm.setMaxHoldingDays(dynamic_time_stop);

            double half_alloc = (cash * config.capital_usage * config.leverage) * 0.5;
            double units_y = half_alloc / y_prices[i];
            double units_x = (half_alloc * est.beta) / x_prices[i];

            entry_y = y_prices[i];
            entry_x = x_prices[i];

            if (action == TradeAction::ENTER_LONG_SPREAD) {
                qty_y = units_y;
                qty_x = -units_x;
            } else {
                qty_y = -units_y;
                qty_x = units_x;
            }

            double entry_fee = (std::abs(qty_y) * entry_y + std::abs(qty_x) * entry_x) * config.commission_rate;
            cash -= entry_fee;
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