#include <iostream>
#include <stdexcept>
#include <Eigen/Dense>
#include "../include/tca/Types.hpp"
#include "../include/tca/Impact.hpp"
#include "../include/tca/IO.hpp"

using namespace tca;

static double r2(const Eigen::VectorXd& y, const Eigen::VectorXd& yhat) {
    const double ymean = y.mean();
    double sst = 0.0, ssr = 0.0;
    for (int i=0;i<y.size();++i) {
        const double d = y[i] - ymean;
        const double e = y[i] - yhat[i];
        sst += d*d;
        ssr += e*e;
    }
    return (sst > 0.0) ? 1.0 - (ssr/sst) : 0.0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: " << argv[0]
                  << " --fills data/fills.csv --mkt data/mkt.csv"
                  << " [--no-spread] [--no-sigma]\n";
        return 1;
    }
    std::string fills_path, mkt_path;
    bool use_spread = true, use_sigma = true;

    for (int i=1; i<argc; ++i) {
        std::string a = argv[i];
        if (a == "--fills" && i+1<argc) fills_path = argv[++i];
        else if (a == "--mkt" && i+1<argc) mkt_path = argv[++i];
        else if (a == "--no-spread") use_spread = false;
        else if (a == "--no-sigma")  use_sigma  = false;
    }
    if (fills_path.empty() || mkt_path.empty()) {
        std::cerr << "missing required args --fills / --mkt\n";
        return 2;
    }

    try {
        auto fills = load_fills_csv(fills_path);
        auto snaps = load_snaps_csv(mkt_path);

        auto D = tca::build_temp_impact_design(fills, snaps, use_spread, use_sigma);
        if (D.X.rows() == 0) {
            std::cerr << "no usable rows for regression\n";
            return 3;
        }

        // Fit temporary impact
        auto params = tca::fit_temporary_impact_ols(D.X, D.y);

        // Simple diagnostics
        Eigen::VectorXd yhat = D.X * (D.X.colPivHouseholderQr().solve(D.y));
        double R2 = r2(D.y, yhat);

        std::cout.setf(std::ios::fixed);
        std::cout.precision(3);
        std::cout << "Rows used: " << D.X.rows()
                  << " | cols: " << D.X.cols()
                  << " | R^2: " << R2 << "\n";
        std::cout << "Temporary impact eta ≈ "
                  << params.eta_bp_per_10pov
                  << " bps per 10% POV\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 4;
    }
    return 0;
}
