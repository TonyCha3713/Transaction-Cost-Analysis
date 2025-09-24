#include "../include/tca/IS.hpp"
#include "../include/tca/Market.hpp"
#include <algorithm>
#include <cassert>

namespace tca {
    ISBreakdown compute_is(const std::vector<Fill>& F, const std::vector<Snap>& M, double p0) {
        assert(!F.empty() && !M.empty());

        double Q = 0.0;
        double paid = 0.0;
        double spread_cost = 0.0;
        double fees = 0.0;

        for (const auto& f : F) {
            Q += f.qty;
            paid += f.qty * f.px;
            const double mid_i = mid_at_or_before(M, f.time);
            const double d = (f.side == Side::BUY) ? (f.px - mid_i) : (mid_i - f.px);
            if (d > 0) spread_cost += d * f.qty;

            fees += f.qty * f.px * (f.fee_bps / 1e4);
        }
        const double denom = Q * p0;
        const double is_dollars = paid - denom;
        const double is_bps     = (is_dollars / denom) * 1e4;

        const double mid_end = M.back().mid;
        const int sign = (F.front().side == Side::BUY) ? +1 : -1;
        const double timing_dollars = Q * sign * (mid_end - p0);

        const double spread_bps   = (spread_cost / denom) * 1e4;
        const double fees_bps     = (fees        / denom) * 1e4;
        const double timing_bps   = (timing_dollars / denom) * 1e4;
        const double residual_bps = is_bps - spread_bps - fees_bps - timing_bps;

        return ISBreakdown{is_bps, spread_bps, fees_bps, timing_bps, residual_bps};
    }

    double infer_arrival_mid(const std::vector<Fill>& F, const std::vector<Snap>& M) {
    assert(!F.empty() && !M.empty());
    double first_time = F.front().time;
    return mid_at_or_before(M, first_time);
    }

}