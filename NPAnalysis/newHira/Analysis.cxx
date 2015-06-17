/*****************************************************************************
 * Copyright (C) 2009-2014    this file is part of the NPTool Project        *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Pierre Morfouace contact address:                        *
 *                                                                           *
 * Creation Date  : march 2025                                               *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
#include<iostream>
using namespace std;
#include"Analysis.h"
#include"NPAnalysisFactory.h"
#include"NPDetectorManager.h"
#include"NPOptionManager.h"
////////////////////////////////////////////////////////////////////////////////
Analysis::Analysis(){
}
////////////////////////////////////////////////////////////////////////////////
Analysis::~Analysis(){
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::Init(){
  InitOutputBranch();
  InitInputBranch();

  Hira= (THiraPhysics*)  m_DetectorManager -> GetDetector("HIRAArray");
  LightCD2 = EnergyLoss("proton_CD2.G4table","G4Table",100 );
  LightAl = EnergyLoss("proton_Al.G4table","G4Table",100);
  LightSi = EnergyLoss("proton_Si.G4table","G4Table",100);
  //BeamCD2 = EnergyLoss("Rb88_CD2.G4table","G4Table",100);
  myReaction = new NPL::Reaction();
  myReaction->ReadConfigurationFile(NPOptionManager::getInstance()->GetReactionFile());
  TargetThickness = m_DetectorManager->GetTargetThickness()*micrometer;
  OriginalBeamEnergy = myReaction->GetBeamEnergy();
  Rand = TRandom3();

  E_ThinSi = 0;
  E_ThickSi = 0;
  E_CsI = 0;
  ELab = 0;
  ThetaLab = 0;
  PhiLab = 0;
  ThetaLab_simu = 0;
  ThetaCM = 0;
  ExcitationEnergy = 0;
  X = 0;
  Y = 0;
  Z = 0;
  TelescopeNumber = 0;
  EnergyThreshold = 1;

}

////////////////////////////////////////////////////////////////////////////////
void Analysis::TreatEvent(){
  // Reinitiate calculated variable
  ReInitValue();
  if(Hira -> ThickSi_E.size() == 1){
    for(int countHira = 0 ; countHira < Hira->ThickSi_E.size() ; countHira++){

      TelescopeNumber = Hira->TelescopeNumber[countHira];

      TargetThickness = m_DetectorManager->GetTargetThickness();

      X = Hira->GetPositionOfInteraction(0).X();
      Y = Hira->GetPositionOfInteraction(0).Y();
      Z = Hira->GetPositionOfInteraction(0).Z();

      TVector3 PositionOnHira = TVector3(X,Y,Z);
      TVector3 ZUnit = TVector3(0,0,1);

      double X_target	= InitialConditions->GetIncidentPositionX();
      double Y_target = InitialConditions->GetIncidentPositionY();
      double Z_target	= InitialConditions->GetIncidentPositionZ();

      TVector3 PositionOnTarget = TVector3(X_target,Y_target,Z_target);
      TVector3 HitDirection	= PositionOnHira - PositionOnTarget;
      TVector3 HitDirectionUnit = HitDirection.Unit();

      TVector3 BeamDirection = InitialConditions->GetBeamDirection();
      double XBeam = BeamDirection.X();
      double YBeam = BeamDirection.Y();
      double ZBeam = BeamDirection.Z();

      ThetaLab = BeamDirection.Angle(HitDirection);
      ThetaLab = ThetaLab/deg;

      PhiLab = HitDirection.Phi()/deg;

      E_ThickSi = Hira->ThickSi_E[countHira];
      E_ThinSi = Hira->ThinSi_E[countHira];
      //ELab = E_ThinSi + E_ThickSi;
      if(Hira->CsI_E.size() == 1){
        for(int countCsI =0; countCsI<Hira->CsI_E.size(); countCsI++){
          E_CsI = Hira->CsI_E[countCsI];
          //ELab += E_CsI;
          if(E_CsI>EnergyThreshold) ELab = E_ThinSi + E_ThickSi + E_CsI;
        }
      }
      else ELab = -1000;

      // ********************** Angle in the CM frame *****************************
      myReaction -> SetNuclei3(ELab, ThetaLab*deg);
      ThetaCM          = myReaction->GetThetaCM()/deg;
      ExcitationEnergy	= myReaction->GetExcitation4();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
void Analysis::End(){
}
////////////////////////////////////////////////////////////////////////////////
void Analysis::InitOutputBranch() {

  RootOutput::getInstance()->GetTree()->Branch( "ThetaLab" , &ThetaLab , "ThetaLab/D" );
  RootOutput::getInstance()->GetTree()->Branch( "PhiLab" , &PhiLab , "PhiLab/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch("ThetaCM", &ThetaCM,"ThetaCM/D") 	;
  RootOutput::getInstance()->GetTree()->Branch( "E_ThinSi" , &E_ThinSi , "E_ThinSi/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "E_ThickSi" , &E_ThickSi , "E_ThickSi/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "E_CsI" , &E_CsI , "E_CsI/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "ELab" , &ELab , "ELab/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch("ExcitationEnergy", &ExcitationEnergy,"ExcitationEnergy/D") ;
  RootOutput::getInstance()->GetTree()->Branch( "X" , &X , "X/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "Y" , &Y , "Y/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "Z" , &Z , "Z/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch( "TelescopeNumber" , &TelescopeNumber , "TelescopeNumber/D" )  ;
  RootOutput::getInstance()->GetTree()->Branch("InteractionCoordinates","TInteractionCoordinates",&InteractionCoordinates);
  RootOutput::getInstance()->GetTree()->Branch("InitialConditions","TInitialConditions",&InitialConditions);

}
////////////////////////////////////////////////////////////////////////////////
void Analysis::InitInputBranch(){
  //TInteractionCoordinate
  InteractionCoordinates = new TInteractionCoordinates();
  RootInput::getInstance()->GetChain()->SetBranchStatus("InteractionCoordinates",true);
  RootInput::getInstance()->GetChain()->SetBranchStatus("fDetected_*",true);
  RootInput::getInstance()->GetChain()->SetBranchAddress("InteractionCoordinates",&InteractionCoordinates);
  
  //TInitialConditions
  InitialConditions = new TInitialConditions();
  RootInput::getInstance()->GetChain()->SetBranchStatus("InitialConditions",true);
  RootInput::getInstance()->GetChain()->SetBranchStatus("fIC_*",true);
  RootInput::getInstance()->GetChain()->SetBranchAddress("InitialConditions",&InitialConditions);
}
////////////////////////////////////////////////////////////////////////////////
void Analysis::ReInitValue(){
  E_ThinSi    = -1000;
  E_ThickSi   = -1000;
  E_CsI       = -1000;
  ELab        = -1000;
  ThetaLab    = -1000;
  PhiLab      = -1000;
  ThetaCM     = -1000;
  ExcitationEnergy = -1000;
  X           = -1000;
  Y           = -1000;
  Z           = -1000;
  TelescopeNumber = -1;
}


////////////////////////////////////////////////////////////////////////////////
//            Construct Method to be pass to the AnalysisFactory              //
////////////////////////////////////////////////////////////////////////////////
NPL::VAnalysis* Analysis::Construct(){
  return (NPL::VAnalysis*) new Analysis();
}

////////////////////////////////////////////////////////////////////////////////
//            Registering the construct method to the factory                 //
////////////////////////////////////////////////////////////////////////////////
extern "C"{
class proxy{
  public:
    proxy(){
      NPL::AnalysisFactory::getInstance()->SetConstructor(Analysis::Construct);
    }
};

proxy p;
}

