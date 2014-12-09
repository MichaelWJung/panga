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


#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE GUI
#include <boost/test/unit_test.hpp>

#include <gmock/gmock.h>

struct InitGMock {
    InitGMock() {
        ::testing::GTEST_FLAG(throw_on_failure) = true;
        ::testing::InitGoogleMock(&boost::unit_test::framework::master_test_suite().argc,
                                  boost::unit_test::framework::master_test_suite().argv);
    }
    ~InitGMock() { }
};
BOOST_GLOBAL_FIXTURE(InitGMock);

// #include <QApplication>
//
// int main(int argc, char* argv[])
// {
//     QApplication app(argc, argv);
//
//     std::locale::global(std::locale::classic());
//
//     return ::boost::unit_test::unit_test_main(&init_unit_test, argc, argv);
// }
