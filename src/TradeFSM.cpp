#include "TradeFSM.h"
#include <algorithm>

TradeFSM::TradeFSM(const RiskParams& params, double initial_equity)
    : current_state_(TradeState::IDLE),
      params_(params),
      peak_equity_(initial_equity),
      days_in_trade_(0),
      kill_switch_triggered_(false) {}

TradeAction TradeFSM::onTick(double z_score, double current_equity) {
    peak_equity_ = std::max(peak_equity_, current_equity);
    double drawdown = (peak_equity_ - current_equity) / peak_equity_;

    if (drawdown >= params_.max_drawdown_pct) {
        kill_switch_triggered_ = true;
        if (current_state_ != TradeState::IDLE) {
            reset();
            return TradeAction::KILL_SWITCH_HALT;
        }
        return TradeAction::NONE;
    }

    if (kill_switch_triggered_) {
        return TradeAction::NONE;
    }

    switch (current_state_) {
        case TradeState::IDLE:
            days_in_trade_ = 0;
            if (z_score < -params_.entry_z) {
                current_state_ = TradeState::LONG_SPREAD;
                days_in_trade_ = 1;
                return TradeAction::ENTER_LONG_SPREAD;
            }
            if (z_score > params_.entry_z) {
                current_state_ = TradeState::SHORT_SPREAD;
                days_in_trade_ = 1;
                return TradeAction::ENTER_SHORT_SPREAD;
            }
            break;

        case TradeState::LONG_SPREAD:
            days_in_trade_++;
            if (z_score < -params_.stop_loss_z) {
                reset();
                return TradeAction::EXIT_STOP_LOSS;
            }
            if (z_score >= -params_.exit_z) {
                reset();
                return TradeAction::EXIT_TAKE_PROFIT;
            }
            if (days_in_trade_ > params_.max_holding_days) {
                reset();
                return TradeAction::EXIT_TIME_STOP;
            }
            break;

        case TradeState::SHORT_SPREAD:
            days_in_trade_++;
            if (z_score > params_.stop_loss_z) {
                reset();
                return TradeAction::EXIT_STOP_LOSS;
            }
            if (z_score <= params_.exit_z) {
                reset();
                return TradeAction::EXIT_TAKE_PROFIT;
            }
            if (days_in_trade_ > params_.max_holding_days) {
                reset();
                return TradeAction::EXIT_TIME_STOP;
            }
            break;
    }

    return TradeAction::NONE;
}

std::string TradeFSM::stateToString(TradeState state) {
    switch (state) {
        case TradeState::IDLE:         return "IDLE";
        case TradeState::LONG_SPREAD:  return "LONG_SPREAD";
        case TradeState::SHORT_SPREAD: return "SHORT_SPREAD";
    }
    return "UNKNOWN";
}

std::string TradeFSM::actionToString(TradeAction action) {
    switch (action) {
        case TradeAction::NONE:               return "HOLD / NO_ACTION";
        case TradeAction::ENTER_LONG_SPREAD:  return "ENTER_LONG";
        case TradeAction::ENTER_SHORT_SPREAD: return "ENTER_SHORT";
        case TradeAction::EXIT_TAKE_PROFIT:   return "EXIT (Take Profit)";
        case TradeAction::EXIT_STOP_LOSS:     return "EXIT (Stop Loss)";
        case TradeAction::EXIT_TIME_STOP:     return "EXIT (Time Stop)";
        case TradeAction::KILL_SWITCH_HALT:   return "HALT (Max Drawdown)";
    }
    return "UNKNOWN";
}