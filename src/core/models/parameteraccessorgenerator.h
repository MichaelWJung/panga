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


#include <boost/preprocessor.hpp>

#include <algorithm>
#include <stdexcept>

#include "parameteraccessor.h"

#if NOBLE_PARAMETER_COUNT > 0

//! Wird zum Zugriff auf die Modellparameter verwendet.
class BOOST_PP_CAT(NOBLE_CLASS_PREFIX, ParameterAccessor) : public ParameterAccessor
{
    //! Indizes der Parameter.
    #define BOOST_PP_LOCAL_MACRO(n) const unsigned i##n##_;
    #define BOOST_PP_LOCAL_LIMITS (1, NOBLE_PARAMETER_COUNT)
    #include BOOST_PP_LOCAL_ITERATE()

    //! Zeiger auf die jeweiligen Parameter im Parametervektor.
    #define BOOST_PP_LOCAL_MACRO(n) const double* p##n##_;
    #define BOOST_PP_LOCAL_LIMITS (1, NOBLE_PARAMETER_COUNT)
    #include BOOST_PP_LOCAL_ITERATE()

    //! Der höchste verwendete Index.
    const unsigned max_index_;

public:

    #define PARAMETER_ACCESSOR_GENERATOR_ARGUMENT_NAMES(z, n, data)      \
        unsigned BOOST_PP_CAT(p, BOOST_PP_INC(n))
    #define PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_INDICES(z, n, data)  \
        BOOST_PP_CAT(BOOST_PP_CAT(i, BOOST_PP_INC(n)), _)                \
        (BOOST_PP_CAT(p, BOOST_PP_INC(n)))
    #define PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_POINTERS(z, n, data) \
        BOOST_PP_CAT(BOOST_PP_CAT(p, BOOST_PP_INC(n)), _)(0)
    #define PARAMETER_ACCESSOR_GENERATOR_MAX_L(z, n, data) std::max(BOOST_PP_CAT(p, BOOST_PP_INC(n)),
    #define PARAMETER_ACCESSOR_GENERATOR_MAX_R(z, n, data) )

    //! Konstruktor. Benötigt die Indizes der Parameter.
    /*!
      Die Indizes der Parameter wurden bei ihrer Registrierung beim ParameterManager zurückgegeben.
    */
    BOOST_PP_CAT(NOBLE_CLASS_PREFIX, ParameterAccessor) (
            BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, PARAMETER_ACCESSOR_GENERATOR_ARGUMENT_NAMES, ~)
            ) :
        BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_INDICES, ~),
        BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_POINTERS, ~),
        max_index_(
            BOOST_PP_REPEAT(BOOST_PP_DEC(NOBLE_PARAMETER_COUNT), PARAMETER_ACCESSOR_GENERATOR_MAX_L, ~)
            BOOST_PP_CAT(p, NOBLE_PARAMETER_COUNT)
            BOOST_PP_REPEAT(BOOST_PP_DEC(NOBLE_PARAMETER_COUNT), PARAMETER_ACCESSOR_GENERATOR_MAX_R, ~)
            )
    {
    }

    #undef PARAMETER_ACCESSOR_GENERATOR_ARGUMENT_NAMES
    #undef PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_INDICES
    #undef PARAMETER_ACCESSOR_GENERATOR_INITIALIZE_POINTERS
    #undef PARAMETER_ACCESSOR_GENERATOR_MAX_L
    #undef PARAMETER_ACCESSOR_GENERATOR_MAX_R

    void SetVectorReference(const Eigen::VectorXd& parameters)
    {
        if (parameters.size() < max_index_)
            throw std::invalid_argument("Error: Eigen::VectorXd supplied to SetVectorReference "
                                        "is not big enough");
        #define BOOST_PP_LOCAL_MACRO(n) p##n##_ = &parameters[i##n##_];
        #define BOOST_PP_LOCAL_LIMITS (1, NOBLE_PARAMETER_COUNT)
        #include BOOST_PP_LOCAL_ITERATE()
    }

    //! Funktionen zum Zugriff auf die Parameter.
    #define BOOST_PP_LOCAL_MACRO(n)                                                            \
        const double& BOOST_PP_TUPLE_ELEM(NOBLE_PARAMETER_COUNT, BOOST_PP_DEC(n), NOBLE_PARAMETER_NAMES)() \
        { return *p##n##_; }
    #define BOOST_PP_LOCAL_LIMITS (1, NOBLE_PARAMETER_COUNT)
    #include BOOST_PP_LOCAL_ITERATE()
};

#else //NOBLE_PARAMETER_COUNT > 0

class BOOST_PP_CAT(NOBLE_CLASS_PREFIX, ParameterAccessor) : public ParameterAccessor
{
public:
    BOOST_PP_CAT(NOBLE_CLASS_PREFIX, ParameterAccessor) () {}
    void SetVectorReference(const Eigen::VectorXd& /*parameters*/) {}
};

#endif //NOBLE_PARAMETER_COUNT > 0
