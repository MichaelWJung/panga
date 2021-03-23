// Copyright © 2014 Michael Jung
// 
// This file is part of Panga.
// 
// Panga is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Panga is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Panga.  If not, see <http://www.gnu.org/licenses/>.


#include <cassert>
#include <iostream>
#include <limits>
#include <stdexcept>

#include "eigen_lm_includes.h"
#include "noblefitfunction.h"

#include "levenbergmarquardtfitter.h"

namespace
{
// Generic functor
template<typename _Scalar, int NX=Eigen::Dynamic, int NY=Eigen::Dynamic>
struct Functor
{
  typedef _Scalar Scalar;
  enum {
    InputsAtCompileTime = NX,
    ValuesAtCompileTime = NY
  };
  typedef Eigen::Matrix<Scalar,InputsAtCompileTime,1> InputType;
  typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,1> ValueType;
  typedef Eigen::Matrix<Scalar,ValuesAtCompileTime,
                        InputsAtCompileTime> JacobianType;

  const int m_inputs, m_values;

  Functor() : m_inputs(InputsAtCompileTime), m_values(ValuesAtCompileTime) {}
  Functor(int inputs, int values) : m_inputs(inputs), m_values(values) {}

  int inputs() const { return m_inputs; }
  int values() const { return m_values; }

  // you should define that in the subclass :
//  void operator() (const InputType& x, ValueType* v, JacobianType* _j=0) const;
};

class FitFunctor : public Functor<double>
{
public:
    FitFunctor(std::shared_ptr<NobleFitFunction> func,
               int n_inputs,
               int n_values);
    int operator()(const Eigen::VectorXd& x, Eigen::VectorXd& fvec) const;
    int df(const Eigen::VectorXd& x, Eigen::MatrixXd& fjac) const;
    
private:
    std::shared_ptr<NobleFitFunction> func_;
};

inline FitFunctor::FitFunctor(
        std::shared_ptr<NobleFitFunction> func,
        int n_inputs,
        int n_values) :
    Functor(n_inputs, n_values),
    func_(func)
{
}

inline int FitFunctor::operator()(
        const Eigen::VectorXd& x,
        Eigen::VectorXd& fvec) const
{
    func_->SetParameters(x);
    func_->CalcResiduals(fvec);
    return 0;
}

inline int FitFunctor::df(const Eigen::VectorXd& x, Eigen::MatrixXd& fjac) const
{
    func_->SetParameters(x);
    func_->CalcJacobian(fjac);
    return 0;
}
}

LevenbergMarquardtFitter::LevenbergMarquardtFitter(
        std::shared_ptr<NobleFitFunction> func) :
    func_(func)
{
}

std::shared_ptr<FitResults> LevenbergMarquardtFitter::fit(
        std::shared_ptr<const FitParameterConfig> pconf
        ) const
{
    std::shared_ptr<FitResults> results(std::make_shared<FitResults>());

    int n_params = pconf->size();
    int n_values = func_->NumberOfConcentrations();
    
    if (n_values >= n_params)
    {
        FitFunctor functor(func_, n_params, n_values);
        Eigen::LevenbergMarquardt<FitFunctor> lm(functor);
        lm.parameters.maxfev = 10000;
        
        Eigen::VectorXd x(pconf->initials());
        
        results->exit_flag = lm.minimize(x);
        
        if (n_params > 0)
        {
            Eigen::covar(lm.fjac, lm.permutation.indices(),
                         std::numeric_limits<double>::epsilon());
            results->covariance_matrix = 
                    lm.fjac.topLeftCorner(n_params, n_params);
            results->residuals = lm.fvec;
        }
        else
        {
            results->covariance_matrix = Eigen::MatrixXd(0, 0);
            functor(x, results->residuals);
        }
        
        results->best_estimate = x;
        results->deviations = results->covariance_matrix.diagonal().cwiseSqrt(); 
        results->chi_square =
                results->residuals.cwiseProduct(results->residuals).sum();
        results->degrees_of_freedom = n_values - n_params;
        results->n_iterations = lm.iter;
    }
    else
    {
        const double nan = std::numeric_limits<double>::quiet_NaN();
        results->exit_flag = Eigen::LM::ImproperInputParameters;
        results->best_estimate.setConstant(n_params, nan); 
        results->chi_square = nan;
        results->covariance_matrix.setConstant(n_params, n_params, nan);
        results->degrees_of_freedom = (n_values - n_params);
        results->deviations.setConstant(n_params, nan);
        results->n_iterations = 0;
        results->residuals.setConstant(n_params, nan);
    }

    func_->CompileResults(results);
    
    return results;
}

std::shared_ptr<LevenbergMarquardtFitter> LevenbergMarquardtFitter::clone() const
{
    return std::make_shared<LevenbergMarquardtFitter>(func_->clone());
}

std::shared_ptr<NobleFitFunction> LevenbergMarquardtFitter::GetFitFunction()
{
    return func_;
}
