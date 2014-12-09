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

Mask::Mask(unsigned size) :
    mask_(size, true),
    exit_flags_(boost::make_shared<std::vector<Eigen::LM::Status>>(size)),
    active_(false),
    old_size_(size),
    n_unmasked_elements_(size),
    per_cent_converged_(0.),
    per_cent_converged_valid_(false)
{
}

Mask::Mask(unsigned size,
           boost::shared_ptr<std::vector<Eigen::LM::Status>> exit_flags) :
    mask_(size, true),
    exit_flags_(exit_flags),
    active_(false),
    old_size_(size),
    n_unmasked_elements_(size),
    per_cent_converged_(0.),
    per_cent_converged_valid_(false)
{
}

std::vector<bool>& Mask::BeginEditMask()
{
    active_ = false;
    per_cent_converged_valid_ = false;
    old_size_ = mask_.size();
    return mask_;
}

void Mask::EndEditMask()
{
    assert(old_size_ == mask_.size());
    active_ = true;
    per_cent_converged_valid_ = false;
    n_unmasked_elements_ = 0;
    std::for_each(mask_.begin(), mask_.end(),
                  [&](bool b)
                  {
                      if (b) ++n_unmasked_elements_;
                  });

    emit MaskChanged();
}

void Mask::InvertMask()
{
    BeginEditMask();
    for(unsigned i = 0; i < mask_.size(); ++i)
        mask_[i] = !mask_[i];
    EndEditMask();
}

unsigned Mask::NumberOfUnmaskedElements() const
{
    return active() ? n_unmasked_elements_ : mask_.size();
}

double Mask::NumberOfUnmaskedElementsPerCent() const
{
    return (active() ? double(n_unmasked_elements_) / mask_.size() : 1.) *
            100;
}

double Mask::PerCentConverged() const
{
    if (!per_cent_converged_valid_) CalcPerCentConverged();
    return per_cent_converged_;
}

void Mask::reset()
{
    active_ = false;
    per_cent_converged_valid_ = false;
    std::fill(mask_.begin(), mask_.end(), true);

    emit MaskChanged();
}

bool Mask::active() const
{
    return active_;
}

void Mask::CalcPerCentConverged() const
{
    unsigned n_converged = 0;
    MaskForEach(*exit_flags_, *this,
                [&](Eigen::LM::Status flag)
                {
                    if (flag == Eigen::LM::RelativeReductionTooSmall         ||
                        flag == Eigen::LM::RelativeErrorTooSmall             ||
                        flag == Eigen::LM::RelativeErrorAndReductionTooSmall ||
                        flag == Eigen::LM::CosinusTooSmall)
                        ++n_converged;
                });
    per_cent_converged_ = double(n_converged) / NumberOfUnmaskedElements() *100;
    per_cent_converged_valid_ = true;
}

bool Mask::IsEnabled(unsigned index) const
{
    return !active() || mask_.at(index);
}
