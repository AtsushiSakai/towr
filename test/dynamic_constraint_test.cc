/**
 @file    dynamic_constraint_test.cc
 @author  Alexander W. Winkler (winklera@ethz.ch)
 @date    Dec 5, 2016
 @brief   Brief description
 */

#include <xpp/opt/dynamic_constraint.h>
#include <xpp/opt/com_spline6.h>
#include <gtest/gtest.h>

namespace xpp {
namespace opt {


TEST(DynamicConstraintTest, EvaluateConstraint)
{
  double T = 0.5;

  auto com = std::make_shared<ComSpline6>();
  com->SetConstantHeight(0.58);
  com->Init(T, 3);

  CenterOfPressure cop;
  double dt_cop = 0.02;
  cop.Init(dt_cop, T);

  DynamicConstraint constraint;
  double dt_constraint = 0.05;
  constraint.Init(*com, cop, T, dt_constraint);

  std::cout << "count: " << constraint.GetNumberOfConstraints() << std::endl;

  std::cout << constraint.EvaluateConstraint().transpose() << std::endl;

  std::cout << constraint.GetJacobianWrtCop() << std::endl << std::endl;
}


} /* namespace opt */
} /* namespace xpp */
