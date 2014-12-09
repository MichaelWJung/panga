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

#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include "serializationhelpers.h"

BOOST_AUTO_TEST_SUITE(EigenClassesSerializations_tests)

BOOST_AUTO_TEST_CASE(MatrixSerialization)
{

    Eigen::MatrixXd mat1(2, 4);
    mat1(0, 0) = 1;
    mat1(0, 1) = 2;
    mat1(0, 2) = 3;
    mat1(0, 3) = 4;
    mat1(1, 0) = 5;
    mat1(1, 1) = 6;
    mat1(1, 2) = 7;
    mat1(1, 3) = 8;
    {
        std::ofstream ofs("eigentestmat.txt");
        boost::archive::text_oarchive oa(ofs);
        oa << mat1;
    }
    Eigen::MatrixXd mat2(3, 1);
    {
        std::ifstream ifs("eigentestmat.txt");
        boost::archive::text_iarchive ia(ifs);
        ia >> mat2;
    }

    BOOST_CHECK_EQUAL(mat1, mat2);
}

BOOST_AUTO_TEST_CASE(VectorSerialization)
{
    Eigen::VectorXd vec1(3);
    vec1[0] = 7;
    vec1[1] = 3;
    vec1[2] = 23;
    {
        std::ofstream ofs("eigentestvec.txt");
        boost::archive::text_oarchive oa(ofs);
        oa << vec1;
    }
    Eigen::VectorXd vec2;
    {
        std::ifstream ifs("eigentestvec.txt");
        boost::archive::text_iarchive ia(ifs);
        ia >> vec2;
    }
    
    BOOST_CHECK_EQUAL(vec1, vec2);
}


    
BOOST_AUTO_TEST_SUITE_END()