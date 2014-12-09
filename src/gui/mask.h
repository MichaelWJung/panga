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


#ifndef MASK_H
#define MASK_H

#include <QObject>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/shared_ptr.hpp>

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

#include "core/fitting/fitresults.h"

class Mask : public QObject
{
    Q_OBJECT

public:
    explicit Mask(unsigned size);
    Mask(unsigned size,
         boost::shared_ptr<std::vector<Eigen::LM::Status>> exit_flags);
    std::vector<bool>& BeginEditMask();
    void EndEditMask();
    void InvertMask();
    unsigned NumberOfUnmaskedElements() const;
    double NumberOfUnmaskedElementsPerCent() const;
    double PerCentConverged() const;
    void reset();
    bool active() const;
    bool IsEnabled(unsigned index) const;

signals:
    void MaskChanged();

private:
    Mask() = default;
    void CalcPerCentConverged() const;

    std::vector<bool> mask_;
    boost::shared_ptr<std::vector<Eigen::LM::Status>> exit_flags_;
    bool active_;
    unsigned old_size_;
    unsigned n_unmasked_elements_;
    mutable double per_cent_converged_;
    mutable double per_cent_converged_valid_;

    template<class T, class Function>
    friend void MaskForEach(const std::vector<T>& vec,
                            const Mask& mask, Function func);

    template<class T, class Function>
    friend void MaskForEach(const std::vector<T>& vec1,
                            const std::vector<T>& vec2,
                            const Mask& mask, Function func);

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned version)
    {
        ar & mask_
           & exit_flags_
           & active_
           & old_size_
           & n_unmasked_elements_
           & per_cent_converged_
           & per_cent_converged_valid_;
    }
};

typedef boost::shared_ptr<Mask> SharedMask;

template<class T, class Function>
void MaskForEach(const std::vector<T>& vec, const Mask& mask, Function func)
{
    if (mask.active())
    {
        assert(vec.size() == mask.mask_.size());
        auto v = vec.begin();
        auto m = mask.mask_.begin();
        for (; v != vec.end(); ++v, ++m)
            if (*m)
                func(*v);
    }
    else
        std::for_each(vec.begin(), vec.end(), func);
}

template<class T, class Function>
void MaskForEach(const std::vector<T>& vec1,
                 const std::vector<T>& vec2,
                 const Mask& mask,
                 Function func)
{
    if (mask.active())
    {
        assert(vec1.size() == mask.mask_.size() &&
               vec2.size() == mask.mask_.size());
        auto v1 = vec1.begin();
        auto v2 = vec2.begin();
        auto m = mask.mask_.begin();
        for (; v1 != vec1.end(); ++v1, ++v2, ++m)
            if (*m)
                func(*v1, *v2);
    }
    else
    {
        auto v1 = vec1.begin();
        auto v2 = vec2.begin();
        for (; v1 != vec1.end(); ++v1, ++v2)
            func(*v1, *v2);
    }
}

#endif // MASK_H
