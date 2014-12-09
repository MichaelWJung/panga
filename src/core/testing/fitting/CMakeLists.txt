# Copyright © 2014 Michael Jung
#
# This file is part of Panga.
# 
# Panga is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Panga is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Panga.  If not, see <http://www.gnu.org/licenses/>.

set(CMAKE_INCLUDE_CURRENT_DIR TRUE)

include_directories(
    ../..
    )

set(fitting_TESTS
    testmain.cpp
    test_fitparameterconfig.cpp
    test_fitresults.cpp
    test_nobleparametermap.cpp
    )

add_executable(test_fitting ${fitting_TESTS})
target_link_libraries(test_fitting core ${LIBRARIES} ${TEST_LIBRARIES})
qt5_use_modules(test_fitting Core)

add_test(NAME Fitting COMMAND test_fitting)
