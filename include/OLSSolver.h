#ifndef STATARB_ENGINE_OLSSOLVER_H
#define STATARB_ENGINE_OLSSOLVER_H

#include <vector>

struct OLSResult {
    double alpha; // Sabit terim (Intercept)
    double beta;  // İlişki katsayısı (Hedge Ratio)
};

class OLSSolver {
public:
    // Verilen X ve Y fiyat serileri için OLS regresyonu çözer
    static OLSResult fit(const std::vector<double>& x_prices,
                         const std::vector<double>& y_prices);
};

#endif // STATARB_ENGINE_OLSSOLVER_H