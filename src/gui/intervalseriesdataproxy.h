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


#ifndef INTERVALSERIESDATAPROXY_H
#define INTERVALSERIESDATAPROXY_H

#include <qwt_series_data.h>

//! Proxy-Objekt für QwtIntervalSeriesData.
/*!
 * Hält einen Pointer auf ein QwtIntervalSeriesData-Objekt und leitet alle
 * Aufrufe an dieses weiter. Wird IntervalSeriesDataProxy zerstört wird das
 * QwtIntervalSeriesData-Objekt allerdings nicht gelöscht. Vermeidet unnötiges
 * Kopieren.
 */
class IntervalSeriesDataProxy : public QwtSeriesData<QwtIntervalSample>
{
public:
    explicit IntervalSeriesDataProxy(
            QwtIntervalSeriesData* interval_series_data) :
        interval_series_data_(interval_series_data)
    {
    }
    
    ~IntervalSeriesDataProxy()
    {
    }
    
    virtual size_t size() const
    {
        return interval_series_data_->size();
    }
    
    virtual QwtIntervalSample sample(size_t i) const
    {
        return interval_series_data_->sample(i);
    }
    
    virtual QRectF boundingRect() const
    {
        return interval_series_data_->boundingRect();
    }
    
    
private:
    QwtIntervalSeriesData* interval_series_data_;
};

#endif // INTERVALSERIESDATAPROXY_H
