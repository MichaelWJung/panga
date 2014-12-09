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
#include <boost/test/floating_point_comparison.hpp>

#include "parametersetupmodel.h"

namespace
{
struct FixtureWithSamples
{
    FixtureWithSamples() :
        sample_names({"A", "B"})
    {
        model.SetSampleNames(sample_names);
    }
    
    ParameterSetupModel model;
    std::vector<std::string> sample_names;
};
}

BOOST_FIXTURE_TEST_SUITE(ParameterSetupModelTests1, FixtureWithSamples)

BOOST_AUTO_TEST_CASE(rowCount_2SamplesAdded_Return2)
{
    BOOST_CHECK_EQUAL(model.rowCount(), 2);
}

BOOST_AUTO_TEST_CASE(columnCount_3ParametersAdded_Return3)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.AddParameter("z");
    BOOST_CHECK_EQUAL(model.columnCount(), 3);
}

BOOST_AUTO_TEST_CASE(columnCount_3ParametersAdded1Removed_Return3)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.AddParameter("z");
    model.RemoveParameter("x");
    BOOST_CHECK_EQUAL(model.columnCount(), 3);
}

BOOST_AUTO_TEST_CASE(columnCount_2ParametersAdded1Removed1Added_Return2)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.RemoveParameter("x");
    model.AddParameter("z");
    BOOST_CHECK_EQUAL(model.columnCount(), 2);
}

BOOST_AUTO_TEST_CASE(headerData_EditRole_ReturnInvalid)
{
    BOOST_CHECK(model.headerData(0, Qt::Vertical, Qt::DecorationRole)
            == QVariant());
}
BOOST_AUTO_TEST_CASE(headerData_2SamplesAdded_ReturnSampleNames)
{
    BOOST_CHECK_EQUAL(model.headerData(0,Qt::Vertical).toString().toStdString(),
                      "A");
    BOOST_CHECK_EQUAL(model.headerData(1,Qt::Vertical).toString().toStdString(),
                      "B");
}

BOOST_AUTO_TEST_CASE(headerData_RequestTooHighLineNumber_ReturnInvalid)
{
    BOOST_CHECK(model.headerData(2, Qt::Vertical) == QVariant());
}

BOOST_AUTO_TEST_CASE(headerData_HorizontalSmallerThanParaCount_ReturnParameter)
{
    model.AddParameter("x");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "x");
}

BOOST_AUTO_TEST_CASE(headerData_HorizontalHigherThanParaCount_ReturnInvalid)
{
    model.AddParameter("x");
    BOOST_CHECK(model.headerData(1, Qt::Horizontal) == QVariant());
}

BOOST_AUTO_TEST_CASE(setHeaderData_SmallerThanParaCount_ReturnTrue)
{
    model.AddParameter("x");
    BOOST_CHECK_EQUAL(model.SetHeaderParameter(0, "x"), true);
}

BOOST_AUTO_TEST_CASE(setHeaderData_HigherThanParaCount_ReturnFalse)
{
    model.AddParameter("x");
    BOOST_CHECK_EQUAL(model.SetHeaderParameter(1, "x"), false);
}

BOOST_AUTO_TEST_CASE(setHeaderData_NameWhichDoesntExist_ReturnFalse)
{
    model.AddParameter("x");
    BOOST_CHECK_EQUAL(model.SetHeaderParameter(0, "y"), false);
}

BOOST_AUTO_TEST_CASE(setHeaderData_EmptyString_ReturnTrue)
{
    model.AddParameter("x");
    BOOST_CHECK_EQUAL(model.SetHeaderParameter(0, ""), true);
}

BOOST_AUTO_TEST_CASE(headerData_HeaderSetToValues_ReturnTheseValues)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.SetHeaderParameter(0, "x");
    model.SetHeaderParameter(1, "y");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "x");
    BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().
            toStdString(), "y");
}

BOOST_AUTO_TEST_CASE(headerData_Add2ParametersRemove1_RemovedInHeaderToo)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.SetHeaderParameter(0, "x");
    model.SetHeaderParameter(1, "y");
    model.RemoveParameter("x");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "");
    BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().
            toStdString(), "y");
    model.RemoveParameter("y");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "");
    BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().
            toStdString(), "");
}

BOOST_AUTO_TEST_CASE(headerData_SetAlreadyExistingParameter_SwitchPlaces)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.SetHeaderParameter(0, "x");
    model.SetHeaderParameter(1, "y");
    model.SetHeaderParameter(1, "x");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "y");
    BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().
            toStdString(), "x");
}

BOOST_AUTO_TEST_CASE(headerData_SetToEmptyString_DontSwitchPlaces)
{
    model.AddParameter("x");
    model.AddParameter("y");
    model.SetHeaderParameter(0, "");
    BOOST_CHECK_EQUAL(model.headerData(0, Qt::Horizontal).toString().
            toStdString(), "");
    BOOST_CHECK_EQUAL(model.headerData(1, Qt::Horizontal).toString().
            toStdString(), "y");
}

BOOST_AUTO_TEST_SUITE_END()

namespace
{
struct FixtureWithSamplesAndParameters
{
    FixtureWithSamplesAndParameters()
    {
        model.SetSampleNames({"A", "B"});
        model.AddParameter("x");
        model.AddParameter("y");
    }
    
    void FillWithSimpleData()
    {
        model.SetHeaderParameter(0, "x");
        model.SetHeaderParameter(1, "y");
        model.setData(model.index(0, 0), 1.);
        model.setData(model.index(0, 1), 2.);
        model.setData(model.index(1, 0), 3.);
        model.setData(model.index(1, 1), 4.);
    }
    
    ParameterSetupModel model;
};
}

BOOST_FIXTURE_TEST_SUITE(ParameterSetupModelTests2,
                         FixtureWithSamplesAndParameters)

BOOST_AUTO_TEST_CASE(data_RequestOutOfRange_ReturnInvalid)
{
    BOOST_CHECK(model.data(model.index(0, 2)) == QVariant());
}

BOOST_AUTO_TEST_CASE(data_RequestInRangeOnNewObject_Return0)
{
    bool ok = true;
    BOOST_CHECK_CLOSE(model.data(model.index(1, 1)).toDouble(&ok), 0, 1e-10);
    BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE(data_DataSetTo1_Return1)
{
    model.setData(model.index(0, 1), 1.);
    bool ok = true;
    BOOST_CHECK_CLOSE(model.data(model.index(0, 1)).toDouble(&ok), 1, 1e-10);
    BOOST_CHECK_EQUAL(ok, true);
}

BOOST_AUTO_TEST_CASE(setData_IndexOutOfRange_ReturnFalse)
{
    BOOST_CHECK_EQUAL(model.setData(model.index(2, 1), 1.), false);
}

BOOST_AUTO_TEST_CASE(GetValue_DataSet_ReturnData)
{
    model.SetHeaderParameter(0, "y");
    model.SetHeaderParameter(1, "x");
    model.setData(model.index(0, 1), 1.);
    model.setData(model.index(1, 0), 2.);
    
    BOOST_CHECK_CLOSE(model.GetValue("x", 0), 1., 1e-10);
    BOOST_CHECK_CLOSE(model.GetValue("y", 1), 2., 1e-10);
}

BOOST_AUTO_TEST_CASE(GetValue_RequestInvalidParameter_ThrowInvalidArgumentError)
{
    model.SetHeaderParameter(0, "y");
    model.SetHeaderParameter(1, "x");

    BOOST_CHECK_THROW(model.GetValue("z", 0),
                      ParameterSetupModel::ParameterNotInHeaderError);
}

BOOST_AUTO_TEST_CASE(GetValue_RequestInvalidSample_ThrowOutOfRangeError)
{
    model.SetHeaderParameter(0, "y");
    model.SetHeaderParameter(1, "x");

    BOOST_CHECK_THROW(model.GetValue("x", 2), std::out_of_range);
}

BOOST_AUTO_TEST_CASE(GetValue_RequestDisabledSample_ReturnNextSample)
{
    FillWithSimpleData();
    
    model.DisableSample(0);
    BOOST_CHECK_CLOSE(model.GetValue("x", 0), 3., 1e-10);
    BOOST_CHECK_CLOSE(model.GetValue("y", 0), 4., 1e-10);
}

BOOST_AUTO_TEST_CASE(GetValue_RequestDisabledSampleWhereNoNextSample_ThrowOutOfRangeError)
{
    FillWithSimpleData();
    
    model.DisableSample(0);
    BOOST_CHECK_THROW(model.GetValue("x", 1), std::out_of_range);
    BOOST_CHECK_THROW(model.GetValue("y", 1), std::out_of_range);
    
    model.DisableSample(1);
    BOOST_CHECK_THROW(model.GetValue("x", 0), std::out_of_range);
    BOOST_CHECK_THROW(model.GetValue("y", 0), std::out_of_range);
}

BOOST_AUTO_TEST_SUITE_END()