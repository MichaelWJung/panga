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

#ifndef NOBLE_HELPER_CLASSES_INCLUDED
    #error You either did not include generatehelperclasses.h first, or you include generatehelpermethots.h multiple times.
#endif
#undef NOBLE_HELPER_CLASSES_INCLUDED
    
private:
    typedef BOOST_PP_CAT(NOBLE_CLASS_PREFIX, ParameterAccessor) LocalParameterAccessor;
    typedef BOOST_PP_CAT(NOBLE_CLASS_PREFIX, DerivativeCollector) LocalDerivativeCollector;
    typedef NOBLE_CLASS_PREFIX::Parameter LocalParameter;
    
public:
    #define HELPER_METHODS_PARAMETER_NAME(z, n, data) \
        BOOST_PP_CAT(index, BOOST_PP_CAT(n, _))
    std::shared_ptr<ParameterAccessor> GetParameterAccessor() const
    {
        return std::make_shared<LocalParameterAccessor>(
                BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, HELPER_METHODS_PARAMETER_NAME, ~));
                
    }
    
    std::shared_ptr<DerivativeCollector> GetDerivativeCollector() const
    {
        return std::make_shared<LocalDerivativeCollector>(
                BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT, HELPER_METHODS_PARAMETER_NAME, ~));
    }
    
private:
    #define HELPER_METHODS_CREATE_PARAMETER_VARIABLE(z, n, data) \
        unsigned HELPER_METHODS_PARAMETER_NAME(z, n, data);
    BOOST_PP_REPEAT(NOBLE_PARAMETER_COUNT, HELPER_METHODS_CREATE_PARAMETER_VARIABLE, ~)
    
    #define HELPER_METHODS_REGISTER_PARAMETERS_ARGUMENTS(z, n, data) \
        const std::string& BOOST_PP_CAT(name, n),         \
        const std::string& BOOST_PP_CAT(unit, n),         \
        const double&      BOOST_PP_CAT(default, n),      \
        const double&      BOOST_PP_CAT(lowest_value , n),\
        const double&      BOOST_PP_CAT(highest_value, n),\
        const double&      BOOST_PP_CAT(highest_error, n)
    #define HELPER_METHODS_ARGUMENT_NAMES(z, n, data) \
        BOOST_PP_CAT(name, n),         \
        BOOST_PP_CAT(unit, n),         \
        BOOST_PP_CAT(default, n),      \
        BOOST_PP_CAT(lowest_value , n),\
        BOOST_PP_CAT(highest_value, n),\
        BOOST_PP_CAT(highest_error, n)
        
    constexpr static double NOBLE_INF()
    {
        return std::numeric_limits<double>::infinity();
    }
        
    void RegisterParameters(std::shared_ptr<ParameterManager> manager,
                            BOOST_PP_ENUM(NOBLE_PARAMETER_COUNT,
                                          HELPER_METHODS_REGISTER_PARAMETERS_ARGUMENTS,
                                          ~))
    {
        #define HELPER_METHODS_REGISTER_PARAMETERS_ASSIGNMENTS(z, n, data) \
            HELPER_METHODS_PARAMETER_NAME(z, n, ~) = \
                    manager->RegisterParameter( \
                        HELPER_METHODS_ARGUMENT_NAMES(z, n, ~));
                                          
        BOOST_PP_REPEAT(NOBLE_PARAMETER_COUNT, HELPER_METHODS_REGISTER_PARAMETERS_ASSIGNMENTS, ~)
    }

    class GeneratorDoubleHelper
    {
    public:
        GeneratorDoubleHelper() : val_(nullptr) {}
        void SetPointer(double* val) { val_ = val; }
        operator double() const { return *val_; }
        GeneratorDoubleHelper& operator =(double val) { *val_  = val; return *this; }
        GeneratorDoubleHelper& operator*=(double val) { *val_ *= val; return *this; }
        GeneratorDoubleHelper& operator/=(double val) { *val_ /= val; return *this; }
        GeneratorDoubleHelper& operator+=(double val) { *val_ += val; return *this; }
        GeneratorDoubleHelper& operator-=(double val) { *val_ -= val; return *this; }
    private:
        double* val_;
    };
    

#define DEFINE_PARAMETER_ACCESSOR(x, parameters) \
    std::shared_ptr<LocalParameterAccessor> x( \
            std::dynamic_pointer_cast<LocalParameterAccessor>(parameters))
            
#define DEFINE_DERIVATIVE_COLLECTOR(y, derivatives) \
    std::shared_ptr<LocalDerivativeCollector> y( \
            std::dynamic_pointer_cast<LocalDerivativeCollector>(derivatives))
            
#define DERIVATIVE_LOOP(parameter, derivative, derivatives) \
    DEFINE_DERIVATIVE_COLLECTOR(generator_local_y, derivatives); \
    const std::vector<std::pair<LocalParameter, double*> >& generator_local_infos = \
            generator_local_y->GetDerivativeInformation(); \
    LocalParameter parameter; \
    GeneratorDoubleHelper derivative; \
    if (!generator_local_infos.empty()) \
    { \
        parameter = generator_local_infos[0].first; \
        derivative.SetPointer(generator_local_infos[0].second); \
    } \
	bool go_on = generator_local_infos.cbegin() != generator_local_infos.cend(); \
    for (auto it = generator_local_infos.cbegin(); \
         go_on; \
         ++it, \
		     go_on = it != generator_local_infos.cend(), \
             parameter = go_on ? it->first : LocalParameter(), \
             derivative.SetPointer(go_on ? it->second : nullptr))
    
            
#undef NOBLE_PARAMETER_COUNT
#undef NOBLE_CLASS_PREFIX
