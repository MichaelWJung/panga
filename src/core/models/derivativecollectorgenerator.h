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


#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/preprocessor.hpp>

#include <map>
#include <vector>

#include "derivativecollector.h"

#if NOBLE_PARAMETER_COUNT > 0

namespace NOBLE_CLASS_PREFIX
{
    enum Parameter
    {
        #define DERIVATIVE_COLLECTOR_GENERATOR_PARAMETER(z, n, data) \
            BOOST_PP_TUPLE_ELEM(NOBLE_PARAMETER_COUNT, n, NOBLE_PARAMETER_NAMES),
        BOOST_PP_REPEAT(NOBLE_PARAMETER_COUNT, DERIVATIVE_COLLECTOR_GENERATOR_PARAMETER, ~)
        #undef DERIVATIVE_COLLECTOR_GENERATOR_PARAMETER
        OTHER
    };
}

//! Wird zum "Einsammeln" der benötigten Ableitung verwendet.
/*!
  Wird zum "Einsammeln" der benötigten Ableitung bei den Methoden zur Berechnung der Ggw-Konzentration
  sowie bei den Excess-Air-Modellen verwendet.
  */
class BOOST_PP_CAT(NOBLE_CLASS_PREFIX, DerivativeCollector) : public DerivativeCollector
{
public:
    #define DERIVATIVE_COLLECTOR_GENERATOR_ARGUMENT_NAMES(z, n, data)                            \
        unsigned BOOST_PP_TUPLE_ELEM(NOBLE_PARAMETER_COUNT, n, NOBLE_PARAMETER_NAMES)
    #define DERIVATIVE_COLLECTOR_GENERATOR_INITIALIZE_INDICES(z, n, data)                        \
        {BOOST_PP_TUPLE_ELEM(NOBLE_PARAMETER_COUNT, n, NOBLE_PARAMETER_NAMES),                   \
        NOBLE_CLASS_PREFIX :: BOOST_PP_TUPLE_ELEM(NOBLE_PARAMETER_COUNT, n, NOBLE_PARAMETER_NAMES)}

    //! Konstruktor. Benötigt die Indizes der Parameter.
    /*!
      Die Indizes der Parameter wurden bei ihrer Registrierung beim ParameterManager zurückgegeben.
      */
    BOOST_PP_CAT(NOBLE_CLASS_PREFIX, DerivativeCollector)(
            BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, DERIVATIVE_COLLECTOR_GENERATOR_ARGUMENT_NAMES, ~)
            ) :
        indices_(),
        derivative_information_()
    {
        indices_ = std::map<unsigned, NOBLE_CLASS_PREFIX ::Parameter>({ 
                BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT,
                              DERIVATIVE_COLLECTOR_GENERATOR_INITIALIZE_INDICES,
                              ~
                              )
                });
    }

    #undef DERIVATIVE_COLLECTOR_GENERATOR_ARGUMENT_NAMES
    #undef DERIVATIVE_COLLECTOR_GENERATOR_INITIALIZE_INDICES

    ~ BOOST_PP_CAT(NOBLE_CLASS_PREFIX, DerivativeCollector)() {}

    //! Gibt einen Vektor zurück in den die benötigten Ableitungen geschrieben werden müssen.
    /*!
      Das erste Element der Paare gibt den Parameter an, nach dem abgeleitet werden soll.
      Das zweite Element einen Zeiger auf die Speicherstelle, an die die Ableitung geschrieben werden
      soll.
      */
    const std::vector<std::pair<NOBLE_CLASS_PREFIX ::Parameter, double*> >& GetDerivativeInformation()
    {
        return derivative_information_;
    }
    

private:

    void UpdateDerivativeInformation(Eigen::RowVectorXd& results, const std::vector<int>& indices)
    {
        derivative_information_.clear();

        int j = 0;
        BOOST_FOREACH(int i, indices)
        {
            if (indices_.count(i))
                derivative_information_.push_back(std::make_pair(indices_[i], &results[j]));
            //#ifdef NOBLE_IS_EXCESS_AIR_MODEL
            else
                derivative_information_.push_back(
                            std::make_pair(NOBLE_CLASS_PREFIX ::OTHER, &results[j]));
            //#endif //NOBLE_IS_EXCESS_AIR_MODEL
            ++j;
        }
    }
    
    std::map<unsigned, NOBLE_CLASS_PREFIX ::Parameter> indices_;

    //! Hier wird gespeichert nach welchen Parametern abgeleitet werden soll.
    /*!
      Das erste Element der Paare gibt den Parameter an, nach dem abgeleitet werden soll.
      Das zweite Element einen Zeiger auf die Speicherstelle, an die die Ableitung geschrieben werden
      soll.
      */
    std::vector<std::pair<NOBLE_CLASS_PREFIX ::Parameter, double*> > derivative_information_;
};

#else //NOBLE_PARAMETER_COUNT > 0
//! \todo 0-Parameter-Fall implementieren.
#error 0 parameter case not yet implemented.
#endif //NOBLE_PARAMETER_COUNT > 0
