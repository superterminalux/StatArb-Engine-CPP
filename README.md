# StatArb-Engine-CPP

A statistical arbitrage (pairs trading) engine written in C++17.

This is an **early-stage project (Phase 0)**. I am still setting up the
project structure, CMake, and the linear algebra layer. There is no working
strategy yet. Nothing here is ready to use.

---

## 1. What is this project?

Pairs trading looks for two stocks that usually move together
(for example `KO`/`PEP`, `V`/`MA`). When their prices move apart for a
short time, the strategy assumes they will come back together.

This engine will:

1. Read two price series from CSV files.
2. Fit a linear model (OLS) to find the hedge ratio.
3. Build a spread and a rolling Z-score.
4. Use a finite state machine (FSM) to decide when to open and close trades.
5. Record the results in a backtest log.

The goal is a **clean, deterministic, and testable** codebase, not a
production trading system.

---

## 2. Planned pipeline

```mermaid
flowchart TD
    A[CSV files] --> B[OLS: find beta]
    B --> C[Build spread]
    C --> D[Rolling Z-score]
    D --> E[FSM: open / close]
    E --> F[Backtest log]
