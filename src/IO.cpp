#include "../include/tca/utils.hpp"
#include "../include/tca/IO.hpp"
#include <fstream>
#include <algorithm>
#include <stdexcept>

namespace tca {

Fills load_fills_csv(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::string line;
  std::getline(in, line); // header
  Fills v;

  while (std::getline(in, line)) {
    if (trim(line).empty()) continue;
    auto c = split_csv(line);
    if (c.size() < 6) continue;

    Fill f;
    f.time = std::stod(c[0]);
    auto s = trim(c[1]);
    if (s == "BUY" || s == "buy" || s == "1") f.side = Side::BUY;
    else if (s == "SELL" || s == "sell" || s == "-1") f.side = Side::SELL;
    else throw std::runtime_error("invalid side: " + s);
    f.qty     = std::stod(c[2]);
    f.px      = std::stod(c[3]);
    f.venue   = trim(c[4]);
    f.fee_bps = std::stod(c[5]);

    v.push_back(f);
  }
  std::sort(v.begin(), v.end(), [](const Fill& a, const Fill& b){ return a.time < b.time; });
  return v;
}

Snaps load_snaps_csv(const std::string& path) {
  std::ifstream in(path);
  if (!in) throw std::runtime_error("cannot open " + path);
  std::string line;
  std::getline(in, line); // header
  Snaps v;

  while (std::getline(in, line)) {
    if (trim(line).empty()) continue;
    auto c = split_csv(line);
    if (c.size() < 5) continue;

    Snap s;
    s.time          = std::stod(c[0]);
    s.mid        = std::stod(c[1]);
    s.spread_bps = std::stod(c[2]);
    s.volume    = std::stod(c[3]);
    s.sigma      = std::stod(c[4]);

    v.push_back(s);
  }
  std::sort(v.begin(), v.end(), [](const Snap& a, const Snap& b){ return a.time < b.time; });
  return v;
}

} // namespace tca
