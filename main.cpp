#include <Eigen/Dense>

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

    // Structure to store paired price data (independent and dependent variables)
struct PricePair {
    std::vector<double> x; // independent variable (e.g. CO)
    std::vector<double> y; // dependent variable (e.g. PEP)
};

    // Function to load price data from a CSV file
PricePair loadPrices(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    PricePair data;
    std::string line;
    std::getline(file, line); // Skip the header row

    std::size_t line_no = 1;
    while (std::getline(file, line)) {
        ++line_no;
        if (line.empty()) {
            continue;
        }
        std::stringstream ss(line);
        std::string date, col_x, col_y;

        // Parse comma-separated values: date, x, y
        if (!std::getline(ss, date, ',') ||
            !std::getline(ss, col_x, ',') ||
            !std::getline(ss, col_y, ',')) {
            std::cerr << "Skipping malformed row " << line_no << '\n';
            continue;
        }
        try {
            data.x.push_back(std::stod(col_x));
            data.y.push_back(std::stod(col_y));
        } catch (const std::exception&) {
            std::cerr << "Skipping non-numeric row " << line_no << '\n';
        }
    }

    if (data.x.empty() || data.x.size() != data.y.size()) {
        throw std::runtime_error("No valid data loaded from " + path);
    }
    return data;
}

    // Structure to store the results of the OLS regression
struct OLSResult {
    double alpha;            // intercept
    double beta;             // slope = hedge ratio
    Eigen::VectorXd residuals; // y - (alpha + beta*x) = spread
};

    // Function to run Ordinary Least Squares (OLS) regression
OLSResult runOLS(const std::vector<double>& x, const std::vector<double>& y) {
    const auto n = static_cast<Eigen::Index>(x.size());

    Eigen::MatrixXd X(n, 2);
    Eigen::VectorXd Y(n);
    for (Eigen::Index i = 0; i < n; ++i) {
        X(i, 0) = 1.0;   // intercept column
        X(i, 1) = x[i];  // regressor column
        Y(i)    = y[i];
    }

    // Numerically stable solution (QR with column pivoting).
    // Alternative: (X^T X)^-1 X^T Y (normal equations) — faster but less stable.
    Eigen::Vector2d beta = X.colPivHouseholderQr().solve(Y);

    Eigen::VectorXd residuals = Y - X * beta;

    return OLSResult{beta(0), beta(1), std::move(residuals)};
}

    // Function to calculate the sample standard deviation
double sampleStdDev(const Eigen::VectorXd& v) {
    if (v.size() < 2) {
        return 0.0;
    }
    const double m = v.mean();
    const double sum_sq = (v.array() - m).square().sum();
    return std::sqrt(sum_sq / static_cast<double>(v.size() - 1));
}

} // namespace

int main() {
    try {
        // Load price data from the CSV file
        const PricePair prices = loadPrices("sample_data.csv");
        std::cout << "Loaded " << prices.x.size() << " rows\n";

        // Run OLS regression to find alpha and beta
        const OLSResult ols = runOLS(prices.x, prices.y);
        std::cout << "OLS: y = " << ols.alpha << " + " << ols.beta << " * x\n";

        // Print statistical metrics of the spread (residuals)
        std::cout << "Spread (residual): mean = " << ols.residuals.mean()
                  << ", std = " << sampleStdDev(ols.residuals) << '\n';
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}