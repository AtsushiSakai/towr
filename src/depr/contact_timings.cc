/**
 @file    contact_timings.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Jul 5, 2017
 @brief   Brief description
 */

#include <xpp/opt/variables/contact_timings.h>

#include <algorithm>
#include <cassert>
#include <cmath>
#include <Eigen/Dense>
#include <string>
#include <vector>

#include <xpp/opt/bound.h>
#include <xpp/opt/variables/polynomial_spline.h>
#include <xpp/opt/variables/variable_names.h>

namespace xpp {
namespace opt {

static int remove_last = 1;

ContactTimings::ContactTimings (int ee, const TimingsVec& t)
    :Component(0, id::contact_timings + std::to_string(ee))
{
//  int n_phases = 2*max_num_steps;
//  double t_total = 3.0; // just an estimate
//  TimingsVec(n_phases, t_total/n_phases);

//  first_phase_ = Contact; // fix to contact

  // fix maximum time based on initial timings
  t_max_ = std::accumulate(t.begin(), t.end(), 0.0);
//  n_phases_ = t.size();

  t_vec_ = t;
  SetRows(t.size()-remove_last);
}

ContactTimings::~ContactTimings ()
{
  // TODO Auto-generated destructor stub
}

VectorXd
xpp::opt::ContactTimings::GetValues () const
{
  VectorXd x(GetRows());

  for (int i=0; i<x.rows(); ++i) {
    x(i) = t_vec_.at(i);
  }

  return x;

//  // duration of last phase is choosen automatic to keep total time constant
//  return VectorXd::Map(t_vec_.data(), t_vec_.size()-remove_last);
}

void
xpp::opt::ContactTimings::SetValues (const VectorXd& x)
{
  for (int i=0; i<GetRows(); ++i) {
    t_vec_.at(i) = x(i);
  }

  t_vec_.back() = t_max_ - x.sum();

  // refactor/use elegant way like below
//  VectorXd::Map(t_vec_.data(), x.rows()-remove_last) = x;
//  t_vec_.back() = t_max_ - x.sum(); // to always keep total duration the same
}

ContactTimings::TimingsVec
ContactTimings::GetTVecWithTransitions () const
{
  TimingsVec t_vec_trans_;
  t_vec_trans_.push_back(t_vec_.front()-eps_/2); // first phase has no transition at front

  for (int p=1; p<t_vec_.size(); ++p) {
    t_vec_trans_.push_back(eps_);              // transition
    t_vec_trans_.push_back(t_vec_.at(p)-eps_); // phase
  }

  t_vec_trans_.back() += eps_/2;

  return t_vec_trans_;
}

VecBound
ContactTimings::GetBounds () const
{
  VecBound bounds(GetRows());
  std::fill(bounds.begin(), bounds.end(), Bound(0.1, t_max_));

  return bounds;
}

double
ContactTimings::GetContactValue (double t_global) const
{
  double t_local = PolynomialSpline::GetLocalTime(t_global, GetTVecWithTransitions());
  double trans_term = std::pow(t_local,2)/std::pow(eps_,3)*(3*eps_-2*t_local);

  switch (GetPhaseType(t_global)) {
    case InContact:    return 1.0;
    case BreakContact: return 1.0 - trans_term;
    case Flight:       return 0.0;
    case MakeContact:  return trans_term;
    default: assert(false);  // phasetype doesn't exist.
  }
}

JacobianRow
ContactTimings::GetJacobianOfContactValueWrtTimings (double t_global) const
{
  int n = GetRows(); // number of optimization variables (removed last one)
  JacobianRow jac(n);

  double t_local = PolynomialSpline::GetLocalTime(t_global, GetTVecWithTransitions());
  double derivative_wrt_time = 6*t_local/std::pow(eps_,3)*(eps_-t_local);

  int idx = Index(t_global);

  switch (GetPhaseType(t_global)) {
    case InContact:
      break;  // contact value does not depend on timings
    case BreakContact:
      jac.coeffRef(idx) = -derivative_wrt_time;
      break;
    case Flight:
      break;  // contact value does not depend on timings
    case MakeContact:
      jac.coeffRef(idx) =  derivative_wrt_time;
      break;
    default: assert(false);  // phasetype doesn't exist.
  }

  return jac;
}

ContactTimings::PhaseType
ContactTimings::GetPhaseType (double t_global) const
{
  // first check in which phase t_global falls into
  int segment = PolynomialSpline::GetSegmentID(t_global, GetTVecWithTransitions());
  return static_cast<PhaseType>(segment%PhaseCount);
}

int
ContactTimings::Index (double t_global) const
{
  // because duration of previous phase affects the first eps/2 seconds of next phase during transition.
  double t_eval = t_global-eps_/2;
  if (t_eval<0.0) t_eval = 0.0;
  return PolynomialSpline::GetSegmentID(t_eval, t_vec_);
}



} /* namespace opt */
} /* namespace xpp */











