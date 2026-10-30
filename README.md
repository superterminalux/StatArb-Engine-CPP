# StatArb-Engine-CPP

📈 **A Statistical Arbitrage and Pairs Trading Engine**

This project is a high-performance quantitative trading engine built in modern C++. It utilizes statistical arbitrage (pairs trading) principles, moving away from discretionary trading towards data-driven, deterministic execution algorithms.

## 🚀 Core Concept

The engine operates on the principle of **Cointegration**. Instead of relying on simple price differences (spreads) or moving averages, it identifies the mathematical relationship between two historically correlated assets (e.g., two technology or energy stocks).

This relationship is calculated using **Ordinary Least Squares (OLS)** linear regression via matrix operations:

$$ \hat{\beta} = (X^T X)^{-1} X^T Y $$

When the current price relationship deviates significantly from this calculated equilibrium (the statistical baseline), the engine identifies an arbitrage opportunity, aiming to capitalize on the "Mean Reversion" principle.

## 🛠️ Technical Architecture

*   **Language:** C++ (Standard 17/20) for low-latency execution and deterministic memory management.
*   **Mathematical Core:** The [Eigen](https://eigen.tuxfamily.org/) library is used for high-speed linear algebra computations, specifically matrix multiplication and inversion required for OLS calculations.
*   **Decision Logic:** Trading signals and order execution logic are structured using a **Finite State Machine (FSM)**. This ensures robust transitions between states like `IDLE`, `LONG_POSITION`, `TAKE_PROFIT`, or `STOP_LOSS`, preventing algorithmic collapse under complex conditions.
*   **Build System:** CMake is utilized for cross-platform compilation and dependency management.

## ⚙️ Build and Installation

### Prerequisites
*   A modern C++ Compiler (GCC, Clang, or MSVC)
*   [CMake](https://cmake.org/) (version 3.20 or higher)
*   [Eigen](https://eigen.tuxfamily.org/) library (header-only)

### Setup Instructions

1.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/yourusername/StatArb-Engine-CPP.git](https://github.com/yourusername/StatArb-Engine-CPP.git)
    cd StatArb-Engine-CPP
    ```

2.  **Configure Eigen:**
    Ensure you have downloaded the Eigen library. Update the `CMakeLists.txt` file to point to your local Eigen extraction directory:
    ```cmake
    include_directories("PATH/TO/YOUR/EIGEN/DIRECTORY")
    ```

3.  **Build the Project:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

## 📊 Future Roadmap / Forward Testing

*   **Phase 1:** Core mathematical engine implementation (OLS and Cointegration calculation).
*   **Phase 2:** FSM logic integration and historical data (CSV) backtesting.
*   **Phase 3 (Live Simulation):** Integration with broker APIs (e.g., Alpaca via REST/WebSockets) for paper trading and real-time execution.
*   **Phase 4 (Performance Logging):** Continuous forward testing with automated weekly PnL and $\beta$ coefficient reporting.

---
*Developed as a practical exploration of Quantitative Finance, Linear Algebra, and Systems Programming.*
