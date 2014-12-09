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

#include "fitresults.h"

std::string FitResults::GetExitFlagAsString(Eigen::LM::Status flag)
{
    switch (flag)
    {
    case Eigen::LM::ImproperInputParameters:
        return "Improper Input Parameters";
    case Eigen::LM::RelativeReductionTooSmall:
    case Eigen::LM::RelativeErrorTooSmall:
    case Eigen::LM::RelativeErrorAndReductionTooSmall:
    case Eigen::LM::CosinusTooSmall:
        return "Converged";
    case Eigen::LM::TooManyFunctionEvaluation:
        return "Too many function calls";
    default:
        return "Error";
    }
}


std::string FitResults::GetExitFlagAsString() const
{
    return GetExitFlagAsString(exit_flag);
}

double FitResults::CorrelationCoefficient(unsigned row, unsigned col) const
{
    assert(row < deviations.size());
    assert(col < deviations.size());
    assert(row < covariance_matrix.rows());
    assert(covariance_matrix.rows() == covariance_matrix.cols());

    return covariance_matrix(row, col) / deviations(row) / deviations(col);
}
