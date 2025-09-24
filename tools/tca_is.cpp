#include <iostream>
#include <stdexcept>

#include "../include/tca/IS.hpp"
#include "../include/tca/IO.hpp"

using namespace tca;

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: " << argv[0] << " --fills fills.csv --mkt mkt.csv --arrival 102.30\n";
        return 1;
    }
    std::string fills_path, mkt_path;
    double arrival_mid = 0.0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--fills"   && i + 1 < argc) fills_path = argv[++i];
        else if (arg == "--mkt" && i + 1 < argc) mkt_path = argv[++i];
        else if (arg == "--arrival" && i + 1 < argc) arrival_mid = std::stod(argv[++i]);
    }
    if (fills_path.empty() || mkt_path.empty() || arrival_mid <= 0.0) {
        std::cerr << "missing/invalid args\n"; return 2;
    }

    try {
        auto fills = load_fills_csv(fills_path);
        auto snaps = load_snaps_csv(mkt_path);
        auto b = compute_is(fills, snaps, arrival_mid);

        std::cout.setf(std::ios::fixed);
        std::cout.precision(3);
        std::cout << "Arrival mid:  " << arrival_mid << "\n"
                  << "IS (bps):     " << b.is_bps << "\n"
                  << "  Spread:     " << b.spread_bps << "\n"
                  << "  Fees:       " << b.fees_bps << "\n"
                  << "  Timing:     " << b.timing_bps << "\n"
                  << "  Residual:   " << b.residual_bps << "\n";
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 3;
    }
    return 0;
}
