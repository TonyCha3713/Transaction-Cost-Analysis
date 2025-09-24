#pragma once

#include <cstddef>
#include <vector>
#include "Types.hpp"

namespace tca {

    struct ISBreakdown {
        double is_bps;
        double spread_bps;
        double fees_bps;
        double timing_bps;
        double residual_bps;
    };

    ISBreakdown compute_is(const std::vector<Fill>& fills, const std::vector<Snap>& snaps, double arrival_time);

    double infer_arrival_mid(const std::vector<Fill>& fills, const std::vector<Snap>& snaps);

}