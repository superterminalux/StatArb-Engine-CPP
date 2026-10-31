#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <Eigen/Dense>

int main() {

    // Initialize containers to store historical price series
    std::vector<double> co_prices;
    std::vector<double> pep_prices;

    // Open the historical price data file
    std::ifstream file("sample_data.csv");
    if (!file.is_open()) {
        std::cerr << "Error: sample_data.csv can not found, be sure that file is in the same directory with .exe \n";
        return 1;
    }

    std::string line;
    bool is_header = true;

    // Read data row by row and store into vectors
    while (std::getline(file, line)) {
        // Skip the header row containing column names
        if (is_header) {
            is_header = false;
            continue;
        }

        std::stringstream ss(line);
        std::string co_str, pep_str;

        if (std::getline(ss, co_str, ',') && std::getline(ss, pep_str, ',')) {
            co_prices.push_back(std::stod(co_str));
            pep_prices.push_back(std::stod(pep_str));
        }
    }
    file.close();

    int n = co_prices.size();
    std::cout << n << " data records read from CSV. \n\n";

    // EIGEN OLS MATH SOLUTION
    Eigen::MatrixXd X(n, 2);
    Eigen::VectorXd Y(n);

    // Populate the design matrix X (with intercept) and target vector Y
    for (int i = 0; i<n; i++) {
        X(i, 0) = 1.0;
        X(i, 1) = co_prices[i];
        Y(i) = pep_prices[i];
    }
    Eigen::MatrixXd X_T = X.transpose();

    // Compute OLS coefficients using the normal equation: Beta = (X^T * X)^-1 * X^T * Y
    Eigen::VectorXd Beta = (X_T * X).inverse() * X_T * Y;

    std::cout << "--- Statistical Arbitrage Engine & OLS Test --- \n";
    std::cout << "Constant (Alpha): " << Beta(0) << "\n";
    std::cout << "Coefficient (Beta): " << Beta(1) << "\n";

    return 0;
}