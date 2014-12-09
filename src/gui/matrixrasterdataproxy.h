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


#ifndef MATRIXRASTERDATAPROXY_H
#define MATRIXRASTERDATAPROXY_H

#include <qwt_matrix_raster_data.h>

#include <cassert>

//! Proxy-Objekt für QwtMatrixRasterData.
/*!
 * Hält einen Pointer auf ein QwtMatrixRasterData-Objekt und leitet alle
 * Aufrufe an dieses weiter. Wird MatrixRasterDataProxy zerstört wird das
 * QwtMatrixRasterData-Objekt allerdings nicht gelöscht. Vermeidet unnötiges
 * Kopieren.
 */
class MatrixRasterDataProxy : public QwtRasterData
{
public:
    explicit MatrixRasterDataProxy(QwtMatrixRasterData* matrix_raster_data) :
        matrix_raster_data_(matrix_raster_data)
    {
        assert(matrix_raster_data_);
    }

    virtual ~MatrixRasterDataProxy()
    {
    }

    virtual double value(double x, double y) const
    {
        return matrix_raster_data_->value(x, y);
    }

    virtual void initRaster(const QRectF& rect, const QSize& raster)
    {
        matrix_raster_data_->initRaster(rect, raster);
    }

    virtual void discardRaster()
    {
        matrix_raster_data_->discardRaster();
    }

private:
    QwtMatrixRasterData* matrix_raster_data_;
};

#endif // MATRIXRASTERDATAPROXY_H
