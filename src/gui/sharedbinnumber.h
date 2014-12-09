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


#ifndef SHARED_BIN_NUMBER_H
#define SHARED_BIN_NUMBER_H

#include <QObject>

#include <boost/serialization/serialization.hpp>

class SharedBinNumber : public QObject
{
    Q_OBJECT
    
public:
    SharedBinNumber();
    explicit SharedBinNumber(unsigned value);
    
    operator unsigned&();
    operator const unsigned&() const;
    SharedBinNumber& operator=(unsigned value);

signals:
    void BinNumberChanged();
    
private:
    unsigned value_;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned version)
    {
        ar & value_;
    }
};

#endif // SHARED_BIN_NUMBER_H