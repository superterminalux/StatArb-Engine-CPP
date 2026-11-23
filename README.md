StatArb-Engine-CPP
A modular statistical arbitrage (pairs trading) research and backtesting engine written in modern C++17.
Built from scratch as an undergraduate engineering project, this repository bridges academic coursework (Linear Algebra, Probability & Statistics, and Discrete Mathematics) with clean, deterministic systems programming. Rather than chasing unrealistic ultra-low-latency HFT claims, the engine focuses on mathematical rigor, multi-timeframe historical data analysis (daily & hourly bars), and out-of-sample validation.
> 🚧 **Project Status: Starting From Scratch (Phase 0)**
>
> The repository is currently being restructured from the ground up with a clean, multi-timeframe architecture. No modules are marked complete yet.
🎯 Core Objectives & Realistic Scope
Multi-Timeframe Data Pipeline: Ingest, align, and validate historical price series across different frequencies—ranging from daily closes (`1D`) to intraday hourly bars (`1H`)—across multiple asset pairs (e.g., `KO/PEP`, `V/MA`, `XOM/CVX`).
Econometric Modeling: Estimate hedge ratios ($\beta$) via Ordinary Least Squares (OLS) using numerically stable matrix decompositions, and test spread residuals for stationarity (Engle–Granger / ADF).
Deterministic State Management: Replace fragile nested conditionals with a formal Finite State Machine (FSM) (`IDLE`, `LONG_SPREAD`, `SHORT_SPREAD`, `EXIT`) grounded in discrete mathematics.
Realistic Backtesting: Simulate historical performance with explicit modeling of transaction costs (commissions), bid-ask spread / slippage, and rolling lookback windows to prevent look-ahead bias.
Out-of-Sample Forward Testing: Track weekly paper-trading signals and simulated portfolio equity on unseen market data to guard against overfitting.
📊 Development Roadmap
Phase	Component	Description	Status
0	Project Scaffolding	CMake setup, Eigen3 integration, modular directory layout	🚧 In Progress
1	Multi-Freq CSV Ingestion	Timestamp alignment and parsing for `1D` / `1H` price series	⬜ Planned
2	OLS Regression Core	Hedge ratio ($\alpha, \beta$) estimation via Eigen QR/SVD solvers	⬜ Planned
3	Stationarity & Spread	Residual spread series, rolling mean/std, Z-score & ADF check	⬜ Planned
4	Signal & FSM Engine	Entry/exit thresholds and deterministic state transitions	⬜ Planned
5	Backtest & Risk Engine	PnL accounting, commission/slippage, max drawdown & Sharpe ratio	⬜ Planned
6	Forward Test Logging	Automated weekly signal generation and equity curve reporting	⬜ Planned
Legend: ✅ Done · 🚧 In Progress · ⬜ Planned
🧮 Mathematical Formulation
Given two historically cointegrated asset price series $X_t$ and $Y_t$ over a lookback window of $N$ bars (daily or hourly):
1. Hedge Ratio Estimation (OLS)
   We model the linear equilibrium relationship as:
   $$
   Y_t = \alpha + \beta X_t + \epsilon_t
   $$
   In matrix form $\mathbf{Y} = \mathbf{X}\hat{\boldsymbol{\beta}} + \boldsymbol{\epsilon}$, where $\mathbf{X} \in \mathbb{R}^{N \times 2}$ includes a column of ones for the intercept $\alpha$:
   $$
   \hat{\boldsymbol{\beta}} = \begin{bmatrix} \hat{\alpha} \ \hat{\beta} \end{bmatrix} = (\mathbf{X}^T \mathbf{X})^{-1} \mathbf{X}^T \mathbf{Y}
   $$
   (Implementation note: In code, we solve this via QR/SVD decomposition in Eigen rather than explicit matrix inversion to ensure numerical stability on larger intraday datasets.)
2. Spread & Rolling Z-Score
   The residual spread at time $t$ measures the deviation from statistical equilibrium:
   $$
   S_t = Y_t - (\hat{\beta} X_t + \hat{\alpha})
   $$
   To normalize the spread across varying volatility regimes, we compute the Z-score over a rolling window $W$:
   $$
   Z_t = \frac{S_t - \mu_{S, W}}{\sigma_{S, W}}
   $$
   $Z_t < -Z_{\text{entry}}$: Spread is significantly below equilibrium $\rightarrow$ Enter `LONG_SPREAD` (Long $Y$, Short $\hat{\beta} X$).
   $Z_t > +Z_{\text{entry}}$: Spread is significantly above equilibrium $\rightarrow$ Enter `SHORT_SPREAD` (Short $Y$, Long $\hat{\beta} X$).
   $\vert{}Z_t\vert{} \le Z_{\text{exit}}$: Mean reversion achieved $\rightarrow$ Transition to `EXIT` and flatten positions.
   📁 Planned Repository Structure
```
StatArb-Engine-CPP/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files (.hpp)
│   ├── DataLoader.hpp      # Multi-timeframe (1D / 1H) CSV parser & aligner
│   ├── Regression.hpp      # Eigen-based OLS & statistical utilities
│   ├── StrategyFSM.hpp     # Finite State Machine for position states
│   └── Backtester.hpp      # PnL, fee/slippage & metrics engine
├── src/                    # Implementation files (.cpp)
│   └── main.cpp            # Entry point & experiment runner
├── data/                   # Local datasets (gitignored except small samples)
│   ├── daily/              # e.g., KO_PEP_1D.csv
│   └── hourly/             # e.g., KO_PEP_1H.csv
└── README.md

```
🛠️ Requirements & Build
Prerequisites
C++17 compatible compiler (`GCC >= 9`, `Clang >= 10`, or `MSVC >= 2019`)
CMake (`>= 3.15`)
Eigen (`>= 3.4`, header-only linear algebra library)
Build Instructions
```
# 1. Clone the repository
git clone https://github.com/superterminalux/StatArb-Engine-CPP.git
cd StatArb-Engine-CPP

# 2. Configure the project (pass Eigen path if not installed system-wide)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Compile
cmake --build build --config Release

# 4. Run
./build/statarb_engine

```
⚠️ Disclaimer
This software is developed strictly for educational and quantitative research purposes. Nothing in this repository constitutes financial advice or a recommendation to trade live capital.