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

        GasType usedgas = Gas::HE3; // WIP: loop different gases

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
    void printconcentrations(){
        double p_ = 2;
        double T_ = 0;
        double S_ = 1;
        double b = 1 /(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))
        )*(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))
        );
        printf("%.15g", 9.77748738846232e-08 * b);  std::cout << std::endl;
        printf("%.15g", 4.48172212527456e-07 * b); std::cout << std::endl;
        printf("%.15g", 9.91230382674763e-04 * b); std::cout << std::endl;
        printf("%.15g", 2.47039748481394e-07 * b); std::cout << std::endl;
        printf("%.15g",   1.328780665567e-13 * b); std::cout << std::endl;

        T_ = 10;
        S_ = .1;
        p_ = 1;
        b = 1 /(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))
        )*(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))
        );        printf("%.15g", 4.64269706836073e-08 * b); std::cout << std::endl;
        printf("%.15g", 2.01619787901304e-07 * b); std::cout << std::endl;
        printf("%.15g", 3.85849249750526e-04 * b); std::cout << std::endl;
        printf("%.15g", 9.09654355930234e-08 * b); std::cout << std::endl;
        printf("%.15g",   6.315021552700E-14 * b); std::cout << std::endl;


        T_ = 20;
        S_ = 0;
        p_ = 1.01;
        b = 1 /(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Gill(T_))
        )*(
            (p_-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))/(1-PhysicalProperties::CalcSaturationVaporPressure_Dickson(T_, S_))
        );
        printf("%.15g", 4.52241473452391e-08 * b); std::cout << std::endl;
        printf("%.15g", 1.86998524534805e-07 * b); std::cout << std::endl;
        printf("%.15g", 3.15103538968795e-04 * b); std::cout << std::endl;
        printf("%.15g", 7.03793929300997e-08 * b); std::cout << std::endl;
        printf("%.15g",   6.156710522179E-14 * b); std::cout << std::endl;
    }
    void test(){
        // manualtesting::test_derivatives_ceqmethod();
        // test_derivatives_physicalproperties();
        printconcentrations();
    }
}