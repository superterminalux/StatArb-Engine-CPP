# StatArb Engine (C++)

A statistical arbitrage engine written in modern C++ — built step by step as a
learning + production-oriented project. The long-term goal is a modular pipeline
covering **cointegration testing → signal generation → FSM-based position
management → backtesting → (later) low-latency execution**.

> ⚠️ **Early-stage project.** The core OLS pipeline is still in progress.
> Nothing here is production-ready. See the status table below.

---

## Project Status

| Component                              | Status |
|----------------------------------------|:------:|
| CSV data ingestion                     |   ✅   |
| OLS regression (Eigen)                 |   ✅   |
| Cointegration test (Engle–Granger/ADF) |   ⬜   |
| Spread & Z-score computation           |   ⬜   |
| Signal generation (entry/exit)         |   ⬜   |
| FSM (IDLE / LONG / SHORT / EXIT)       |   ⬜   |
| Backtesting engine                     |   ⬜   |
| Risk management (SL/TP, sizing)        |   ⬜   |
| Low-latency execution layer            |   ⬜   |

**Legend:** ✅ Done · 🚧 In Progress · ⬜ Planned

---

## Overview

Pairs trading / statistical arbitrage relies on finding two instruments whose
prices are cointegrated. The engine will:

1. Load historical price data (CSV).
2. Estimate the hedge ratio via OLS (currently the focus).
3. Test the residual (spread) for stationarity.
4. Generate trading signals from the spread's Z-score.
5. Manage positions through a finite state machine.
6. Backtest the strategy and report performance metrics.

Only steps **1 and (part of) 2** are implemented right now.

---

## Requirements

- **C++17** compiler (GCC ≥ 9, Clang ≥ 10, MSVC ≥ 2019)
- **CMake ≥ 3.15**
- **Eigen ≥ 3.4** (header-only, must be available to CMake)

---

## Build

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run (expects sample_data.csv in the working directory)
./build/statarb