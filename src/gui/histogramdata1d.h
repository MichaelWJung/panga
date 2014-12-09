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


#ifndef HISTOGRAMDATA1D_H
#define HISTOGRAMDATA1D_H

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/make_shared.hpp>

#include "datavector.h"

#include "histogramdata.h"

class HistogramData1D : public HistogramDataBase
{
    Q_OBJECT

public:
    HistogramData1D(
            unsigned long size,
            const SharedBinNumber& n_bins_default,
            SharedMask mask,
            boost::shared_ptr<const DataVector> data);

    ~HistogramData1D()
    {
        delete histogramplot_cache_;
    }

    double CalcMean() const;
    double CalcStdDev() const;

    //! Gibt die Histogramdaten zurück.
    /*!
     * Falls sie nicht gecacht sind, werden sie erst berechnet.
     */
    QwtIntervalSeriesData* GetHistogramData() const;

    void MaskWithSelection();

    const QwtInterval& GetSelectionInterval() const;
    void SetSelectionInterval(const QwtInterval& interval);
    void ClearSelection();

    double GetOriginalResult() const;
    void SetOriginalResult(double result);

public slots:
    void ZoomIn(QRectF rect);
    //! Zoomt raus. Tut nichts wenn schon auf der äußersten Zoomstufe.
    void ZoomOut();

private:
    void DetermineOutmostZoomIfZoomStackIsEmpty();

    boost::shared_ptr<const DataVector> data_;
    QwtInterval selection_interval_;
    double original_result_;
    std::stack<QwtInterval> zoom_stack_;
    mutable QwtIntervalSeriesData* histogramplot_cache_;

    HistogramData1D() = default;
    friend class boost::serialization::access;

    template<class Archive>
    friend void boost::serialization::save_construct_data
            (Archive& ar, const HistogramData1D* d, const unsigned version);

    template<class Archive>
    void save(Archive& ar, const unsigned version) const
    {
        ar << IsPlotSpecificBinNumberUsed()
           << GetPlotSpecificBinNumber()
           << zoom_stack_
           << selection_interval_
           << original_result_
           << IsSelectionInverted();
    }

    template<class Archive>
    void load(Archive& ar, const unsigned version)
    {
        bool use_plotspecific_bin_number;
        unsigned bin_number;
        ar >> use_plotspecific_bin_number
           >> bin_number
           >> zoom_stack_
           >> selection_interval_;
        if (version >= 1)
            ar >> original_result_;

        SetPlotSpecificBinNumber(bin_number);
        if (version >= 2)
            UsePlotSpecificBinNumber(use_plotspecific_bin_number);
        else
            // Vor Version 2 war es: use_default_bin_number
            UsePlotSpecificBinNumber(!use_plotspecific_bin_number);

        bool selection_inverted;
        if (version >= 3)
        {
            ar >> selection_inverted;
            // Für den Fall, dass hier noch etwas falsch gespeichert sein sollte
            if (!selection_interval_.isValid() && selection_inverted)
                selection_inverted = false;
        }
        else
            selection_inverted = false;
        SetSelectionInverted(selection_inverted);

        // Bei älteren Dateien könnte der Zoom-Stack möglicherweise leer sein.
        DetermineOutmostZoomIfZoomStackIsEmpty();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(HistogramData1D, 4)


namespace boost { namespace serialization {
template<class Archive>
inline void save_construct_data
        (Archive& ar, const HistogramData1D* d, const unsigned version)
{
    const SharedBinNumber* n_bins_default = &d->default_bin_number_;
    ar << d->size
       << n_bins_default
       << d->mask_
       << d->data_;
}

template<class Archive>
inline void load_construct_data
        (Archive& ar, HistogramData1D* d, const unsigned version)
{
    unsigned long size;
    const SharedBinNumber* n_bins_default;
    SharedMask mask;
    ar >> size
       >> n_bins_default
       >> mask;

    boost::shared_ptr<const DataVector> data;
    if (version < 4)
    {
        boost::shared_ptr<const std::vector<double>> old_data;
        ar >> old_data;
        data = boost::make_shared<DataVector>(
                const_pointer_cast<std::vector<double>>(old_data));
    }
    else
        ar >> data;

    ::new(d) HistogramData1D(size, *n_bins_default, mask, data);
}

}}

#endif // HISTOGRAMDATA1D_H