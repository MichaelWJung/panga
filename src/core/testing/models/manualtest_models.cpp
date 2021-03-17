#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <Eigen/Core>

#include <memory>

#include <stdexcept>

#include "core/models/parametermanager.h"
#include "core/models/jenkinsmethod.h"
#include "core/models/weissmethod.h"
#include "core/models/physicalproperties.cpp"

namespace manualtesting
{
    struct Fixture
    {
        Fixture() :
            manager(new ParameterManager()), //WIP: Warum mit new einführen?
            method(manager),
            accessor(method.GetParameterAccessor()),
            vec(manager->GetParametersInOrder().size()),
            T(vec[manager->GetParameterIndex("T")]),
            S(vec[manager->GetParameterIndex("S")]),
            p(vec[manager->GetParameterIndex("p")])
        {
            accessor->SetVectorReference(vec);
        }

        ~Fixture()
        {
        }

        std::shared_ptr<ParameterManager> manager;
        JenkinsMethod method; //Change tested method
        std::shared_ptr<ParameterAccessor> accessor;
        Eigen::VectorXd vec;

        double& T;
        double& S;
        double& p;
    };
    void test_derivatives_ceqmethod(){
        Fixture fxt;
        fxt.T = 0;
        fxt.S = 1;
        fxt.p = 2;

        // Stuff for the Derivatives
        std::shared_ptr<DerivativeCollector> dcol(fxt.method.GetDerivativeCollector());
        Eigen::RowVectorXd derivatives(3);
        std::vector<int> indices;
        indices.push_back(fxt.manager->GetParameterIndex("p")); //Achtung Reihenfolge muss zu NOBLE_PARAMETER_NAMES passen?!
        indices.push_back(fxt.manager->GetParameterIndex("S"));
        indices.push_back(fxt.manager->GetParameterIndex("T"));
        dcol->SetDerivativesAndResultsVector(derivatives, indices);

        GasType usedgas = Gas::KR; // WIP: loop different gases
        
        double xi;
        std::cout << "Weiss/Jenkins T (Gas " << usedgas << ")" << std::endl;  //Param
        for(xi=0; xi<35; xi++) {
            double factor = 1.;
            fxt.T=xi*factor;   //Param
            printf("%.3g ",fxt.T ); //Param
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, usedgas); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double b = 1E-06;
            fxt.T=xi*factor+b; //Param
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[fxt.manager->GetParameterIndex("T")]);   //Param
            std::cout << std::endl;
        }
        std::cout << "Weiss/Jenkins p (Gas " << usedgas << ")" << std::endl; //Param
        for(xi=0; xi<100; xi++) {
            double factor = 1/100.;
            fxt.p=xi*factor;    //Param
            printf("%.3g ",fxt.p ); //Param
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, usedgas); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double b = 1E-08;
            fxt.p=xi*factor+b;  //Param
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[fxt.manager->GetParameterIndex("p")]);   //Param
            std::cout << std::endl;
        }
        std::cout << "Weiss/Jenkins S (Gas " << usedgas << ")" << std::endl; //Param
        for(xi=0; xi<35; xi++) {
            double factor = 1.;
            fxt.S=xi*factor;   //Param
            printf("%.5g ",fxt.S ); //Param
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, usedgas); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double b = 1E-08;
            fxt.S=xi+b; //Param
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, usedgas);

            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[fxt.manager->GetParameterIndex("S")]); //Param
            std::cout << std::endl;
        }
    }
    void printequilibriumconcentrations(){
        Fixture fxt;
        fxt.p = 1; 
        fxt.S = .1;
        fxt.T = 10;
        printf("%.15g ",fxt.method.CalculateConcentration(fxt.accessor, Gas::KR) ); std::cout << std::endl;
        fxt.p = 2;
        fxt.S = 1;
        fxt.T = 0;
        printf("%.15g ",fxt.method.CalculateConcentration(fxt.accessor, Gas::KR) ); std::cout << std::endl;
    }
    void test_derivatives_physicalproperties(){
        double xi;
        double s_=1.1;
        double p_=0.91;
        double t_=14.4;
        std::cout << "Dickson T" << std::endl;
        for(xi=-5; xi<50; xi++) {
            double b = 1E-06;
            double a = PhysicalProperties::CalcSaturationVaporPressure_Dickson(xi+b, s_)
            - PhysicalProperties::CalcSaturationVaporPressure_Dickson(xi, s_);
            double deriv = PhysicalProperties::CalcSaturationVaporPressureDerivedByT_Dickson(xi, s_);
            printf("%.20g ", a/b );
            printf("%.20g", deriv);
            std::cout << std::endl;
        }
        std::cout << "Dickson S" << std::endl;
        for(xi=-5; xi<50; xi++) {
            double b = 1E-06;
            double a = PhysicalProperties::CalcSaturationVaporPressure_Dickson(t_, xi+b)
            - PhysicalProperties::CalcSaturationVaporPressure_Dickson(t_, xi);
            double deriv = PhysicalProperties::CalcSaturationVaporPressureDerivedByS_Dickson(t_, xi);
            printf("%.20g ", a/b );
            printf("%.20g", deriv);
            std::cout << std::endl;
        }
        std::cout << "CalcReq T" << std::endl;
                for(xi=-5; xi<50; xi++) {
            double b = 1E-06;
            double a = PhysicalProperties::CalcReq(xi+b, s_)
            - PhysicalProperties::CalcReq(xi, s_);
            double deriv = PhysicalProperties::CalcReqDerivedByT(xi, s_);
            printf("%.20g ", a/b );
            printf("%.20g", deriv);
            std::cout << std::endl;
        }
    }
    // void printconcentrations(){
    //     double p_; double S_; double T_; double A; double F; double c;
    //     p_ = 1; 
    //     S_ = .1;
    //     T_ = 10;
    //     A = .01;
    //     F = .2; 
    //     c = 1.31506883127e-08  ;
    //     printf("%.15g", c+(1-F)*A*PhysicalProperties::GetDryAirVolumeFraction(Gas::KR)/(1+F*A*PhysicalProperties::GetDryAirVolumeFraction(Gas::KR)/c)); std::cout << std::endl;

    //     p_ = 2;
    //     S_ = 1;
    //     T_ = 0;
    //     A = 3;
    //     F = 4;
    //     c = 2.47039861698971e-07 ;
    //     printf("%.15g", c+(1-F)*A*PhysicalProperties::GetDryAirVolumeFraction(Gas::KR)/(1+F*A*PhysicalProperties::GetDryAirVolumeFraction(Gas::KR)/c)); std::cout << std::endl;
    // }   

    void test(){
        manualtesting::test_derivatives_ceqmethod();
        test_derivatives_physicalproperties();
        // printequilibriumconcentrations();
        // printconcentrations();
    }
}