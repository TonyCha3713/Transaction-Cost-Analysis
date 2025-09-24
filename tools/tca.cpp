#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <nlohmann/json.hpp>

#include "../include/tca/Types.hpp"
#include "tca/IO.hpp"
#include "tca/Market.hpp"
#include "tca/IS.hpp"
#include "tca/Impact.hpp"
#include "tca/Optimize.hpp"
#include "tca/Report.hpp"

using namespace tca;
using nlohmann::json;

static void die(const std::string& msg){ std::cerr << "error: " << msg << "\n"; std::exit(2); }
static void usage() {
  std::cerr <<
  "tca <subcommand> [options]\n\n"
  "Subcommands:\n"
  "  is --fills F --mkt M --arrival P0\n"
  "  fit-impact --fills F --mkt M [--no-spread] [--no-sigma]\n"
  "  optimize --order order.json --mkt M --impact impact.json --out schedule.csv\n"
  "  report --symbol SYM --fills F --mkt M --arrival P0 --impact impact.json "
  "          --order order.json --out report.json [--sched schedule.csv] [--is is.csv]\n";
}

int main(int argc, char** argv) {
  if (argc < 2) { usage(); return 1; }
  std::string cmd = argv[1];

  try {
    if (cmd == "is") {
      std::string fills, mkt; double p0 = 0.0;
      for (int i=2;i<argc;++i){
        std::string a=argv[i];
        if (a=="--fills"&&i+1<argc) fills=argv[++i];
        else if (a=="--mkt"&&i+1<argc) mkt=argv[++i];
        else if (a=="--arrival"&&i+1<argc) p0=std::stod(argv[++i]);
      }
      if (fills.empty()||mkt.empty()||p0<=0.0) die("is: need --fills --mkt --arrival");
      auto F = load_fills_csv(fills);
      auto M = load_snaps_csv(mkt);
      auto b = compute_is(F,M,p0);
      std::cout.setf(std::ios::fixed); std::cout.precision(3);
      std::cout<<"IS (bps): "<<b.is_bps<<"\n  Spread: "<<b.spread_bps
               <<"\n  Fees: "<<b.fees_bps<<"\n  Timing: "<<b.timing_bps
               <<"\n  Residual: "<<b.residual_bps<<"\n";
      return 0;
    }

    if (cmd == "fit-impact") {
      std::string fills, mkt; bool sp=true, sg=true;
      for (int i=2;i<argc;++i){
        std::string a=argv[i];
        if (a=="--fills"&&i+1<argc) fills=argv[++i];
        else if (a=="--mkt"&&i+1<argc) mkt=argv[++i];
        else if (a=="--no-spread") sp=false;
        else if (a=="--no-sigma")  sg=false;
      }
      if (fills.empty()||mkt.empty()) die("fit-impact: need --fills --mkt");
      auto F = load_fills_csv(fills);
      auto M = load_snaps_csv(mkt);
      auto D = build_temp_impact_design(F,M,sp,sg);
      auto P = fit_temporary_impact_ols(D.X, D.y);
      std::cout.setf(std::ios::fixed); std::cout.precision(3);
      std::cout<<"eta ≈ "<<P.eta_bp_per_10pov<<" bps per 10% POV\n";
      return 0;
    }

    if (cmd == "optimize") {
      std::string orderp, mktf, impactp, out="schedule.csv";
      for (int i=2;i<argc;++i){
        std::string a=argv[i];
        if (a=="--order"&&i+1<argc) orderp=argv[++i];
        else if (a=="--mkt"&&i+1<argc) mktf=argv[++i];
        else if (a=="--impact"&&i+1<argc) impactp=argv[++i];
        else if (a=="--out"&&i+1<argc) out=argv[++i];
      }
      if (orderp.empty()||mktf.empty()||impactp.empty()) die("optimize: need --order --mkt --impact");
      // read JSON files
      auto read_json = [](const std::string& path){ std::ifstream in(path); if(!in) throw std::runtime_error("cannot open " + path); json j; in>>j; return j; };
      json jo = read_json(orderp), ji = read_json(impactp);
      OrderSpec spec;
      spec.side = (jo.value("side","BUY")=="SELL")?Side::SELL:Side::BUY;
      spec.qty  = jo.at("qty").get<double>();
      spec.horizon_s = jo.value("horizon_s",0.0);
      spec.slices    = jo.at("slices").get<int>();
      spec.max_pov   = jo.value("max_pov",1.0);
      spec.risk_lambda = jo.value("risk_lambda",0.0);
      ImpactParams ip;
      ip.eta_bp_per_10pov   = ji.value("eta_bp_per_10pov",0.0);
      ip.gamma_bp_per_10pov = ji.value("gamma_bp_per_10pov",0.0);
      auto M = load_snaps_csv(mktf);
      if ((int)M.size()!=spec.slices) die("mkt.csv rows must equal order.slices");

      auto sch = optimize_schedule(spec, M, ip);
      write_schedule_csv(out, sch);
      double tot=0; for(double v:sch.x) tot+=v;
      std::cout<<"Wrote "<<out<<" | slices="<<sch.x.size()<<" | sum="<<tot<<"\n";
      return 0;
    }

    if (cmd == "report") {
      // end-to-end: IS + eta + schedule + JSON/CSV outputs
      std::string sym="UNKNOWN", fills, mkt, impactp, orderp, out="report.json", sched="schedule.csv", iscsv="";
      double p0 = 0.0;
      for (int i=2;i<argc;++i){
        std::string a=argv[i];
        if (a=="--symbol"&&i+1<argc) sym=argv[++i];
        else if (a=="--fills"&&i+1<argc) fills=argv[++i];
        else if (a=="--mkt"&&i+1<argc) mkt=argv[++i];
        else if (a=="--arrival"&&i+1<argc) p0=std::stod(argv[++i]);
        else if (a=="--impact"&&i+1<argc) impactp=argv[++i];
        else if (a=="--order"&&i+1<argc) orderp=argv[++i];
        else if (a=="--out"&&i+1<argc) out=argv[++i];
        else if (a=="--sched"&&i+1<argc) sched=argv[++i];
        else if (a=="--is"&&i+1<argc) iscsv=argv[++i];
      }
      if (fills.empty()||mkt.empty()||impactp.empty()||orderp.empty()||p0<=0.0) {
        die("report: need --symbol --fills --mkt --arrival --impact --order");
      }
      auto F = load_fills_csv(fills);
      auto M = load_snaps_csv(mkt);
      // parse JSONs
      auto read_json = [](const std::string& path){ std::ifstream in(path); if(!in) throw std::runtime_error("cannot open " + path); json j; in>>j; return j; };
      json ji = read_json(impactp), jo = read_json(orderp);

      ImpactParams ip;
      ip.eta_bp_per_10pov   = ji.value("eta_bp_per_10pov",0.0);
      ip.gamma_bp_per_10pov = ji.value("gamma_bp_per_10pov",0.0);

      OrderSpec spec;
      spec.side = (jo.value("side","BUY")=="SELL")?Side::SELL:Side::BUY;
      spec.qty  = jo.at("qty").get<double>();
      spec.slices = jo.at("slices").get<int>();
      spec.max_pov = jo.value("max_pov",1.0);
      spec.risk_lambda = jo.value("risk_lambda",0.0);

      // compute pieces
      TCAReport R;
      R.symbol = sym;
      R.arrival_mid = p0;
      R.is = compute_is(F, M, p0);
      R.impact = ip;
      if ((int)M.size()!=spec.slices) die("mkt.csv rows must equal order.slices");
      R.schedule = optimize_schedule(spec, M, ip);

      // write outputs
      write_report_json(out, R, true);
      write_schedule_csv(sched, R.schedule);
      if (!iscsv.empty()) write_is_csv(iscsv, R.is);

      std::cout<<"Wrote "<<out<<" and "<<sched;
      if(!iscsv.empty()) std::cout<<" and "<<iscsv;
      std::cout<<"\n";
      return 0;
    }

    usage();
    return 1;

  } catch (const std::exception& e) {
    std::cerr << "error: " << e.what() << "\n";
    return 3;
  }
}
