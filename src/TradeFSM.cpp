#include "TradeFSM.h"
#include <algorithm>

TradeFSM::TradeFSM(const RiskParams& params, double initial_equity)
    : current_state_(TradeState::IDLE),
      params_(params),
      peak_equity_(initial_equity),
      kill_switch_triggered_(false) {}

TradeAction TradeFSM::onTick(double z_score, double current_equity) {
    // 1. Zirve sermayeyi güncelle ve Max Drawdown (Devre Kesici) kontrolü yap
    peak_equity_ = std::max(peak_equity_, current_equity);
    double drawdown = (peak_equity_ - current_equity) / peak_equity_;

    if (drawdown >= params_.max_drawdown_pct) {
        kill_switch_triggered_ = true;
        if (current_state_ != TradeState::IDLE) {
            current_state_ = TradeState::IDLE;
            return TradeAction::KILL_SWITCH_HALT;
        }
        return TradeAction::NONE;
    }

    if (kill_switch_triggered_) {
        return TradeAction::NONE;
    }

    // 2. Sonlu Durum Makinesi (FSM) Geçişleri
    switch (current_state_) {
        case TradeState::IDLE:
            if (z_score < -params_.entry_z) {
                current_state_ = TradeState::LONG_SPREAD;
                return TradeAction::ENTER_LONG_SPREAD;
            }
            if (z_score > params_.entry_z) {
                current_state_ = TradeState::SHORT_SPREAD;
                return TradeAction::ENTER_SHORT_SPREAD;
            }
            break;

        case TradeState::LONG_SPREAD:
            // Spread daha da açılmaya devam ederse (yapısal kırılma) zararı kes
            if (z_score < -params_.stop_loss_z) {
                current_state_ = TradeState::IDLE;
                return TradeAction::EXIT_STOP_LOSS;
            }
            // Spread ortalamaya (-exit_z seviyesine) döndüyse kârı al
            if (z_score >= -params_.exit_z) {
                current_state_ = TradeState::IDLE;
                return TradeAction::EXIT_TAKE_PROFIT;
            }
            break;

        case TradeState::SHORT_SPREAD:
            // Spread yukarı yönlü kırılırsa zararı kes
            if (z_score > params_.stop_loss_z) {
                current_state_ = TradeState::IDLE;
                return TradeAction::EXIT_STOP_LOSS;
            }
            // Spread ortalamaya (+exit_z seviyesine) döndüyse kârı al
            if (z_score <= params_.exit_z) {
                current_state_ = TradeState::IDLE;
                return TradeAction::EXIT_TAKE_PROFIT;
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
        case TradeAction::ENTER_LONG_SPREAD:  return "ENTER_LONG (Buy PEP, Sell Beta*CO)";
        case TradeAction::ENTER_SHORT_SPREAD: return "ENTER_SHORT (Sell PEP, Buy Beta*CO)";
        case TradeAction::EXIT_TAKE_PROFIT:   return "EXIT (Take Profit)";
        case TradeAction::EXIT_STOP_LOSS:     return "EXIT (Stop Loss)";
        case TradeAction::KILL_SWITCH_HALT:   return "HALT (Max Drawdown Breached)";
    }
    return "UNKNOWN";
}