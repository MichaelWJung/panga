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


#ifndef RANDOMNUMBERBUFFER_H
#define RANDOMNUMBERBUFFER_H

#include <vector>
#include <boost/random.hpp>
#include <boost/thread.hpp>

#include "randomnumbergenerator.h"

class RandomNumberBuffer
{
public:
    RandomNumberBuffer(RandomNumberGenerator& generator) :
        generator_(generator),
        variate_generator_(generator, boost::normal_distribution<>()),
        buffer_(BUFFER_SIZE)
    {
        RefillBuffer();
    }

    double operator()()
    {
        if (iterator_ == buffer_.end())
            RefillBuffer();

        return *iterator_++;
    }

private:
    void RefillBuffer()
    {
        {
            boost::lock_guard<boost::mutex> lock(generator_.mutex);
            for (auto it = buffer_.begin(); it != buffer_.end(); ++it)
                *it = variate_generator_();
        }

        iterator_ = buffer_.begin();
    }

    static const int BUFFER_SIZE = 1000;

    RandomNumberGenerator& generator_;
    boost::variate_generator<RandomNumberGenerator&,
                             boost::normal_distribution<> > variate_generator_;

    std::vector<double> buffer_;
    std::vector<double>::iterator iterator_;
};

#endif // RANDOMNUMBERBUFFER_H
