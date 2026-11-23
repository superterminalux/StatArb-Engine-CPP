#ifndef STATARB_ENGINE_KALMANFILTER_H
#define STATARB_ENGINE_KALMANFILTER_H

#include <Eigen/Dense>

struct KalmanEstimate {
    double alpha;
    double beta;
    double spread;      // Tahmin hatası (e_t = Y_t - (beta * X_t + alpha))
    double spread_std;  // Tahmin hatasının anlık standart sapması (sqrt(S_t))
};

class KalmanFilter {
public:
    // delta: durum gürültüsü (beta'nın değişim hızı, örn. 1e-4)
    // obs_noise: gözlem gürültüsü (R, örn. 1.0)
    explicit KalmanFilter(double delta = 1e-4, double obs_noise = 1.0);

    // Yeni (X_t, Y_t) fiyat çifti geldiğinde alpha, beta ve anlık spread'i günceller
    KalmanEstimate update(double x_price, double y_price);

    double getAlpha() const { return theta_(0); }
    double getBeta() const { return theta_(1); }

private:
    Eigen::Vector2d theta_; // [alpha, beta]^T
    Eigen::Matrix2d P_;     // Hata kovaryans matrisi (2x2)
    Eigen::Matrix2d Q_;     // Süreç gürültüsü kovaryans matrisi (2x2)
    double R_;              // Ölçüm gürültüsü varyansı
    bool initialized_;
};

#endif // STATARB_ENGINE_KALMANFILTER_H