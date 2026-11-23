#include "CointegrationTester.h"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

ADFResult CointegrationTester::testADF(const std::vector<double>& spread, size_t lags) {
    const size_t n = spread.size();
    if (n <= lags + 5) {
        throw std::invalid_argument("[CointegrationTester Error] Not enough observations for ADF test.");
    }

    // 1. Birinci farkları hesapla: diff[t] = spread[t+1] - spread[t]
    std::vector<double> diff(n - 1);
    for (size_t i = 0; i < n - 1; ++i) {
        diff[i] = spread[i + 1] - spread[i];
    }

    // 2. ADF regresyon matrisi X ve hedef vektör Y oluştur
    // Örneklem sayısı: M = (n - 1) - lags
    const size_t M = (n - 1) - lags;
    const size_t k = 1 + lags; // e_{t-1} + gecikmeli farklar

    Eigen::MatrixXd X(M, k);
    Eigen::VectorXd Y(M);

    for (size_t row = 0; row < M; ++row) {
        size_t t = row + lags; // diff indeksi (spread[t+1] - spread[t])
        Y(row) = diff[t];
        X(row, 0) = spread[t]; // e_{t-1}

        for (size_t lag = 1; lag <= lags; ++lag) {
            X(row, lag) = diff[t - lag]; // \Delta e_{t-lag}
        }
    }

    // 3. OLS çözümü: theta = (X^T * X)^-1 * X^T * Y
    Eigen::MatrixXd XtX_inv = (X.transpose() * X).inverse();
    Eigen::VectorXd theta = XtX_inv * X.transpose() * Y;

    // 4. Kalıntı varyansı (sigma^2) ve gamma standart hatası (SE)
    Eigen::VectorXd residuals = Y - X * theta;
    double rss = residuals.squaredNorm();
    double sigma_sq = rss / static_cast<double>(M - k);
    double se_gamma = std::sqrt(sigma_sq * XtX_inv(0, 0));

    double t_stat = theta(0) / se_gamma;
    const double critical_5pct = -3.34; // Engle-Granger 2-var MacKinnon %5 kritik eşiği

    return ADFResult{
        t_stat,
        critical_5pct,
        t_stat < critical_5pct
    };
}