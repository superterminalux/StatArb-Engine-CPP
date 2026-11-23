#include "CointegrationTester.h"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

double CointegrationTester::computeHalfLife(const std::vector<double>& spread) {
    const size_t n = spread.size();
    if (n < 10) return -1.0;

    const size_t M = n - 1;
    Eigen::MatrixXd X(M, 2);
    Eigen::VectorXd Y(M);

    for (size_t i = 0; i < M; ++i) {
        X(i, 0) = 1.0;                      // Sabit terim (mu)
        X(i, 1) = spread[i];                // S_{t-1}
        Y(i)    = spread[i + 1] - spread[i]; // dS_t
    }

    Eigen::VectorXd theta = (X.transpose() * X).inverse() * X.transpose() * Y;
    double lambda = theta(1);

    // lambda >= 0 ise seri ortalamaya dönmüyor (ıraksıyor) demektir
    if (lambda >= -1e-6) {
        return 999.0;
    }

    return -std::log(2.0) / lambda;
}

ADFResult CointegrationTester::testADF(const std::vector<double>& spread, size_t lags) {
    const size_t n = spread.size();
    if (n <= lags + 5) {
        throw std::invalid_argument("[CointegrationTester Error] Not enough observations for ADF test.");
    }

    std::vector<double> diff(n - 1);
    for (size_t i = 0; i < n - 1; ++i) {
        diff[i] = spread[i + 1] - spread[i];
    }

    const size_t M = (n - 1) - lags;
    const size_t k = 1 + lags;

    Eigen::MatrixXd X(M, k);
    Eigen::VectorXd Y(M);

    for (size_t row = 0; row < M; ++row) {
        size_t t = row + lags;
        Y(row) = diff[t];
        X(row, 0) = spread[t];

        for (size_t lag = 1; lag <= lags; ++lag) {
            X(row, lag) = diff[t - lag];
        }
    }

    Eigen::MatrixXd XtX_inv = (X.transpose() * X).inverse();
    Eigen::VectorXd theta = XtX_inv * X.transpose() * Y;

    Eigen::VectorXd residuals = Y - X * theta;
    double rss = residuals.squaredNorm();
    double sigma_sq = rss / static_cast<double>(M - k);
    double se_gamma = std::sqrt(sigma_sq * XtX_inv(0, 0));

    double t_stat = theta(0) / se_gamma;
    const double critical_5pct = -3.34;
    double hl = computeHalfLife(spread);

    return ADFResult{
        t_stat,
        critical_5pct,
        hl,
        t_stat < critical_5pct
    };
}