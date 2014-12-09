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


#include <algorithm>

#include "montecarlocontroller.h"

MonteCarloController::MonteCarloController(
        const std::vector<long unsigned>& n_monte_carlos) :
    n_monte_carlos_(n_monte_carlos),
    current_fit_(0),
    n_done_current_fit_(0),
    queue_(),
    condition_(),
    mutex_()
{
    auto first_fit_with_monte_carlos =
            std::find_if(n_monte_carlos_.cbegin(), n_monte_carlos_.cend(),
                         [](unsigned i){return i > 0;});

    current_fit_ = first_fit_with_monte_carlos - n_monte_carlos_.cbegin();
}

std::shared_ptr<boost::promise<std::shared_ptr<FitResults> > >
        MonteCarloController::GetNewJob(unsigned& fit_index)
{
    boost::lock_guard<boost::mutex> lock(mutex_);

    if (!JobsLeft())
    {
        throw NoJobsLeft();
    }

    fit_index = current_fit_;

    if (++n_done_current_fit_ >= n_monte_carlos_[current_fit_])
    {
        current_fit_ = std::find_if(n_monte_carlos_.begin() + current_fit_ + 1, n_monte_carlos_.end(),
                                    [](unsigned i){return i > 0;})
                - n_monte_carlos_.begin();
        n_done_current_fit_ = 0;
    }

    std::shared_ptr<boost::promise<std::shared_ptr<FitResults> > > promise(
                std::make_shared<boost::promise<
                        std::shared_ptr<FitResults> > >());

    std::shared_ptr<boost::unique_future<std::shared_ptr<FitResults> > > future(
            new boost::unique_future<std::shared_ptr<FitResults> >(promise->get_future()));

    queue_.push_back(std::make_pair(fit_index, future));

    condition_.notify_one();

    return promise;
}

std::pair<unsigned, std::shared_ptr<FitResults> >
MonteCarloController::GetNextResult()
{
    std::shared_ptr<boost::unique_future<std::shared_ptr<FitResults> > >
            results;
    unsigned fit_index;
    {
        boost::unique_lock<boost::mutex> lock(mutex_);

        if (!ResultsLeft())
            throw NoResultsLeft();

        while(queue_.empty())
            condition_.wait(lock);

        fit_index = queue_.front().first;
        results = queue_.front().second;
        queue_.pop_front();
    }

    return std::make_pair(fit_index, results->get());
}

bool MonteCarloController::JobsLeft() const
{
    return current_fit_ < n_monte_carlos_.size();
}

bool MonteCarloController::ResultsLeft() const
{
    return !queue_.empty() || JobsLeft();
}
