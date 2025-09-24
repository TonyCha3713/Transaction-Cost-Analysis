#pragma once

#include <vector>
#include <cstddef>
#include <cmath>
#include <Eigen/Dense>
#include "Types.hpp"

namespace tca {

struct ImpactParams {
  double eta_bp_per_10pov   = 0.0;
  double gamma_bp_per_10pov = 0.0;
};

inline double pov(double slice_qty, double vol_est_i) {
  return (vol_est_i > 0.0) ? std::abs(slice_qty) / vol_est_i : 0.0;
}

struct RegrData {
  Eigen::MatrixXd X;         
  Eigen::VectorXd y;         
  std::vector<std::size_t> kept_rows; 
};

RegrData build_temp_impact_design(const Fills& fills,
                                  const Snaps& snaps,
                                  bool include_spread_control = true,
                                  bool include_sigma_control  = true);

ImpactParams fit_temporary_impact_ols(const Eigen::MatrixXd& X,
                                      const Eigen::VectorXd& y);

ImpactParams fit_permanent_impact_ols(const Eigen::MatrixXd& Xp,
                                      const Eigen::VectorXd& yp);

}
