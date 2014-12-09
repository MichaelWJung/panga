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

set(models_TESTS
    testmain.cpp
    test_cemodel.cpp
    test_ceqcalculationmethod.cpp
    test_combinedmodel.cpp
    test_combinedmodelfactory.cpp
    test_diffusioncoefficientinair.cpp
    test_parametermanager.cpp
    test_physicalproperties.cpp
    test_clevermethod.cpp
    test_weissderivativecollector.cpp
    test_weissmethod.cpp
    test_weissparameteraccessor.cpp
    )

add_executable(test_models ${models_TESTS})

target_link_libraries(test_models core ${LIBRARIES} ${TEST_LIBRARIES})

qt5_use_modules(test_models Core)

add_test(NAME Models COMMAND test_models)
