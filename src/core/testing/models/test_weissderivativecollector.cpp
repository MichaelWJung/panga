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


#include <boost/test/unit_test.hpp>

#include <boost/assign.hpp>

#include <Eigen/Core>

#include <stdexcept>

#include "core/models/weissmethod.h"

namespace
{
    struct Fixture
    {
        Fixture() {}
        ~Fixture() { if (w) delete w;}

        void Setup(
            unsigned p_param_index,
            unsigned S_param_index,
            unsigned T_param_index,
            const std::vector<int>& indices
            )
        {
            w = new WeissDerivativeCollector(
                        p_param_index,
                        S_param_index,
                        T_param_index
                        );
            d = w;
            derivatives.resize(indices.size());

            d->SetDerivativesAndResultsVector(derivatives, indices);
        }

        void StandardTest(
            int p_vec_pos,
            int S_vec_pos,
            int T_vec_pos
            )
        {
            const std::vector<std::pair<Weiss::Parameter, double*> >& infos =
                    w->GetDerivativeInformation();

            int p = -1;
            int S = -1;
            int T = -1;

            for (unsigned i = 0; i < infos.size(); ++i)
            {
                switch (infos[i].first)
                {
                case Weiss::p :
                    p = i;
                    break;
                case Weiss::S :
                    S = i;
                    break;
                case Weiss::T :
                    T = i;
                    break;
                default:
                    break;
                }
            }

            BOOST_REQUIRE_EQUAL(infos.size(), derivatives.size());

            if (p == -1)
                BOOST_CHECK_EQUAL(p_vec_pos, -1);
            else
                BOOST_CHECK_EQUAL(infos[p].second, &derivatives[p_vec_pos]);

            if (S == -1)
                BOOST_CHECK_EQUAL(S_vec_pos, -1);
            else
                BOOST_CHECK_EQUAL(infos[S].second, &derivatives[S_vec_pos]);

            if (T == -1)
                BOOST_CHECK_EQUAL(T_vec_pos, -1);
            else
                BOOST_CHECK_EQUAL(infos[T].second, &derivatives[T_vec_pos]);

        }

        WeissDerivativeCollector* w;
        DerivativeCollector* d;
        Eigen::RowVectorXd derivatives;
    };
}

BOOST_AUTO_TEST_SUITE(WeissDerivativeCollector_tests)

BOOST_AUTO_TEST_CASE(SetupWeissDerivativeCollector)
{
    WeissDerivativeCollector* w = new WeissDerivativeCollector(2, 0, 4);
    DerivativeCollector* d = w;

    std::vector<int> indices = boost::assign::list_of(1)(2)(0)(3)(4);

    Eigen::RowVectorXd derivatives(6);

    BOOST_CHECK_THROW(d->SetDerivativesAndResultsVector(derivatives, indices), std::invalid_argument);

    derivatives.resize(5);

    BOOST_CHECK_NO_THROW(d->SetDerivativesAndResultsVector(derivatives, indices));
}

BOOST_AUTO_TEST_CASE(ChangeResultsVector)
{
    WeissDerivativeCollector* w = new WeissDerivativeCollector(2, 0, 4);
    DerivativeCollector* d = w;

    std::vector<int> indices = boost::assign::list_of(1)(2)(0)(3)(4);
    Eigen::RowVectorXd d1(5);

    d->SetDerivativesAndResultsVector(d1, indices);

    indices = {7, 2, 4};
    Eigen::RowVectorXd d2(3);

    d->SetDerivativesAndResultsVector(d2, indices);

    const std::vector<std::pair<Weiss::Parameter, double*> >& infos = w->GetDerivativeInformation();

    unsigned p = 0;
    unsigned S = 0;
    unsigned T = 0;

    for (unsigned i = 0; i < infos.size(); ++i)
    {
        switch (infos[i].first)
        {
        case Weiss::p :
            p++;
            BOOST_CHECK_EQUAL(infos[i].second, &d2[1]);
            break;
        case Weiss::S :
            S++;
            break;
        case Weiss::T :
            T++;
            BOOST_CHECK_EQUAL(infos[i].second, &d2[2]);
            break;
        default:
            break;
        }
    }

    BOOST_CHECK_EQUAL(p, 1);
    BOOST_CHECK_EQUAL(S, 0);
    BOOST_CHECK_EQUAL(T, 1);
}

BOOST_FIXTURE_TEST_SUITE(GetDerivativeInformationSuite, Fixture)

BOOST_AUTO_TEST_CASE(GetDerivativeInformation1)
{
    Setup(0, 1, 2, boost::assign::list_of<int>(0)(1)(2));
    StandardTest(0, 1, 2);
}

BOOST_AUTO_TEST_CASE(GetDerivativeInformation2)
{
    Setup(2, 0, 5, boost::assign::list_of<int>(1)(2)(0)(3)(5)(4));
    StandardTest(1, 2, 4);
}

BOOST_AUTO_TEST_CASE(GetDerivativeInformation3)
{
    Setup(2, 0, 5, boost::assign::list_of<int>(0)(2));
    StandardTest(1, 0, -1);
}

BOOST_AUTO_TEST_CASE(GetDerivativeInformation4)
{
    Setup(2, 3, 1, boost::assign::list_of<int>(3)(0));
    StandardTest(-1, 0, -1);
}

BOOST_AUTO_TEST_CASE(GetDerivativeInformation5)
{
    Setup(2, 3, 8, boost::assign::list_of<int>(1)(0)(4)(5)(6)(7));
    StandardTest(-1, -1, -1);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
