#include "tca/Report.hpp"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

namespace tca {

using nlohmann::json;

static json to_json(const ISBreakdown& b) {
  return json{
    {"is_bps", b.is_bps},
    {"spread_bps", b.spread_bps},
    {"fees_bps", b.fees_bps},
    {"timing_bps", b.timing_bps},
    {"residual_bps", b.residual_bps}
  };
}

static json to_json(const ImpactParams& p) {
  return json{
    {"eta_bp_per_10pov", p.eta_bp_per_10pov},
    {"gamma_bp_per_10pov", p.gamma_bp_per_10pov}
  };
}

static json to_json(const Schedule& s) {
  json a = json::array();
  for (size_t i=0;i<s.x.size();++i) a.push_back({{"slice", i}, {"shares", s.x[i]}});
  return a;
}

void write_report_json(const std::string& path, const TCAReport& R, bool pretty) {
  json j{
    {"symbol", R.symbol},
    {"arrival_mid", R.arrival_mid},
    {"is", to_json(R.is)},
    {"impact", to_json(R.impact)},
    {"schedule", to_json(R.schedule)}
  };
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot write " + path);
  out << (pretty ? j.dump(2) : j.dump());
}

void write_schedule_csv(const std::string& path, const Schedule& s) {
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot write " + path);
  out << "slice,shares\n";
  for (size_t i=0;i<s.x.size();++i) out << i << "," << s.x[i] << "\n";
}

void write_is_csv(const std::string& path, const ISBreakdown& b) {
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot write " + path);
  out << "metric,value_bps\n";
  out << "is_bps,"      << b.is_bps       << "\n";
  out << "spread_bps,"  << b.spread_bps   << "\n";
  out << "fees_bps,"    << b.fees_bps     << "\n";
  out << "timing_bps,"  << b.timing_bps   << "\n";
  out << "residual_bps,"<< b.residual_bps << "\n";
}

} // namespace tca
