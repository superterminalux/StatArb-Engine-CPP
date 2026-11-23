#ifndef STATARB_ENGINE_DATALOADER_H
#define STATARB_ENGINE_DATALOADER_H

#include <string>
#include <vector>

class DataLoader {
public:
    static bool loadCSV(const std::string& filepath,
                        std::vector<std::string>& dates,
                        std::vector<double>& x_prices,
                        std::vector<double>& y_prices);
};

#endif // STATARB_ENGINE_DATALOADER_H