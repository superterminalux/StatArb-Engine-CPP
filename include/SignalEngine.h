#ifndef STATARB_ENGINE_SIGNALENGINE_H
#define STATARB_ENGINE_SIGNALENGINE_H

#include <vector>
#include <cstddef>

class SignalEngine {
public:
    // Spread_t = Y_t - (beta * X_t + alpha) serisini hesaplar
    static std::vector<double> computeSpread(const std::vector<double>& x_prices,
                                             const std::vector<double>& y_prices,
                                             double alpha,
                                             double beta);

    // Belirtilen pencere (window) boyutuna göre hareketli Z-Skoru serisini hesaplar
    static std::vector<double> computeRollingZScore(const std::vector<double>& spread,
                                                    size_t window);
};

#endif // STATARB_ENGINE_SIGNALENGINE_H