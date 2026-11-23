#include "KalmanFilter.h"
#include <cmath>

KalmanFilter::KalmanFilter(double delta, double obs_noise)
    : theta_(Eigen::Vector2d::Zero()),
      P_(Eigen::Matrix2d::Identity() * 1000.0),
      R_(obs_noise),
      initialized_(false) {
    // Rastgele yürüyüş (random walk) süreç gürültüsü
    double q_val = delta / (1.0 - delta);
    Q_ = Eigen::Matrix2d::Identity() * q_val;
}

KalmanEstimate KalmanFilter::update(double x_price, double y_price) {
    Eigen::RowVector2d H;
    H << 1.0, x_price;

    if (!initialized_) {
        // İlk gözlemde beta'yı oran olarak başlat
        theta_(0) = 0.0;
        theta_(1) = (std::abs(x_price) > 1e-9) ? (y_price / x_price) : 1.0;
        initialized_ = true;
    }

    // 1. Zaman Güncellemesi (Predict)
    Eigen::Matrix2d P_pred = P_ + Q_;

    // 2. Ölçüm Yeniliği / Spread (Innovation)
    double y_pred = H * theta_;
    double error = y_price - y_pred; // e_t

    // 3. Yenilik Varyansı (S_t) ve Kalman Kazancı (K_t)
    double S = (H * P_pred * H.transpose())(0, 0) + R_;
    Eigen::Vector2d K = (P_pred * H.transpose()) / S;

    // 4. Durum ve Kovaryans Güncellemesi (Update)
    theta_ = theta_ + K * error;
    Eigen::Matrix2d I = Eigen::Matrix2d::Identity();
    P_ = (I - K * H) * P_pred;

    return KalmanEstimate{
        theta_(0),
        theta_(1),
        error,
        std::sqrt(std::max(S, 1e-12))
    };
}