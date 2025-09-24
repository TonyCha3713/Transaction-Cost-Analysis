#pragma once
#include <algorithm>
#include "Types.hpp"

namespace tca {

// Find latest mid with s.t <= t; clamp to first if t precedes all.
inline double mid_at_or_before(const Snaps& M, double t) {
  auto it = std::upper_bound(M.begin(), M.end(), t,
    [](double tt, const Snap& s){ return tt < s.time; });
  if (it == M.begin()) return M.front().mid;
  return std::prev(it)->mid;
}

} // namespace tca
