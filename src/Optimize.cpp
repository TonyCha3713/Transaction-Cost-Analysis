#include "../include/tca/Optimize.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

namespace tca {
    static inline int side_sign(Side s) { return (s == Side::BUY) ? +1 : -1; }

    static inline double cap_for_slice(double max_pov, double vol_est) {
        if (max_pov <= 0.0 || vol_est <= 0.0) return 0.0;
        return max_pov * vol_est; 
    }

    static bool enforce_caps_and_completion(std::vector<double>& x, double Q, const std::vector<double>& caps)
    {
        const int n = static_cast<int>(x.size());
        const int sgn = (Q >= 0.0) ? +1 : -1;
        for (int i = 0; i < n; ++i) {
            double xi_abs = std::min(std::abs(x[i]), caps[i]);
            x[i] = sgn * std::max(0.0, xi_abs); 
        }

        double sum = std::accumulate(x.begin(), x.end(), 0.0);
        auto almost_eq = [](double a, double b) { return std::abs(a - b) <= 1e-9 * (1.0 + std::abs(b)); };
        if (almost_eq(sum, std::abs(Q))) return true;

        if (sum > std::abs(Q)) {
            double scale = (sum > 0.0) ? (std::abs(Q) / sum) : 0.0;
            for (int i = 0; i < n; ++i) x[i] *= scale;
            return true; 
        }

        for (int iter = 0; iter < 4; ++iter) { 
            double need = std::abs(Q) - std::accumulate(x.begin(), x.end(), 0.0);
            if (need <= 1e-9) return true;

            std::vector<double> head(n);
            for (int i = 0; i < n; ++i) head[i] = std::max(0.0, caps[i] - x[i]);

            double head_sum = std::accumulate(head.begin(), head.end(), 0.0);
            if (head_sum <= 1e-12) break; 

            for (int i = 0; i < n; ++i) {
            double add = need * (head[i] / head_sum);
            x[i] = std::min(caps[i], x[i] + add);
            }
        }
        double filled = std::accumulate(x.begin(), x.end(), 0.0);
        return std::abs(filled - std::abs(Q)) <= 1e-6 * (1.0 + std::abs(Q));
    }

    static std::vector<double> build_caps(const OrderSpec& spec, const Snaps& forecast)
    {
        const int n = static_cast<int>(forecast.size());
        std::vector<double> caps(n, 0.0);
        for (int i = 0; i < n; ++i) {
            caps[i] = cap_for_slice(spec.max_pov, forecast[i].volume);
        }
        return caps;
    }

    static void validate_inputs(const OrderSpec& spec, const Snaps& forecast) {
        if (spec.qty <= 0.0) throw std::invalid_argument("OrderSpec.qty must be > 0");
        if (spec.slices <= 0) throw std::invalid_argument("OrderSpec.slices must be >= 1");
        if (static_cast<int>(forecast.size()) != spec.slices)
            throw std::invalid_argument("forecast.size() must equal OrderSpec.slices");
    }


    Schedule twap_schedule(const OrderSpec& spec, const Snaps& forecast)
    {
        validate_inputs(spec, forecast);
        const int n = spec.slices;
        const int sgn = side_sign(spec.side);

        std::vector<double> x(n, std::abs(spec.qty) / std::max(1, n));
        for (double& xi : x) xi = sgn * xi;

        auto caps = build_caps(spec, forecast);
        bool ok = enforce_caps_and_completion(x, sgn * std::abs(spec.qty), caps);
        Schedule sch;
        sch.x = std::move(x);
        return sch;
    }

    Schedule vwap_schedule(const OrderSpec& spec, const Snaps& forecast)
    {
        validate_inputs(spec, forecast);
        const int n = spec.slices;
        const int sgn = side_sign(spec.side);

        std::vector<double> w(n, 0.0);
        for (int i = 0; i < n; ++i) w[i] = std::max(0.0, forecast[i].volume);

        double wsum = std::accumulate(w.begin(), w.end(), 0.0);
        std::vector<double> x(n, 0.0);
        if (wsum <= 0.0) {
            x.assign(n, std::abs(spec.qty) / std::max(1, n));
        } else {
            for (int i = 0; i < n; ++i) x[i] = std::abs(spec.qty) * (w[i] / wsum);
        }
        for (double& xi : x) xi = sgn * xi;

        auto caps = build_caps(spec, forecast);
        (void)enforce_caps_and_completion(x, sgn * std::abs(spec.qty), caps);

        Schedule sch;
        sch.x = std::move(x);
        return sch;
    }


    Schedule optimize_schedule(const OrderSpec& spec, const Snaps& forecast, const ImpactParams& impact)
    {
        validate_inputs(spec, forecast);
        const int n   = spec.slices;
        const int sgn = side_sign(spec.side);

        bool any_vol = false;
        for (const auto& s : forecast) if (s.volume > 0.0) { any_vol = true; break; }
        if (!any_vol) return twap_schedule(spec, forecast);

        const double eta10 = std::max(0.0, impact.eta_bp_per_10pov); 
        const double A = 1.0;    
        const double B = 10.0;   
        const double C = spec.risk_lambda;
        const double EPS = 1e-9;

        std::vector<double> score(n, 0.0);
        for (int i = 0; i < n; ++i) {
            const double V = std::max(0.0, forecast[i].volume);
            const double sprbp = std::max(0.0, forecast[i].spread_bps);
            const double sig = std::max(0.0, forecast[i].sigma);

            const double impact_proxy = (eta10 / 10.0) / std::max(1.0, V);

            const double denom = A * sprbp + B * impact_proxy + C * sig + EPS;
            score[i] = (V > 0.0) ? (V / denom) : 0.0;
        }

        double sc_sum = std::accumulate(score.begin(), score.end(), 0.0);
        std::vector<double> x(n, 0.0);
        if (sc_sum <= 0.0) {
            x.assign(n, std::abs(spec.qty) / std::max(1, n));
        } else {
            for (int i = 0; i < n; ++i) x[i] = std::abs(spec.qty) * (score[i] / sc_sum);
        }

        for (double& xi : x) xi = sgn * xi;

        auto caps = build_caps(spec, forecast);
        bool ok = enforce_caps_and_completion(x, sgn * std::abs(spec.qty), caps);

        (void)ok;

        Schedule sch;
        sch.x = std::move(x);
        return sch;
    }

} 
