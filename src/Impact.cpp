#include "../include/tca/Impact.hpp"
#include "../include/tca/Market.hpp"
#include <algorithm>
#include <stdexcept>

namespace tca {
    RegrData build_temp_impact_design(const Fills& fills,
                                    const Snaps& snaps,
                                    bool include_spread_control,
                                    bool include_sigma_control) {
    std::size_t n = 0;
    for (const auto& f : fills) {
        (void)f;
        ++n;
    }

    int k = 2;
    if (include_spread_control) ++k;
    if (include_sigma_control)  ++k;

    RegrData D;
    D.X.resize(n, k);
    D.y.resize(n);
    D.kept_rows.reserve(n);

    std::size_t r = 0;
    for (std::size_t i = 0; i < fills.size(); ++i) {
        const auto& f = fills[i];
        const double mid_pre = mid_at_or_before(snaps, f.time);
        const double signed_slip_bps =
        ((f.side == Side::BUY) ? (f.px - mid_pre) : (mid_pre - f.px)) / mid_pre * 1e4;

        // We need a volume estimate at (or near) this time; use nearest snap at/before f.t
        // In this simple builder we reuse the same snap as for mid.
        // (You can refine by slice-bucketing later.)
        auto it = std::upper_bound(snaps.begin(), snaps.end(), f.time,
        [](double tt, const Snap& s){ return tt < s.time; });
        const Snap& sref = (it == snaps.begin()) ? snaps.front() : *std::prev(it);

        const double pov_i = pov(f.qty, sref.volume);
        const double signed_pov = ((f.side == Side::BUY) ? +1.0 : -1.0) * pov_i;

        // Row r
        int c = 0;
        D.X(r, c++) = 1.0;            // intercept
        D.X(r, c++) = signed_pov;     // main regressor
        if (include_spread_control) D.X(r, c++) = sref.spread_bps;
        if (include_sigma_control)  D.X(r, c++) = sref.sigma;

        D.y(r) = signed_slip_bps;
        D.kept_rows.push_back(i);
        ++r;
    }

    // If we skipped rows (e.g., invalid vol_est), shrink to r rows
    D.X.conservativeResize(r, Eigen::NoChange);
    D.y.conservativeResize(r);
    return D;
    }

    ImpactParams fit_temporary_impact_ols(const Eigen::MatrixXd& X,
                                        const Eigen::VectorXd& y) {
    if (X.rows() == 0 || X.rows() != y.size())
        throw std::invalid_argument("X/y shapes invalid");

    // Solve least squares via QR (stable)
    Eigen::VectorXd w = X.colPivHouseholderQr().solve(y);

    // We assume column order: [intercept, signed_pov, ...controls]
    const double slope_pov_bps_per_1pov = w(1);        // bps per POV=1.0
    ImpactParams p;
    p.eta_bp_per_10pov = slope_pov_bps_per_1pov * 0.1; // scale to per 10% POV
    // gamma left for a separate fit later
    return p;
    }

    ImpactParams fit_permanent_impact_ols(const Eigen::MatrixXd& Xp,
                                        const Eigen::VectorXd& yp) {
    // Stub for now (or implement similarly with a longer-horizon y)
    (void)Xp; (void)yp;
    return ImpactParams{};
    }

} // namespace tca
