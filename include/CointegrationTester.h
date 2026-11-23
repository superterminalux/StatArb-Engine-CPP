#ifndef STATARB_ENGINE_COINTEGRATIONTESTER_H
#define STATARB_ENGINE_COINTEGRATIONTESTER_H

#include <vector>
#include <cstddef>

struct ADFResult {
    double t_stat;          // Hesaplanan ADF t-istatistiği
    double critical_5pct;   // MacKinnon %5 kritik değeri (-3.34)
    double half_life;       // Ornstein-Uhlenbeck yarılanma ömrü (gün)
    bool is_cointegrated;   // t_stat < critical_5pct ise true
};

class CointegrationTester {
public:
    static ADFResult testADF(const std::vector<double>& spread, size_t lags = 1);

    // Ornstein-Uhlenbeck (dS_t = lambda * S_{t-1} + mu) üzerinden yarılanma ömrünü hesaplar
    static double computeHalfLife(const std::vector<double>& spread);
};

#endif // STATARB_ENGINE_COINTEGRATIONTESTER_H