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


#ifndef PHYSICALPROPERTIES_H
#define PHYSICALPROPERTIES_H

#include <map>
#include <vector>

#include "core/misc/gas.h"

class PhysicalProperties
{
public:

    //! Berechnet den Sättigungsdampfdruck von Wasser.
    /*!
      Berechnung nach Gill 1982, Atmosphere-Ocean Dynamics:
      \f[
        log_{10}(e_w(T))=\frac{0.7859+0.03477\cdot T}{1+0.00412\cdot T}
      \f]
      \f$e_w\f$: Sättigungsdampfdruck von Wasser in mbar.\n
      \f$T\f$: Temperatur in °C.
      \param T_c Temperatur in °C.
      \return Sättigungsdampfdruck in atm.
      */
    static double CalcSaturationVaporPressure_Gill(double T_c);
    //! Berechnet den Sättigungsdampfdruck von Wasser.
    /*!
      Berechnung nach Dickson 2007, Guide to Best Practices for Ocean CO2 Measurements. 
      Basierend auf einem Skript von Roberta Hamme (vpress Version 2.0 : 27 October 2012).
      */
    static double CalcSaturationVaporPressure_Dickson(double T_c, double S);

    //! Berechnet die Ableitung des \ref CalcSaturationVaporPressure_Gill "Sättigungsdampfdrucks" von Wasser.
    static double CalcSaturationVaporPressureDerivative_Gill(double T_c);
    //! Berechnet die Ableitungen des \ref CalcSaturationVaporPressure_Dickson "Sättigungsdampfdrucks" von Wasser.
    static double CalcSaturationVaporPressureDerivedByT_Dickson(double T_c, double S);
    static double CalcSaturationVaporPressureDerivedByS_Dickson(double T_c, double S);

    //! Berechnet die Dichte von Wasser für die gegebenen Bedingungen.
    /*!
      \param p Druck in atm.
      \param S Salinität in g/kg.
      \param T_c Temperatur in °C.
      \return Dichte des Wasser für die gegebenen Bedingungen in kg/m^3.
      */
    static double CalcWaterDensity(double p, double S, double T_c);

    //! Berechnet die Mole Fraction Solubility von Xenon.
    /*!
      Aus Clever 1979, Solubility %Data Series Volume 2: Krypton, Xenon and Radon - %Gas Solubilities:
      \f[
        ln(X_{Xe}) = -74.7398+\frac{105.210}{\frac{T}{100}}+27.4664\cdot ln\left(\frac{T}{100}\right)
      \f]

      \f$X_{Xe}\f$: Mole fraction solubility von Xenon (einheitenlos).\n
      \f$T\f$: Temperatur in K.
      \param T_c Temperatur in °C.
      \return Mole fraction solubility von Xenon (einheitenlos).
      */
    static double CalcXeMoleFractionSolubility(double T_c);

    //! Berechnet den Salting Coefficient von Xenon.
    /*!
      Aus Smith and Kennedy 1983, The solubility of noble gases in water and in NaCl brine. Geochimica
      et Cosmochimica Acta Vol. 47:
      \f[
        K_{Xe}=-14.1338+\frac{21.8772}{\frac{T}{100}}+6.5527\cdot ln\left(\frac{T}{100}\right)
      \f]

      \f$K_{Xe}\f$: Salting Coefficient von Xenon in l/mol (Einheit unsicher).\n
      \f$T\f$: Temperatur in K.
      \param T_c Temperatur in °C.
      \return Salting Coefficient von Xenon in l/mol (Einheit unsicher).
      */
    static double CalcXeSaltingCoefficient(double T_c);

    //! Wandelt eine Gasmenge von ccSTP in mol um.
    /*!
      \param ccstp Gasmenge in ccSTP.
      \param gas Gas, für das die Umwandlung durchgeführt werden soll.
      \return Gasmenge in mol.
      */
    static double ConvertToMole(double ccstp, GasType gas);

    //! Wandelt Gaskonzentrationen im Wasser von ccSTP/g in mol/l um.
    /*!
      Da das Wasservolumen keine Stoffmenge angibt müssen Druck, Salinität und Temperatur mit angegeben werden.
      \param ccstp_g Konzentration in ccSTP/g.
      \param p Druck in atm.
      \param S Salinität in g/kg.
      \param T_c Temperatur in °C.
      \param gas Gas, für das die Umwandlung durchgeführt werden soll.
      \return Konzentration in mol/l.
      */
    static double ConvertToMolePerLiter(double ccstp_g, double p, double S, double T_c, GasType gas);

    //! Gibt den Volumenanteil des übergebenen Gases in trockener Luft zurück.
    static double GetDryAirVolumeFraction(GasType gas);
    
    //! Gibt den Diffusionskoeffizienten in Wasser, normiert auf den Neon-Koeffizienten in m²/s zurück.
    /*!
     * \param T_c Temperatur in °C.
     */
    static double GetDiffusionCoefficientInWater(double T_c, GasType gas);
    
    //! Gibt die Ableitung nach T des auf Neon normierten Diffusionskoeffizienten in Wasser zurück.
    /*!
     * \param T_c Temperatur in °C.
     */    
    static double GetDiffusionCoefficientInWaterDerivative(double T_c,
                                                           GasType gas);
    
    //! Berechnet das 3He/4He-Verhältnis.
    /*!
     * aus SOLUB NG MASTER_WAH.xls
     */
    static double CalcReq(double t, double s);
    
    //! Ableitung von Req nach T.
    static double CalcReqDerivedByT(double t, double s);
    
    //! Ableitung von Req nach S.
    static double CalcReqDerivedByS(double t, double s);

    //! Molvolumina der Edelgase zur Umrechnung von Jenkins und Clever Löslichkeiten.
    /*!
     * based on Dymond and Smith, 1980
     * in ccSTP/mol
     */
    static double GetMolarVolume(GasType gas);

//     //! Gaskonstante in atm*l/(mol*K).
//     static const double R;


private:

    //! Berechnet die Molvolumina der Edelgase.
    /*!
      Aus CRC Handbook of Chemistry and Physics 76th Edition 1995-1996, Seiten 6-28ff.

      Berechnet nach den Formeln:
      \f[
        B = \sum_{i=1}^na(i)\left[\frac{T_0}{T}-1\right]^{i-1}
      \f]
      \f[
        V_m = V_{m,i} + B
      \f]
      \f$B\f$: Zweiter Virialkoeffizient in cm^3 mol^-1.\n
      \f$a(i)\f$: Die in virial_coefficient_parameters_ definierten Koeffizienten.\n
      \f$T_0\f$: 298,15K.\n
      \f$T\f$: Temperatur für die der Koeffizient berechnet werden soll. Hier: 273,15K.\n
      \f$V_m\f$: Molvolumen des Gases.\n
      \f$V_{m,i}\f$: Molvolumen eines idealen Gases.
      \param a Die in virial_coefficient_parameters_ definierten Koeffizienten.
      \return Molvolumina in l/mol.
      */
    static std::map<GasType, double> CalculateMolarVolumes(
        const std::map<GasType, std::vector<double> >& a
        );

    //! Konstante der Gill-Formel.
    static const double c1_;

    //! Konstante der Gill-Formel.
    static const double c2_;

    //! Konstante der Gill-Formel.
    static const double c3_;

    //! Parameter zur Berechnung der Virialkoeffizienten zu einer bestimmten Temperatur.
    /*!
      Aus CRC Handbook of Chemistry and Physics 91st Edition 2010-2011, Seiten 6-46ff.
      \todo Konstanten nochmal nachprüfen.
      */
    static const std::map<GasType, std::vector<double> > virial_coefficient_parameters_;

    //! Molvolumen eines idealen Gases in cm^3/mol.
    /*!
      Aus CRC Handbook of Chemistry and Physics 76th Edition 1995-1996, Seite 1-4.
      */
    static const double molar_volume_ideal_gas_;

    //! Molvolumina der Edelgase, berechnet mit CalculateMolarVolumes.
    static const std::map<GasType, double> molar_volumes_;
    
    //! Molvolumina der Edelgase, basierend auf Dymond and Smith (1980).
    static const std::map<GasType, double> molar_volumes_new;

    //! Volumenanteile der Edelgase in trockener Luft.
    static const std::map<GasType, double> dry_air_volume_fractions_;
    
    //! Konstanten, die zur Berechnung des Diffusionskoeffizienten in Luft benötigt werden.
    static const std::map<GasType, std::pair<double, double>>
        diffusion_coefficients_in_water_constants_;
    
    //! Gaskonstante in J/(mol*K).
    static const double R;
    
    //! Konstante zur Berechnung von Req.
    static const double r1_;
    
    //! Konstante zur Berechnung von Req.
    static const double r2_;
    
    //! Konstante zur Berechnung von Req.
    static const double r3_;
    
    //! Konstante zur Berechnung von Req.
    static const double r4_;
    
    //! Konstante zur Berechnung von Req.
    static const double r5_;
};

#endif // PHYSICALPROPERTIES_H
