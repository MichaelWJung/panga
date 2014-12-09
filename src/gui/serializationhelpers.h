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


#ifndef SERIALIZATIONHELPERS_H
#define SERIALIZATIONHELPERS_H

#include <QPolygonF>
#include <QString>
#include <QStringList>

#include <qwt_interval.h>

#include <Eigen/Core>

#include <boost/serialization/deque.hpp>
#include <boost/serialization/split_free.hpp>
#include <boost/serialization/string.hpp>

#include <cassert>
#include <stack>

#include "core/fitting/fitresults.h"
#include "core/misc/data.h"
#include "core/models/modelparameter.h"

namespace boost {
namespace serialization {

template<class Archive>
inline void serialize(Archive& ar, FitResults& res, const unsigned version)
{
    ar & res.chi_square
       & res.best_estimate
       & res.deviations
       & res.covariance_matrix
       & res.residuals
       & res.residual_gases
       & res.n_iterations
       & res.degrees_of_freedom
       & res.exit_flag
       & res.model_concentrations
       & res.equilibrium_concentrations
       & res.measured_concentrations;
}

template<class Archive>
inline void serialize(Archive& ar, Data& d, const unsigned version)
{
    ar & d.value
       & d.error;
}

template<class Archive>
inline void serialize(Archive& ar, ModelParameter& p, const unsigned version)
{     
    ar & p.name
       & p.unit
       & p.default_value
       & p.index;
       
    if (version >= 1)
        ar & p.lowest_normal_value
           & p.highest_normal_value
           & p.highest_normal_error;
}
    
template<class Archive>
inline void save(Archive& ar,
                 const Eigen::VectorXd& vec,
                 const unsigned version)
{
    Eigen::EigenBase<Eigen::VectorXd>::Index i = vec.size();
    ar << i;
    for (unsigned i = 0; i < vec.size(); ++i)
        ar << vec[i];
}

template<class Archive>
inline void load(Archive& ar, Eigen::VectorXd& vec, const unsigned version)
{
    Eigen::EigenBase<Eigen::VectorXd>::Index i;
    ar >> i;
    vec.resize(i);
    for (unsigned j = 0; j < i; ++j)
    {
        ar >> vec[j];
    }
}

template<class Archive>
inline void save(Archive& ar,
                 const Eigen::MatrixXd& mat,
                 const unsigned version)
{
    Eigen::EigenBase<Eigen::MatrixXd>::Index rows = mat.rows();
    Eigen::EigenBase<Eigen::MatrixXd>::Index cols = mat.cols();
    ar << rows;
    ar << cols;
    for (unsigned i = 0; i < rows; ++i)
        for (unsigned j = 0; j < cols; ++j)
            ar << mat(i, j);
}

template<class Archive>
inline void load(Archive& ar, Eigen::MatrixXd& mat, const unsigned version)
{
    Eigen::EigenBase<Eigen::MatrixXd>::Index rows;
    Eigen::EigenBase<Eigen::MatrixXd>::Index cols;
    ar >> rows;
    ar >> cols;
    mat.resize(rows, cols);
    for (unsigned i = 0; i < rows; ++i)
        for (unsigned j = 0; j < cols; ++j)
            ar >> mat(i, j);
}

template<class Archive>
inline void save(Archive& ar, const QString& q_str, const unsigned version)
{
    std::string str = q_str.toStdString();
    ar << str;
}

template<class Archive>
inline void load(Archive& ar, QString& q_str, const unsigned version)
{
    std::string str;
    ar >> str;
    q_str = QString::fromStdString(str);
}

template<class Archive>
inline void save(Archive& ar, const QwtInterval& i, const unsigned version)
{
    double min = i.minValue();
    double max = i.maxValue();
    QwtInterval::BorderFlags flags = i.borderFlags();
    assert(flags == QwtInterval::IncludeBorders);
    ar << min
       << max;
}

template<class Archive>
inline void load(Archive& ar, QwtInterval& i, const unsigned version)
{
    double min;
    double max;
    ar >> min
       >> max;
    i.setInterval(min, max);
}

template<class Archive, class T>
inline void save(Archive& ar, const std::stack<T>& s, const unsigned version)
{
    struct HackedStack : private std::stack<T> {
        static const std::deque<T>& GetContainer(const std::stack<T>& s)
        {
            return s.*&HackedStack::c;
        }
    };
    ar << HackedStack::GetContainer(s);
}

template<class Archive, class T>
inline void load(Archive& ar, std::stack<T>& s, const unsigned version)
{
    std::deque<T> deque;
    ar >> deque;
    s = std::stack<T>(std::move(deque));
}

template<class Archive, class T>
inline void serialize(Archive& ar, std::stack<T>& s, const unsigned version)
{
    split_free(ar, s, version);
}

template<class Archive>
inline void save(Archive& ar, const QPolygonF& p, const unsigned version)
{
    std::vector<QPointF> vec = p.toStdVector();
    ar << vec;
}

template<class Archive>
inline void load(Archive& ar, QPolygonF& p, const unsigned version)
{
    std::vector<QPointF> vec;
    ar >> vec;
    p = QPolygonF(QVector<QPointF>::fromStdVector(vec));
}

template<class Archive>
inline void serialize(Archive& ar, QPointF& p, const unsigned version)
{
    ar & p.rx()
       & p.ry();
}

template<class Archive>
inline void save(Archive& ar, const QStringList& l, const unsigned version)
{
    std::list<QString> list = l.toStdList();
    ar << list;
}

template<class Archive>
inline void load(Archive& ar, QStringList& l, const unsigned version)
{
    std::list<QString> list;
    ar >> list;
    l = QStringList(QList<QString>::fromStdList(list));
}

}
}

BOOST_SERIALIZATION_SPLIT_FREE(Eigen::VectorXd)
BOOST_SERIALIZATION_SPLIT_FREE(Eigen::MatrixXd)
BOOST_SERIALIZATION_SPLIT_FREE(QString)
BOOST_SERIALIZATION_SPLIT_FREE(QwtInterval)
BOOST_SERIALIZATION_SPLIT_FREE(QPolygonF)
BOOST_SERIALIZATION_SPLIT_FREE(QStringList)

BOOST_CLASS_VERSION(ModelParameter, 1)

#endif //SERIALIZATIONHELPERS_H
