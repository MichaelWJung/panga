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


#include <boost/make_shared.hpp>

#include "mask.h"

#include "datavector.h"

DataVector::DataVector() :
    vector_(boost::make_shared<std::vector<double>>())
{
}

DataVector::DataVector(std::initializer_list<double> l) :
    vector_(boost::make_shared<std::vector<double>>(l))
{
}

DataVector::DataVector(boost::shared_ptr<std::vector<double>> v) :
    vector_(v)
{
}

DataVector::DataVector(const std::vector<double>& data) :
    vector_(boost::make_shared<std::vector<double>>(data))
{
}

DataVector::~DataVector()
{
}

double DataVector::CalcMean(const Mask& mask) const
{
    double sum = 0.;
    unsigned n = 0;
    MaskForEach(*vector_, mask,
            [&](const double& x)
            {
                sum += x;
                ++n;
            });
    return sum / n;
}

double DataVector::CalcStdDev(const Mask& mask) const
{
    double sum = 0.0;
    unsigned n = 0;
    double mean = CalcMean(mask);
    MaskForEach(*vector_, mask,
            [&](const double& x)
            {
                sum += std::pow(mean - x, 2);
                ++n;
            });

    if (n == 0)
        return std::numeric_limits<double>::quiet_NaN();
    return std::sqrt(sum / (n - 1));
}

double DataVector::CalcCorrelation(const DataVector& other,
                                   const Mask& mask) const
{
    double sum = 0.;
    unsigned n = 0;
    double x_mean = CalcMean(mask);
    double y_mean = other.CalcMean(mask);
    MaskForEach(*vector_, *other.vector_, mask,
            [&](const double& x, const double& y)
            {
                sum += (x - x_mean) * (y - y_mean);
                ++n;
            });

    if (n == 0)
        return std::numeric_limits<double>::quiet_NaN();
    return (sum / (n - 1)) / (CalcStdDev(mask) * other.CalcStdDev(mask));
}

