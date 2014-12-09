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


#ifndef HISTOGRAMDATA_H
#define HISTOGRAMDATA_H

#include <QMetaType>
#include <QPolygon>

#include <qwt_interval.h>
#include <qwt_matrix_raster_data.h>
#include <qwt_series_data.h>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#include <limits>
#include <stack>
#include <vector>

#include "mask.h"
#include "serializationhelpers.h"
#include "sharedbinnumber.h"

class HistogramDataBase;
class HistogramData1D;
class HistogramData2D;
typedef boost::shared_ptr<HistogramDataBase> SharedHistogramDataBase;
typedef boost::shared_ptr<HistogramData1D> SharedHistogramData1D;
typedef boost::shared_ptr<HistogramData2D> SharedHistogramData2D;
typedef boost::variant<SharedHistogramData1D,
                       SharedHistogramData2D> HistogramData;

namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data
        (Archive& ar, const HistogramData1D* d, const unsigned version);
template<class Archive>
inline void save_construct_data
        (Archive& ar, const HistogramData2D* d, const unsigned version);
}}

class HistogramDataBase : public QObject
{
    Q_OBJECT

public:
    explicit HistogramDataBase(
            unsigned long size,
            const SharedBinNumber& n_bins_default,
            SharedMask mask);

    static SharedHistogramDataBase GetBase(HistogramData data);

    const Mask& GetMask() const;

    const bool& IsPlotSpecificBinNumberUsed() const;
    //! Gibt die aktuelle Binzahl zurück.
    unsigned GetCurrentBinNumber() const;
    const bool& IsSelectionInverted() const;

public slots:
    void ResetMask();
    void InvertMask();
    virtual void MaskWithSelection() = 0;
    virtual void ClearSelection();
    //! Setzt die plotspezifische Binzahl.
    void SetBinNumber(int bin_number);
    void UsePlotSpecificBinNumber(bool use_plotspecific_bin_number);
    void SwitchSelectionInverted();

public:
    const unsigned long size;

signals:
    void BinNumberChanged();
    void SelectionChanged();
    void HistogramChanged();

protected:
    void SetSelectionInverted(bool inverted);
    std::vector<bool>& BeginEditMask();
    void EndEditMask();
    void MakeCacheValid() const;
    bool IsCacheInvalid() const;
    void UseDefaultBinNumber(bool use_default_bin_number);
    const unsigned& GetPlotSpecificBinNumber() const;
    void SetPlotSpecificBinNumber(unsigned bin_number);

protected slots:
    void InvalidateCachedHistogram();

private:
    bool use_plotspecific_bin_number_;
    unsigned bin_number_;
    const SharedBinNumber& default_bin_number_;
    mutable bool cache_invalid;
    SharedMask mask_;
    bool selection_inverted_;

    template<class Archive>
    friend void boost::serialization::save_construct_data
            (Archive& ar, const HistogramData1D* d, const unsigned version);
    template<class Archive>
    friend void boost::serialization::save_construct_data
            (Archive& ar, const HistogramData2D* d, const unsigned version);
};

Q_DECLARE_METATYPE(HistogramData)


#endif // HISTOGRAMDATA_H
