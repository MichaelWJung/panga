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


#ifndef MONTECARLOCONTROLLER_H
#define MONTECARLOCONTROLLER_H

#include <boost/thread.hpp>
#include <memory>

#include <deque>
#include <vector>

#include "levenbergmarquardtfitter.h"

//! Klasse die die Verteilung der Monte-Carlo-Berechnungen auf die Arbeiter-Threads kontrolliert.
class MonteCarloController
{
public:

    //! Konstruktor.
    /*!
      \param n_monte_carlos Zahl der für jeden Fit durchzuführenden Monte-Carlo-Simulationen.
      */
    MonteCarloController(const std::vector<unsigned long>& n_monte_carlos);

    //! Vergibt einen neuen Berechnungsjob.
    /*!
      \return Promise, in das die Ergebnisse geschrieben werden müssen.
      \post \a fit_index enthält den Index des Fits, für den eine Monte-Carlo-Berechnung durchgeführt
        werden muss.
      */
    std::shared_ptr<boost::promise<std::shared_ptr<FitResults>>>
            GetNewJob(unsigned& fit_index);

    //! Gibt das jeweils nächste Fitergebniss zurück.
    /*!
      Blockiert den aktuellen Thread, falls dieses noch nicht vorliegt.
      */
    std::pair<unsigned, std::shared_ptr<FitResults>> GetNextResult();

    class NoJobsLeft{};
    class NoResultsLeft{};


private:

    //! Gibt wahr zurück, solange noch Berechnungen abzuarbeiten sind.
    bool JobsLeft() const;

    //! Gibt wahr zurück, solange noch Berechnungsergebnisse abgeholt werden können.
    bool ResultsLeft() const;

    //! Zahl der Monte-Carlo-Berechnungen für die jeweiligen Fits.
    const std::vector<unsigned long> n_monte_carlos_;

    //! Gibt an, für welchen Fit gerade die Monte-Carlo-Berechnungen durchgeführt werden.
    unsigned long current_fit_;

    //! Gibt an, wie viele Monte-Carlo-Berechnungen für diesen Fit bereits durchgeführt wurden.
    unsigned long n_done_current_fit_;

    //! Hier werden die Berechnungsergebnisse zwischengespeichert.
    std::deque<std::pair<unsigned,
                         std::shared_ptr<boost::unique_future<
                                 std::shared_ptr<FitResults>>>
                         > > queue_;

    //! Wird zum Aufwecken des auf die Ergebnisse wartenden Threads benutzt, sobald neue vorliegen.
    boost::condition_variable condition_;

    //! Mutex für den Zugriff auf queue_.
    boost::mutex mutex_;
};

#endif // MONTECARLOCONTROLLER_H
