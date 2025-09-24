#pragma once
#include <string>
#include "Types.hpp"
#include "IS.hpp"
#include "Impact.hpp"
#include "Optimize.hpp"

namespace tca {

struct TCAReport {
  // inputs for provenance
  std::string symbol;
  double arrival_mid = 0.0;

  // core results
  ISBreakdown is{};
  ImpactParams impact{};
  Schedule schedule{};
};

// Write JSON report to path (pretty by default)
void write_report_json(const std::string& path, const TCAReport& R, bool pretty=true);

// Write schedule CSV: "slice,shares"
void write_schedule_csv(const std::string& path, const Schedule& sch);

// Write IS breakdown CSV: "metric,value_bps"
void write_is_csv(const std::string& path, const ISBreakdown& b);

} // namespace tca
