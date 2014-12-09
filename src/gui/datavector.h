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


#ifndef DATAVECTOR_H
#define DATAVECTOR_H

#include <boost/serialization/serialization.hpp>
#include <boost/shared_ptr.hpp>

#include <initializer_list>
#include <vector>

class Mask;
class DataVector
{

public:
    DataVector();
    DataVector(std::initializer_list<double> l);
    DataVector(boost::shared_ptr<std::vector<double>> v);
    DataVector(const std::vector<double>& data);
    virtual ~DataVector();

    double CalcMean(const Mask& mask) const;
    double CalcStdDev(const Mask& mask) const;
    double CalcCorrelation(const DataVector& other, const Mask& mask) const;

    boost::shared_ptr<std::vector<double>> vector_;

private:

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned version)
    {
        ar & vector_;
    }
};

#endif // DATAVECTOR_H
