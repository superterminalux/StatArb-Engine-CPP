#ifndef STATARB_ENGINE_COINTEGRATIONTESTER_H
#define STATARB_ENGINE_COINTEGRATIONTESTER_H

#include <vector>
#include <cstddef>

struct ADFResult {
    double t_stat;          // Hesaplanan ADF t-istatistiği
    double critical_5pct;   // MacKinnon %5 kritik değeri (-3.34)
    bool is_cointegrated;   // t_stat < critical_5pct ise true
};

class CointegrationTester {
public:
    // Spread (OLS kalıntı) serisi üzerinde Augmented Dickey-Fuller (ADF) testi uygular
    static ADFResult testADF(const std::vector<double>& spread, size_t lags = 1);
};

#endif // STATARB_ENGINE_COINTEGRATIONTESTER_H