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


#ifndef RUNDATA_H
#define RUNDATA_H

#include <QObject>

#define BOOST_RESULT_OF_USE_DECLTYPE
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/serialization/access.hpp>

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "data.h"
#include "gas.h"
#include "typedefs.h"

//! Speichert einen Satz von Proben.
/*!
 * Probennamen werden zusammen mit Gaskonzentrationen gespeichert.
 */
class RunData : public QObject
{
    Q_OBJECT
    
    class ReturnConcentrations;
    class ReturnEnabled;
    
    struct Tuple
    {
        Tuple() : enabled(true) {}
        Tuple(std::string&& name, SampleConcentrations&& concentrations) :
            name(std::move(name)),
            concentrations(std::move(concentrations)),
            enabled(true)
        {
        }
        Tuple(const Tuple&) = default;
        
        std::string name;
        SampleConcentrations concentrations;
        bool enabled;
        
    private:
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive& ar, const unsigned version);
    };
    
    typedef std::vector<Tuple> VectorType;
    typedef boost::filter_iterator<ReturnEnabled, VectorType::const_iterator>
            FilterIterator;
    
public:
    
    RunData();
    RunData(const RunData& other);
    RunData& operator=(const RunData& other);
    
    //! Probe hinzufügen.
    /*!
     * \warning Die Konzentrationen werden nach RunData verschoben, der übergebene Pointer
     * zeigt danach auf eine leere map.
     */
    void Add(Sample&& sample);
    
    size_t EnabledSize() const;
    size_t TotalSize() const;
    
    typedef boost::transform_iterator<ReturnConcentrations, FilterIterator>
            const_concentrations_iterator;
    
    //! Gibt einen Iterator der über alle Konzentrations-Maps iteriert zurück.
    /*!
     * Dabei werden deaktivierte Proben jedoch übersprungen.
     */
    const_concentrations_iterator concentrations_begin() const;
    
    //! Gibt einen Iterator der über alle Konzentrations-Maps iteriert zurück.
    /*!
     * Dabei werden deaktivierte Proben jedoch übersprungen.
     */
    const_concentrations_iterator concentrations_end() const;
    
    //! Gibt die Namen aller aktivierten Proben zurück.
    std::vector<std::string> GetEnabledSampleNames() const;
    
    //! Gibt die Namen aller (auch deaktivierter) Proben zurück.
    std::vector<std::string> GetAllSampleNames() const;
    
    //! Gibt einen bestimmten Probennamen zurück.
    const std::string& GetSampleName(unsigned index) const;
    
    //! Gibt die Gaskonzentrationen einer bestimmten Probe zurück.
    /*!
     * \warning Übergeht deaktivierte Proben \b nicht.
     */
    const SampleConcentrations& GetSampleConcentrations(unsigned index) const;
    
    //! Entfernt ein Gas aus allen Proben.
    void RemoveGas(GasType gas);
    
    void DisableSample(unsigned index);
    void EnableSample(unsigned index);
    
    bool IsEnabled(unsigned index) const;
    
    void EnableAllSamples();
    void DisableAllSamples();
    
    void clear();

signals:
    void SampleEnabled(unsigned index);
    void SampleDisabled(unsigned index);
        
private:
    //! Funktor, der die Konzentrationen zurückgibt.
    class ReturnConcentrations :
    public std::unary_function<const Tuple&, const SampleConcentrations&>
    {
    public:
        const SampleConcentrations& operator()(const Tuple& in) const;
    };
    
    //! Funktor, der zurückgibt, ob die Probe aktiviert oder deaktiviert ist.
    class ReturnEnabled : public std::unary_function<const Tuple&, bool>
    {
    public:
        bool operator()(const Tuple& in) const;
    };
    
    VectorType data_;
    
    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& ar, const unsigned);
};

template<class Archive>
void RunData::Tuple::serialize(Archive& ar, const unsigned version)
{
    ar & name
       & concentrations
       & enabled;
}
        

template<class Archive>
void RunData::serialize(Archive& ar, const unsigned)
{
    ar & data_;
}
        
#endif //RUNDATA_H