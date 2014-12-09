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


#include <QFileDialog>
#include "montecarloexportercsv.h"

MonteCarloExporterCsv::MonteCarloExporterCsv() :
    file_(),
    stream_(),
    is_new_line_(true)
{
    QString filename = QFileDialog::getSaveFileName(
            nullptr,
            "Export Monte Carlo data",
            QString(),
            "CSV file (*.csv)",
            nullptr
#ifdef __APPLE__
            ,QFileDialog::DontUseNativeDialog
#endif
            );
    if (filename.isEmpty())
        throw Canceled();
    file_.setFileName(filename);
    file_.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    stream_.setDevice(&file_);
}

void MonteCarloExporterCsv::NextSample()
{
    stream_ << '\n';
    is_new_line_ = true;
}

MonteCarloExporter& MonteCarloExporterCsv::operator<<(QVariant in)
{
    if (is_new_line_)
        is_new_line_ = false;
    else
        stream_ << ',';
    stream_ << in.toString();
    return *this;
}
