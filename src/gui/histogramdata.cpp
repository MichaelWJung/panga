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


#include "histogramdata.h"
#include "histogramdata1d.h"
#include "histogramdata2d.h"

HistogramDataBase::HistogramDataBase(
        long unsigned int size,
        const SharedBinNumber& n_bins_default,
        SharedMask mask)  :
    size(size),
    use_plotspecific_bin_number_(false),
    bin_number_(0),
    default_bin_number_(n_bins_default),
    cache_invalid(true),
    mask_(mask),
    selection_inverted_(false)
{
    connect(mask_.get(), SIGNAL(MaskChanged()),
            this, SLOT(InvalidateCachedHistogram()));
    connect(&default_bin_number_, SIGNAL(BinNumberChanged()),
            this, SLOT(InvalidateCachedHistogram()));
    connect(&default_bin_number_, SIGNAL(BinNumberChanged()),
            this, SIGNAL(BinNumberChanged()));
}

const Mask& HistogramDataBase::GetMask() const
{
    return *mask_;
}

void HistogramDataBase::ResetMask()
{
    mask_->reset();
}

void HistogramDataBase::InvertMask()
{
    mask_->InvertMask();
}

void HistogramDataBase::ClearSelection()
{
    selection_inverted_ = false;
    emit SelectionChanged();
}

const bool& HistogramDataBase::IsPlotSpecificBinNumberUsed() const
{
    return use_plotspecific_bin_number_;
}

void HistogramDataBase::UsePlotSpecificBinNumber(
        bool use_plotspecific_bin_number)
{
    if (use_plotspecific_bin_number_ != use_plotspecific_bin_number)
    {
        use_plotspecific_bin_number_ = use_plotspecific_bin_number;
        // bin_number_ == 0 bedeutet uninitialisiert.
        if (use_plotspecific_bin_number_ && bin_number_ == 0)
            bin_number_ = default_bin_number_;
        if (bin_number_ != default_bin_number_)
            InvalidateCachedHistogram();
        emit BinNumberChanged();
    }
}

unsigned HistogramDataBase::GetCurrentBinNumber() const
{
    const unsigned bin_number = use_plotspecific_bin_number_
            ? bin_number_
            : static_cast<unsigned>(default_bin_number_);
    assert(bin_number > 0);
    return bin_number;
}

void HistogramDataBase::SetBinNumber(int bin_number)
{
    bool number_changed =
            GetCurrentBinNumber() != static_cast<unsigned>(bin_number);
    use_plotspecific_bin_number_ = true;
    bin_number_ = bin_number;
    if (number_changed)
        InvalidateCachedHistogram();
    emit BinNumberChanged();
}

const bool& HistogramDataBase::IsSelectionInverted() const
{
    return selection_inverted_;
}

void HistogramDataBase::SetSelectionInverted(bool inverted)
{
    bool selection_inverted_changed = selection_inverted_ != inverted;
    selection_inverted_ = inverted;
    if (selection_inverted_changed)
        emit SelectionChanged();
}

void HistogramDataBase::SwitchSelectionInverted()
{
    selection_inverted_ = !selection_inverted_;
    emit SelectionChanged();
}

std::vector<bool>& HistogramDataBase::BeginEditMask()
{
    return mask_->BeginEditMask();
}

void HistogramDataBase::EndEditMask()
{
    mask_->EndEditMask();
}

void HistogramDataBase::InvalidateCachedHistogram()
{
    cache_invalid = true;
    emit HistogramChanged();
}

void HistogramDataBase::MakeCacheValid() const
{
    cache_invalid = false;
}

bool HistogramDataBase::IsCacheInvalid() const
{
    return cache_invalid;
}

const unsigned int& HistogramDataBase::GetPlotSpecificBinNumber() const
{
    return bin_number_;
}

void HistogramDataBase::SetPlotSpecificBinNumber(unsigned bin_number)
{
    bool number_changed = bin_number_ == bin_number;
    bin_number_ = bin_number;
    if (use_plotspecific_bin_number_ && number_changed)
    {
        emit BinNumberChanged();
        InvalidateCachedHistogram();
    }
}

class GetHistogramDataBaseVisitor :
    public boost::static_visitor<SharedHistogramDataBase>
{
public:
    template<class T>
    SharedHistogramDataBase operator()(T data) const;
};

template<class T>
inline SharedHistogramDataBase
GetHistogramDataBaseVisitor::operator()(T data) const
{
    return boost::dynamic_pointer_cast<HistogramDataBase>(data);
}

SharedHistogramDataBase HistogramDataBase::GetBase(HistogramData data)
{
    return boost::apply_visitor(GetHistogramDataBaseVisitor(), data);
}