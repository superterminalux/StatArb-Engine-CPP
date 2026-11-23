# StatArb-Engine-CPP

A low-latency, deterministic **Statistical Arbitrage & Pairs Trading Engine** written in modern C++17 using **Eigen** for linear algebra. Built with zero look-ahead bias, the system combines a 2-state **Kalman Filter** for dynamic hedge ratio estimation, **Engle-Granger ADF** cointegration gating, **Ornstein-Uhlenbeck** half-life adaptation, and a **Finite State Machine (FSM)** risk manager.

---

## Mathematical Architecture

### 1. Dynamic Hedge Ratio via 2-State Kalman Filter
Instead of static OLS regressions prone to regime drift, the hidden state vector $\theta_t = [\alpha_t, \beta_t]^T$ is updated recursively at $O(1)$ complexity on log-prices ($Y_t = \ln P_{Y,t}$, $X_t = \ln P_{X,t}$):

$$\hat{\theta}_{t|t-1} = \hat{\theta}_{t-1|t-1}, \quad P_{t|t-1} = P_{t-1|t-1} + Q$$

$$e_t = Y_t - H_t \hat{\theta}_{t|t-1}, \quad K_t = \frac{P_{t|t-1} H_t^T}{H_t P_{t|t-1} H_t^T + R}, \quad \hat{\theta}_{t|t} = \hat{\theta}_{t|t-1} + K_t e_t$$

### 2. Cointegration Gatekeeper & Mean-Reversion Half-Life
Before any spread entry, the rolling residual series is validated via the **Augmented Dickey-Fuller (ADF)** test and modeled as an **Ornstein-Uhlenbeck** process ($\Delta S_t = \lambda S_{t-1} + \mu + \epsilon_t$) to compute the mean-reversion half-life:

$$\text{Half-Life} = -\frac{\ln(2)}{\lambda}$$

* Trades are blocked if $t_{\text{ADF}} \ge -2.57$ or $\text{Half-Life} > 60\text{ bars}$.
* Position holding limits dynamically scale as $\text{Time-Stop} = 2.5 \times \text{Half-Life}$.

### 3. Deterministic Execution & Risk FSM
Trade lifecycle transitions (`IDLE` $\rightarrow$ `LONG_SPREAD` / `SHORT_SPREAD` $\rightarrow$ `EXIT_TAKE_PROFIT` / `EXIT_STOP_LOSS` / `EXIT_TIME_STOP` / `KILL_SWITCH_HALT`) are governed by an explicit Finite State Machine with a hard equity drawdown circuit breaker.

---

## Production Walk-Forward Benchmark (Out-of-Sample)

* **Pair / Timeframe:** `JPM / BAC` (`1H` Intraday Bars)
* **Parameters:** $\delta_{\text{Kalman}} = 10^{-6}$, $Z_{\text{entry}} = 1.50$, $Z_{\text{exit}} = 0.10$, Commission $= 2\text{ bps/leg}$, Notional Leverage $= 2.5\text{x}$

| Metric | Result |
| :--- | :--- |
| **Initial Capital** | `$10,000.00` |
| **Final Equity** | `$10,940.88` |
| **Net PnL** | `+$940.88 (+9.41%)` |
| **Max Drawdown** | `0.99%` |
| **Completed Trades (Win Rate)** | `6 (100.00%)` |
| **Signals Filtered by ADF/OU Gate** | `200` |

---

## Build & Run

### Prerequisites
* C++17 compatible compiler (GCC / Clang / MinGW)
* CMake $\ge 3.20$
* [Eigen 3.4+ / 5.0+](https://eigen.tuxfamily.org/)

### Compilation
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
./StatArb_Engine