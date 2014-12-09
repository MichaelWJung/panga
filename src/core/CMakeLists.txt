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

set(core_SOURCES
    fitting/defaultfitter.cpp
    fitting/fitconfiguration.cpp 
    fitting/fitparameterconfig.cpp
    fitting/fitresults.cpp
    fitting/levenbergmarquardtfitter.cpp
    fitting/montecarlocontroller.cpp
    fitting/noblefitfunction.cpp
    fitting/nobleparametermap.cpp
    models/clevermethod.cpp
    models/combinedmodel.cpp
    models/combinedmodelfactory.cpp
    models/derivativecollector.cpp
    models/diffusioncoefficientinair.cpp
    models/modelmanager.cpp
    models/parametermanager.cpp
    models/physicalproperties.cpp
    models/weissmethod.cpp
    misc/rundata.cpp
 
    models/cemodel.cpp
    models/cemodelfactory.cpp
    models/grmodel.cpp
    models/grmodelfactory.cpp
    models/odmodel.cpp
    models/odmodelfactory.cpp
    models/pdmodel.cpp
    models/pdmodelfactory.cpp
    models/prmodel.cpp
    models/prmodelfactory.cpp
    models/uamodel.cpp
    models/uamodelfactory.cpp
    )

include_directories(
    ..
    ${Boost_INCLUDE_DIRS}
    )

add_definitions(${DEFINITIONS})
add_library(core ${core_SOURCES})
target_link_libraries(core ${LIBRARIES})
qt5_use_modules(core Widgets)

if(TESTING_ENABLED)
    add_subdirectory(testing)
endif(TESTING_ENABLED)
