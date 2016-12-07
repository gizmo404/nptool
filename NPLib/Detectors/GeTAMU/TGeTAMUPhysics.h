#ifndef TGETAMUPHYSICS_H
#define TGETAMUPHYSICS_H
/*****************************************************************************
 * Copyright (C) 2009-2016    this file is part of the NPTool Project        *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: a.matta@surrey.ac.uk      *
 *                  Peter Bender  contact address: bender@triumf.ca          *
 * Creation Date  : November 2012                                            *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class hold GeTAMU treated data                                     *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *****************************************************************************/
#include <vector>
#include <map>
#include <string>
using namespace std;

// NPL
#include "TGeTAMUData.h"
#include "NPCalibrationManager.h"
#include "NPVDetector.h"
#include "NPInputParser.h"

// ROOT
#include "TObject.h"
#include "TVector3.h"
#include "TLorentzVector.h"

class TGeTAMUPhysics :  public TObject, public NPL::VDetector{

  public:
    TGeTAMUPhysics();
    ~TGeTAMUPhysics()  { };

    void Clear();
    void Clear(const Option_t*){Clear();}

  public: // inherited from VDetector
    //   Read stream at ConfigFile to pick-up parameters of detector (Position,...) using Token
    void ReadConfiguration(NPL::InputParser);

    //   Add Parameter to the CalibrationManger
    void AddParameterToCalibrationManager();      

    //   Activated associated Branches and link it to the private member DetectorData address
    //   In this method mother Branches (Detector) AND daughter leaf (fDetector_parameter) have to be activated
    void InitializeRootInputRaw() ;

    //   Activated associated Branches and link it to the private member DetectorPhysics address
    //   In this method mother Branches (Detector) AND daughter leaf (parameter) have to be activated
    void InitializeRootInputPhysics() {};

    //   Create associated branches and associated private member DetectorPhysics address
    void InitializeRootOutput() ;

    //   This method is called at each event read from the Input Tree. Aime is to build treat Raw dat in order to extract physical parameter. 
    void BuildPhysicalEvent() ;

    //   Same as above, but only the simplest event and/or simple method are used (low multiplicity, faster algorythm but less efficient ...).
    //   This method aimed to be used for analysis performed during experiment, when speed is requiered.
    //   NB: This method can eventually be the same as BuildPhysicalEvent.
    void BuildSimplePhysicalEvent(){BuildPhysicalEvent();} ;

    //   Clear the Event Physics
    void ClearEventPhysics() {Clear();}      
    void ClearEventData() ;

  public:
    void PreTreat();

  private:   //   Root Input and Output tree classes

    TGeTAMUData* m_EventData;//!
    TGeTAMUData* m_PreTreatedData;//!
    TGeTAMUPhysics* m_EventPhysics;//!

  public: // Data Member
    // add back by clover
    vector<double> AddBack_E;   
    vector<double> AddBack_T;   
    vector<double> AddBack_DC;   
    vector<double> AddBack_Theta;
    vector<double> AddBack_X;
    vector<double> AddBack_Y;
    vector<double> AddBack_Z;
    vector<int> AddBack_Clover;
    vector<int> AddBack_Crystal;
    vector<int> AddBack_Segment;

  private: // use for anlysis
   
    TLorentzVector m_GammaLV; //!
  public:
    TVector3 GetPositionOfInteraction(unsigned int& i);
    double GetDopplerCorrectedEnergy(double& energy , TVector3 position, TVector3& beta);
    // Add a detector and computes its coordinate
    void AddClover(unsigned int ID, double R, double Theta, double Phi);
    TVector3 GetCloverPosition(int& CloverNbr);
    TVector3 GetCorePosition(int& CloverNbr, int& CoreNbr);
    TVector3 GetSegmentPosition(int& CloverNbr, int& CoreNbr, int& SegmentNbr);
    inline TVector3 GetCrystalPosition(int& CloverNbr, int& CoreNbr){return GetCorePosition(CloverNbr,CoreNbr);};

  private:
    map<unsigned int,TVector3> m_CloverPosition;//!

  public: // Static constructor to be passed to the Detector Factory
    static NPL::VDetector* Construct();
    ClassDef(TGeTAMUPhysics,1)  // GeTAMUPhysics structure
};



#endif
