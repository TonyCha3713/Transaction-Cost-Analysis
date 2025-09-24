#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tca {

    enum class Side : int { BUY = +1, SELL = -1 };

    struct Fill {
        double time;
        Side side;
        double qty;
        double px;
        std::string venue;
        double fee_bps;
    };

    struct Snap {
        double time;
        double mid;
        double spread_bps;
        double volume;
        double sigma;
    };

    using Fills = std::vector<Fill>;
    using Snaps = std::vector<Snap>;
}