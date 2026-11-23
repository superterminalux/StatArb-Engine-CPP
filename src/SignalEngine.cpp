#include "SignalEngine.h"
#include <cmath>
#include <stdexcept>

std::vector<double> SignalEngine::computeSpread(const std::vector<double>& x_prices,
                                                const std::vector<double>& y_prices,
                                                double alpha,
                                                double beta) {
    if (x_prices.size() != y_prices.size() || x_prices.empty()) {
        throw std::invalid_argument("[SignalEngine Error] Mismatched or empty price series.");
    }

    std::vector<double> spread(x_prices.size());
    for (size_t i = 0; i < x_prices.size(); ++i) {
        spread[i] = y_prices[i] - (beta * x_prices[i] + alpha);
    }
    return spread;
}

std::vector<double> SignalEngine::computeRollingZScore(const std::vector<double>& spread,
                                                       size_t window) {
    if (window < 2 || spread.size() < window) {
        throw std::invalid_argument("[SignalEngine Error] Invalid rolling window size.");
    }

    std::vector<double> z_scores(spread.size(), 0.0);

    for (size_t i = window - 1; i < spread.size(); ++i) {
        // 1. Pencere ortalamasını (mean) hesapla
        double sum = 0.0;
        for (size_t j = i + 1 - window; j <= i; ++j) {
            sum += spread[j];
        }
        double mean = sum / static_cast<double>(window);

        // 2. Pencere standart sapmasını (sample std dev) hesapla
        double sq_diff_sum = 0.0;
        for (size_t j = i + 1 - window; j <= i; ++j) {
            double diff = spread[j] - mean;
            sq_diff_sum += diff * diff;
        }
        double std_dev = std::sqrt(sq_diff_sum / static_cast<double>(window - 1));

        // 3. Sıfıra bölünme korumasıyla Z-Skorunu üret
        if (std_dev > 1e-12) {
            z_scores[i] = (spread[i] - mean) / std_dev;
        } else {
            z_scores[i] = 0.0;
        }
    }

    return z_scores;
}