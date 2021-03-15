#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <Eigen/Core>

#include <memory>

#include <stdexcept>

#include "core/models/parametermanager.h"
#include "core/models/jenkinsmethod.h"
#include "core/models/weissmethod.h"

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
        WeissMethod method;
        std::shared_ptr<ParameterAccessor> accessor;
        Eigen::VectorXd vec;

        double& T;
        double& S;
        double& p;
    };


    void test_ceqmethod(){
        Fixture fxt;
        fxt.T = 0;
        fxt.S = 1;
        fxt.p = 2;

        // Stuff for the Derivatives
        std::shared_ptr<DerivativeCollector> dcol(fxt.method.GetDerivativeCollector());
        Eigen::RowVectorXd derivatives(5);
        std::vector<int> indices;
        indices.push_back(17);
        indices.push_back(fxt.manager->GetParameterIndex("S"));
        indices.push_back(fxt.manager->GetParameterIndex("p"));
        indices.push_back(42);
        indices.push_back(fxt.manager->GetParameterIndex("T"));
        dcol->SetDerivativesAndResultsVector(derivatives, indices);


        // Test loops
        // WIP: loop über gase
        double xi;
        std::cout << "T" << std::endl;
        for(xi=0; xi<35; xi++) {
            fxt.T=xi;
            printf("%.3g ",fxt.T );
            Gas::GasType gas_ = Gas::StringToGasType("xe");
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, Gas::HE); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double b = 1E-08;
            fxt.T=xi+b;
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[4/*T*/]);
            std::cout << std::endl;
        }
        std::cout << "p" << std::endl;
        for(xi=0; xi<100; xi++) {
            fxt.p=xi/100;
            printf("%.3g ",fxt.T );
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, Gas::HE); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double b = 1E-08;
            fxt.p=xi/100+b;
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[2/*p*/]);
            std::cout << std::endl;
        }
        std::cout << "p" << std::endl;
        for(xi=0; xi<35; xi++) {
            fxt.S=xi;
            printf("%.3g ",fxt.T );
            fxt.method.CalculateDerivatives(fxt.accessor, dcol, Gas::HE); 
            double c1 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double b = 1E-08;
            fxt.S=xi+b;
            double c2 = fxt.method.CalculateConcentration(fxt.accessor, Gas::HE);
            double a = c2-c1;
            printf("%.20g ", a/b );
            printf("%.20g", derivatives[1/*S*/]);
            std::cout << std::endl;
        }
    }
    void test(){
        manualtesting::test_ceqmethod();
    }
}