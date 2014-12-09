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


#ifndef HISTOGRAMDATA2D_H
#define HISTOGRAMDATA2D_H

#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/make_shared.hpp>

#include "datavector.h"

#include "histogramdata.h"

class AxisIntervals
{
public:
    AxisIntervals(double min_x, double max_x, double min_y, double max_y) :
        x(min_x, max_x), y(min_y, max_y) {}
    QwtInterval x, y;

private:
    AxisIntervals() = default;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned version)
    {
        ar & x & y;
    }
};

class HistogramData2D : public HistogramDataBase
{
    Q_OBJECT

public:
    HistogramData2D(
            unsigned long size,
            const SharedBinNumber& n_bins_default,
            SharedMask mask,
            boost::shared_ptr<const DataVector> x_data,
            boost::shared_ptr<const DataVector> y_data);

    ~HistogramData2D()
    {
        delete histogramplot_cache_;
    }

    double CalcXMean() const;
    double CalcYMean() const;
    double CalcXStdDev() const;
    double CalcYStdDev() const;
    double CalcCorrelation() const;

    //! Gibt das Histogramraster zurück.
    /*!
     * Falls es nicht gecachet ist, wird es erst berechnet.
     */
    QwtMatrixRasterData* GetHistogramRasterData() const;

    const QwtInterval& GetXInterval() const;
    const QwtInterval& GetYInterval() const;
    void MaskWithSelection();

    const QPolygonF& GetSelectionPolygon() const;
    void SetSelectionPolygon(const QPolygonF& polygon);
    void ClearSelection();

    const QPointF& GetOriginalResult() const;
    void SetOriginalResult(double x, double y);

public slots:
    void ZoomIn(QRectF rect);
    //! Zoomt raus. Tut nichts wenn schon auf der äußersten Zoomstufe.
    void ZoomOut();

private:
    void DetermineOutmostZoomIfZoomStackIsEmpty();

    boost::shared_ptr<const DataVector> x_data_;
    boost::shared_ptr<const DataVector> y_data_;
    QPolygonF selection_polygon_;
    QPointF original_result_;
    double original_result_x_;
    double original_result_y_;
    std::stack<AxisIntervals> zoom_stack_;
    mutable QwtMatrixRasterData* histogramplot_cache_;

    HistogramData2D() = delete;
    friend class boost::serialization::access;

    template<class Archive>
    friend void boost::serialization::save_construct_data
            (Archive& ar, const HistogramData2D* d, const unsigned version);

    template<class Archive>
    void save(Archive& ar, const unsigned version) const
    {
        ar << IsPlotSpecificBinNumberUsed()
           << GetPlotSpecificBinNumber()
           << zoom_stack_
           << selection_polygon_
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
           >> selection_polygon_;
        if (version == 1)
            ar >> original_result_.rx()
               >> original_result_.ry();
        else if (version >= 2)
            ar >> original_result_;

        if (version < 3)
            // Vor Version 3 war es: use_default_bin_number
            UsePlotSpecificBinNumber(!use_plotspecific_bin_number);
        else
            UsePlotSpecificBinNumber(use_plotspecific_bin_number);
        SetPlotSpecificBinNumber(bin_number);

        bool selection_inverted;
        if (version < 4)
            selection_inverted = false;
        else
        {
            ar >> selection_inverted;
            // Für den Fall, dass hier noch etwas falsch gespeichert sein sollte
            if (selection_polygon_.empty() && selection_inverted)
                selection_inverted = false;
        }
        SetSelectionInverted(selection_inverted);

        // Bei älteren Dateien könnte der Zoom-Stack möglicherweise leer sein.
        DetermineOutmostZoomIfZoomStackIsEmpty();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
};

BOOST_CLASS_VERSION(HistogramData2D, 5)

namespace boost { namespace serialization {

template<class Archive>
inline void save_construct_data
        (Archive& ar, const HistogramData2D* d, const unsigned version)
{
    const SharedBinNumber* n_bins_default = &d->default_bin_number_;
    ar << d->size
       << n_bins_default
       << d->mask_
       << d->x_data_
       << d->y_data_;
}

template<class Archive>
inline void load_construct_data
        (Archive& ar, HistogramData2D* d, const unsigned version)
{
    unsigned long size;
    const SharedBinNumber* n_bins_default;
    SharedMask mask;
    boost::shared_ptr<const DataVector> x_data;
    boost::shared_ptr<const DataVector> y_data;
    ar >> size
       >> n_bins_default
       >> mask;
    if (version < 5)
    {
        boost::shared_ptr<const std::vector<double>> old_x_data;
        boost::shared_ptr<const std::vector<double>> old_y_data;
        ar >> old_x_data
           >> old_y_data;
        x_data = boost::make_shared<DataVector>(
                const_pointer_cast<std::vector<double>>(old_x_data));
        y_data = boost::make_shared<DataVector>(
                const_pointer_cast<std::vector<double>>(old_y_data));
    }
    else
        ar >> x_data
           >> y_data;

    ::new(d) HistogramData2D(size, *n_bins_default, mask, x_data, y_data);
}
}}

#endif // HISTOGRAMDATA2D_H