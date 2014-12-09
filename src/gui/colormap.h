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


#ifndef COLORMAP_H
#define COLORMAP_H

#include <qwt_color_map.h>


class ColorMap : public QwtLinearColorMap
{
public:
    ColorMap();
    
private:
    static const std::vector<int> COLORMAP;
};

class LogarithmicColorMap : public ColorMap
{
public:
    
    //! Bildet den übergebenen Wert auf eine Farbe ab.
    /*!
      \param interval Das Intervall, über das sich die Farbskala erstreckt.
      \param value Abzubildender Wert.
      \return Resultierende Farbe.
      */
    QRgb rgb(const QwtInterval &interval, double value) const;
};

class HistogramColorMap : public ColorMap
{
public:
    
    //! Bildet den übergebenen Wert auf eine Farbe ab.
    /*!
      \param interval Das Intervall, über das sich die Farbskala erstreckt.
      \param value Abzubildender Wert.
      \return Resultierende Farbe.
      */
    QRgb rgb(const QwtInterval &interval, double value) const;
};


#endif // COLORMAP_H
