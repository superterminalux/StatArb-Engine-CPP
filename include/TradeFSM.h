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
    KILL_SWITCH_HALT
};

struct RiskParams {
    double entry_z = 2.0;          // Pozisyon açılış Z-Skoru eşiği
    double exit_z = 0.2;           // Ortalamaya dönüş kâr al eşiği
    double stop_loss_z = 3.5;      // Eşbütünleşme kırılması zarar kes eşiği
    double max_drawdown_pct = 0.10;// %10 maksimum sermaye düşüş limiti
};

class TradeFSM {
public:
    explicit TradeFSM(const RiskParams& params, double initial_equity);

    // Yeni gelen Z-Skoru ve güncel bakiyeye göre durum geçişini hesaplar
    TradeAction onTick(double z_score, double current_equity);

    TradeState getState() const { return current_state_; }
    bool isHalted() const { return kill_switch_triggered_; }

    static std::string stateToString(TradeState state);
    static std::string actionToString(TradeAction action);

private:
    TradeState current_state_;
    RiskParams params_;
    double peak_equity_;
    bool kill_switch_triggered_;
};

#endif // STATARB_ENGINE_TRADEFSM_H