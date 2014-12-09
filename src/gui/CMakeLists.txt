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

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in"
               "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)

set(gui_SOURCES
    chi2explorer.cpp
    chi2parameterconfigmodel.cpp
    chi2parameterslider.cpp
    colormap.cpp
    columntypelistmodel.cpp
    concentrationsheaderview.cpp
    concentrationsmodel.cpp
    concentrationsview.cpp
    contourplot.cpp
    contourplotdata.cpp
    contourplotfitter.cpp
    datavector.cpp
    doubleeditorfactory.cpp
    ensemblefitconfigurationdialog.cpp
    ensembleresultsprocessorproxy.cpp
    fitsetup.cpp
    fitsetupwidget.cpp
    fittingthread.cpp
    guiresultsprocessor.cpp
    headercombobox.cpp
    headercomboboxlistview.cpp
    histogramdata.cpp
    histogramdata1d.cpp
    histogramdata2d.cpp
    histogramplot.cpp
    histogramplot1d.cpp
    histogramplot2d.cpp
    histogramplotzoomer.cpp
    logarithmicslider.cpp
    mainwindow.cpp
    mask.cpp
    montecarloeditor.cpp
    montecarloexportercsv.cpp
    montecarloplotdelegate.cpp
    montecarloresultsmodel.cpp
    montecarloresultsview.cpp
    montecarlosummaryproxymodel.cpp
    parametercheckbox.cpp
    parametersetupheaderview.cpp
    parametersetupmodel.cpp
    parametersetupview.cpp
    resultsmodel.cpp
    resultsview.cpp
    resultswindow.cpp
    setupmontecarloplotsdialog.cpp
    sharedbinnumber.cpp
    standardfitresultsmodel.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/version.cpp"
    )

set(gui_FORMS
    chi2explorer.ui
    chi2parameterslider.ui
    fitsetupwidget.ui
    mainwindow.ui
    montecarloeditor.ui
    resultswindow.ui
    setupmontecarloplotsdialog.ui
    )

qt5_wrap_ui(gui_FORMS_HEADERS ${gui_FORMS})

include_directories(
    ..
    ${Boost_INCLUDE_DIRS}
    ${QWT_INCLUDE_DIR}
    )

add_library(gui
    ${gui_SOURCES}
    ${gui_FORMS_HEADERS}
    )

qt5_use_modules(gui Widgets)

add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 main.cpp)
target_link_libraries(
    ${PROJECT_NAME}
    gui
    core
    ${LIBRARIES}
    )

qt5_use_modules(${PROJECT_NAME} Widgets)

if(APPLE)
    set(EXECUTABLE "${PROJECT_NAME}.app")
    install(TARGETS ${PROJECT_NAME} BUNDLE DESTINATION "\${CMAKE_INSTALL_BINDIR}")
    install(FILES "${Qt5Core_DIR}/../../../plugins/platforms/libqcocoa.dylib" DESTINATION "${EXECUTABLE}/Contents/Plugins/platforms")
    install(CODE "include(BundleUtilities)
                  set(PREFIX \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
                  file(WRITE \"\${PREFIX}${EXECUTABLE}/Contents/Resources/qt.conf\" \"[Paths]\nPlugins = Plugins\")
                  fixup_bundle(\"\${PREFIX}${EXECUTABLE}\" \"\${PREFIX}${EXECUTABLE}/Contents/PlugIns/platforms/libqcocoa.dylib\" \"\")")
elseif(WIN32)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
    install(FILES "${Qt5Core_DIR}/../../../plugins/platforms/qwindows.dll" DESTINATION "plugins/platforms")
    set(EXECUTABLE "${PROJECT_NAME}.exe")
    get_target_property(QT_CORELIB Qt5::Core LOCATION)
    get_filename_component(QT_DIR ${QT_CORELIB} PATH)
    get_filename_component(QWT_DIR ${QWT_LIBRARY} PATH)
    install(CODE "include(BundleUtilities)
                  set(PREFIX \$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})
                  fixup_bundle(\"\${PREFIX}${EXECUTABLE}\" \"\" \"${QT_DIR};${QWT_DIR}\")
                  file(WRITE \"\${PREFIX}qt.conf\" \"[Paths]\nPlugins = plugins\")")
endif()


if(TESTING_ENABLED)
    add_subdirectory(testing)
endif(TESTING_ENABLED)
