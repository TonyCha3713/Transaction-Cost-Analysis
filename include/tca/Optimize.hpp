#pragma once

#include <vector>
#include <cstddef>
#include "Types.hpp"
#include "Impact.hpp"

namespace tca {
    struct OrderSpec {
        Side side = Side::BUY;
        double qty = 0.0;
        double horizon_s = 0.0;
        int slices = 1;
        double max_pov = 0.0;
        double risk_lambda = 0.0;
    };

    struct Schedule {
        std::vector<double> x;
    };

    Schedule optimize_schedule(const OrderSpec& spec, const Snaps& forecast, const ImpactParams& impact);

    Schedule twap_schedule(const OrderSpec& spec, const Snaps& forecast);
    Schedule vwap_schedule(const OrderSpec& spec, const Snaps& forecast);
}