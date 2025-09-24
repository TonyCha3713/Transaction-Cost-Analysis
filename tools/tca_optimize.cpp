#include <iostream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <nlohmann/json.hpp>

#include "../include/tca/Types.hpp"
#include "../include/tca/Impact.hpp"
#include "../include/tca/IO.hpp"
#include "../include/tca/Optimize.hpp"

using json = nlohmann::json;
using namespace tca;

static OrderSpec read_order_json(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  json j; in >> j;

  OrderSpec s;
  const std::string side = j.value("side", "BUY");
  s.side = (side == "SELL") ? Side::SELL : Side::BUY;
  s.qty = j.at("qty").get<double>();
  s.horizon_s = j.value("horizon_s", 0.0);
  s.slices = j.at("slices").get<int>();
  s.max_pov = j.value("max_pov", 1.0);
  s.risk_lambda = j.value("risk_lambda", 0.0);
  return s;
}

static ImpactParams read_impact_json(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  json j; in >> j;
  ImpactParams p;
  p.eta_bp_per_10pov   = j.value("eta_bp_per_10pov", 0.0);
  p.gamma_bp_per_10pov = j.value("gamma_bp_per_10pov", 0.0);
  return p;
}

static void write_schedule_csv(const std::string& path, const Schedule& sch) {
  std::ofstream out(path);
  if (!out) throw std::runtime_error("cannot write " + path);
  out << "slice,shares\n";
  for (size_t i = 0; i < sch.x.size(); ++i) {
    out << i << "," << sch.x[i] << "\n";
  }
}

static void usage(const char* argv0){
  std::cerr << "usage: " << argv0
    << " --order order.json --mkt mkt.csv --impact impact.json --out schedule.csv\n";
}

int main(int argc, char** argv) {
  if (argc < 9) { usage(argv[0]); return 1; }

  std::string order_path, mkt_path, impact_path, out_path = "schedule.csv";
  for (int i=1; i<argc; ++i) {
    std::string a = argv[i];
    if (a=="--order" && i+1<argc)  order_path  = argv[++i];
    else if (a=="--mkt" && i+1<argc) mkt_path   = argv[++i];
    else if (a=="--impact" && i+1<argc) impact_path = argv[++i];
    else if (a=="--out" && i+1<argc)   out_path  = argv[++i];
  }
  if (order_path.empty() || mkt_path.empty() || impact_path.empty()){
    usage(argv[0]); return 2;
  }

  try {
    OrderSpec spec = read_order_json(order_path);
    Snaps forecast = load_snaps_csv(mkt_path);
    if ((int)forecast.size() != spec.slices)
      throw std::runtime_error("forecast rows must equal OrderSpec.slices");

    ImpactParams impact = read_impact_json(impact_path);

    Schedule sch = optimize_schedule(spec, forecast, impact);
    write_schedule_csv(out_path, sch);

    // quick stdout summary
    double total = 0.0; for (double xi : sch.x) total += xi;
    std::cout.setf(std::ios::fixed); std::cout.precision(3);
    std::cout << "Wrote " << out_path << " | slices=" << sch.x.size()
              << " | sum(shares)=" << total << "\n";
  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n"; return 3;
  }
  return 0;
}
