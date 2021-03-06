#ifndef __PLASTICDATA__
#define __PLASTICDATA__
/*****************************************************************************
 * Copyright (C) 2009-2016    this file is part of the NPTool Project        *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author:    contact address:                                      *
 *                                                                           *
 * Creation Date  :                                                          *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *                                                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *****************************************************************************/
#include <vector>

#include "TObject.h"
using namespace std ;


class TPlasticData : public TObject {
 private:
   // ADC
   vector<double>  fPlastic_Energy;   
   vector<double>  fPlastic_Time ;
   vector<short>   fPlastic_Number ;
   
 public:
   TPlasticData();
   virtual ~TPlasticData();

   void   Clear();
   void   Clear(const Option_t*) {};
   void   Dump() const;

   /////////////////////           GETTERS           ////////////////////////
   // (E)
   inline double   GetEnergy(const int& i) const { return fPlastic_Energy[i] ;}
   // (T)
   inline double   GetTime(const int& i)   const { return fPlastic_Time[i] ;}
   // (N)
   inline int      GetPlasticNumber(const int& i) const { return fPlastic_Number[i] ;}
   
   //Mult
   // E
   inline double   GetEnergyMult() const { return fPlastic_Energy.size() ;}
   // (T)
   inline double   GetTimeMult()   const { return fPlastic_Time.size() ;}
   // (N)
   inline int      GetPlasticNumberMult() const { return fPlastic_Number.size() ;}
   
   /////////////////////           SETTERS           ////////////////////////
   // (E)
   inline void   SetEnergy(const double& E)     { fPlastic_Energy.push_back(E) ;}
   inline void   SetTime(const double&  T)      { fPlastic_Time.push_back(T) ;}
   inline void   SetPlasticNumber(const int& N) { fPlastic_Number.push_back(N) ;}
   //
   ClassDef(TPlasticData,1)  // PlasticData structure
};

#endif
