#include "OLSSolver.h"
#include <Eigen/Dense>
#include <stdexcept>

OLSResult OLSSolver::fit(const std::vector<double>& x_prices,
                         const std::vector<double>& y_prices) {
    if (x_prices.size() != y_prices.size() || x_prices.size() < 2) {
        throw std::invalid_argument("[OLSSolver Error] Invalid or mismatched input vector sizes.");
    }

    const size_t n = x_prices.size();

    // Tasarım matrisi X (n x 2) ve hedef vektör Y (n x 1)
    Eigen::MatrixXd X(n, 2);
    Eigen::VectorXd Y(n);

    for (size_t i = 0; i < n; ++i) {
        X(i, 0) = 1.0;         // Intercept (alpha) sütunu
        X(i, 1) = x_prices[i]; // Bağımsız değişken (CO)
        Y(i)    = y_prices[i]; // Bağımlı değişken (PEP)
    }

    // Normal Denklem: theta = (X^T * X)^-1 * X^T * Y
    Eigen::MatrixXd X_T = X.transpose();
    Eigen::VectorXd theta = (X_T * X).inverse() * X_T * Y;

    return OLSResult{theta(0), theta(1)};
}