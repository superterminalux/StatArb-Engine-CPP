#ifndef STATARB_ENGINE_TRADEFSM_H
#define STATARB_ENGINE_TRADEFSM_H

#include <string>

enum class TradeState {
    IDLE,
    LONG_SPREAD,
    SHORT_SPREAD
};

enum class TradeAction {
    NONE,
    ENTER_LONG_SPREAD,
    ENTER_SHORT_SPREAD,
    EXIT_TAKE_PROFIT,
    EXIT_STOP_LOSS,
    EXIT_TIME_STOP,
    KILL_SWITCH_HALT
};

struct RiskParams {
    double entry_z = 2.0;
    double exit_z = 0.2;
    double stop_loss_z = 3.5;
    double max_drawdown_pct = 0.10;
    int max_holding_days = 30;
};

class TradeFSM {
public:
    explicit TradeFSM(const RiskParams& params, double initial_equity);

    TradeAction onTick(double z_score, double current_equity);

    void setMaxHoldingDays(int days) { params_.max_holding_days = days; }

    void reset() {
        current_state_ = TradeState::IDLE;
        days_in_trade_ = 0;
    }

    TradeState getState() const { return current_state_; }
    bool isHalted() const { return kill_switch_triggered_; }

    static std::string stateToString(TradeState state);
    static std::string actionToString(TradeAction action);

private:
    TradeState current_state_;
    RiskParams params_;
    double peak_equity_;
    int days_in_trade_;
    bool kill_switch_triggered_;
};

#endif // STATARB_ENGINE_TRADEFSM_H