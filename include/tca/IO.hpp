#pragma once
#include <string>
#include "Types.hpp"

namespace tca {

// Loaders assume a header row.
// fills.csv: ts,side,qty,price,venue,fee_bps
Fills load_fills_csv(const std::string& path);

// mkt.csv: ts,mid,spread_bps,vol_est,sigma
Snaps load_snaps_csv(const std::string& path);

} // namespace tca
