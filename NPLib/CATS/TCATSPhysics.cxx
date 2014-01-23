/*****************************************************************************
 * Copyright (C) 2009-2010   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Sandra Giron  contact address: giron@ipno.in2p3.fr       *
 *                                                                           *
 * Creation Date  : febuary 2010                                             *
 * Last update    : modification november 2011 by Pierre Morfouace			 *
 * Contact adress : morfouac@ipno.in2p3.fr									 *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class hold CATS treated data                                        *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *****************************************************************************/

#include "TCATSPhysics.h"
using namespace LOCAL_CATS;

//	STL
#include <cmath>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <set>
using namespace std;
//	NPL
#include "RootInput.h"
#include "RootOutput.h"
//	ROOT
#include "TChain.h"
#include "TRandom.h"

ClassImp(TCATSPhysics)

  ///////////////////////////////////////////////////////////////////////////
  TCATSPhysics::TCATSPhysics(){
    m_EventData 				= new TCATSData	;
    m_PreTreatedData    = new TCATSData ;
    m_EventPhysics 			= this			    ;
    m_NumberOfCATS      = 0             ;
  }

///////////////////////////////////////////////////////////////////////////
TCATSPhysics::~TCATSPhysics(){
}

///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::PreTreat(){
  ClearPreTreatedData();   
  gRandom->SetSeed(0);
  // X
  unsigned int sizeX = m_EventData->GetCATSMultX();
  for(unsigned int i = 0 ; i < sizeX ; i++){
    // Valid Channel X
    if(IsValidChannel("X", m_EventData->GetCATSDetX(i), m_EventData->GetCATSStripX(i)) ){
      if( fCATS_Threshold_X(m_EventData , i) ){
        double QX = fCATS_X_Q(m_EventData , i);
        m_PreTreatedData->SetCATSChargeX( QX );
        //Inversion X
        if( *(m_CATSXInversion[m_EventData->GetCATSDetX(i)-1].begin() + m_EventData->GetCATSStripX(i)-1) != m_EventData->GetCATSStripX(i) ){
          m_PreTreatedData->SetCATSStripX( *(m_CATSXInversion[m_EventData->GetCATSDetX(i)-1].begin() + m_EventData->GetCATSStripX(i)-1) );
        }
        else {
          m_PreTreatedData->SetCATSStripX( m_EventData->GetCATSStripX(i) );
        }
        m_PreTreatedData->SetCATSDetX( m_EventData->GetCATSDetX(i) );
      }
    }
  }

  // Y
  unsigned int sizeY = m_EventData->GetCATSMultY();
  for(unsigned int i = 0 ; i < sizeY ; i++){
    // Valid Channel Y
    if(IsValidChannel("Y", m_EventData->GetCATSDetY(i), m_EventData->GetCATSStripY(i))){
      if( fCATS_Threshold_Y(m_EventData , i) ){
        double QY = fCATS_Y_Q(m_EventData , i);
        m_PreTreatedData->SetCATSChargeY( QY );
        //Inversion Y
        if( *(m_CATSYInversion[m_EventData->GetCATSDetY(i)-1].begin() + m_EventData->GetCATSStripY(i)-1) != m_EventData->GetCATSStripY(i) ){
          m_PreTreatedData->SetCATSStripY( *(m_CATSYInversion[m_EventData->GetCATSDetY(i)-1].begin() + m_EventData->GetCATSStripY(i)-1) );
        }
        else {
          m_PreTreatedData->SetCATSStripY( m_EventData->GetCATSStripY(i) );
        }
        m_PreTreatedData->SetCATSDetY( m_EventData->GetCATSDetY(i) );
      }
    }
  }
  return;
}

/////////////////////////////////////////////////////////////////////////////
void TCATSPhysics::BuildSimplePhysicalEvent(){
  BuildPhysicalEvent();
}

//////////////////////////////////////////////////////////////////////////////		
void TCATSPhysics::BuildPhysicalEvent(){
  PreTreat();
  double Pi = 3.14159265;

  // Look how many CATS were fired
  // use a set to identify which detector has been hit
  set<int> DetectorHit;
  unsigned int sizeX = m_PreTreatedData->GetCATSMultX() ;
  for( unsigned short i = 0 ; i < m_PreTreatedData->GetCATSMultX() ; i++ ){ 
    // Insert detector number in the set, if the key already exist, do nothing
    DetectorHit.insert(m_PreTreatedData->GetCATSDetX(i));
  }
  
  unsigned int sizeY = m_PreTreatedData->GetCATSMultY() ;
  for( unsigned short i = 0 ; i < m_PreTreatedData->GetCATSMultY() ; i++ ){ 
    // Insert detector number in the set, if the key already exist, do nothing
    DetectorHit.insert(m_PreTreatedData->GetCATSDetY(i));
  }
  // The number of CATS hit, i.e. the number of CATS that we are going to analyse
  unsigned int NumberOfCATSHit = DetectorHit.size();

   vector<double> ChargeArray;
   ChargeArray.resize(28,-1);

  // INITIALISATION OF VECTORS : DIM = NumberOfCATSHit
  for(set<int>::iterator it=DetectorHit.begin(); it!=DetectorHit.end(); ++it){
    // Assign order for the detector number
    DetMaxX.push_back(*it);
    DetMaxY.push_back(*it);
    // X
    StripMaxX.push_back(-1); 
    ChargeMaxX.push_back(-1);
    QsumX.push_back(0);
    // Y
    StripMaxY.push_back(-1); 
    ChargeMaxY.push_back(-1);
    QsumY.push_back(0);
  
    Buffer_X_Q.push_back(ChargeArray);
    Buffer_Y_Q.push_back(ChargeArray);  
  }
  
  // Fill up the Charge And Max field for X
  int HitX = 0 ;
  for(unsigned int i = 0 ; i < sizeX ; i++ ){
    int StrX					         = m_PreTreatedData->GetCATSStripX(i);
    int NX						         = m_PreTreatedData->GetCATSDetX(i);
    double CATS_X_Q				     = m_PreTreatedData->GetCATSChargeX(i) ;
    ChargeX.push_back(CATS_X_Q);
    StripX.push_back(StrX);
    DetNumberX.push_back(NX);
        for(unsigned int j = 0 ; j < NumberOfCATSHit ; j++){
      if(NX == DetMaxX[j] ){
        Buffer_X_Q[j][StrX-1]= CATS_X_Q;
        QsumX[j]+= CATS_X_Q;	
        HitX++;
        if(HitX==1) {
           StripMaxX[j] = StrX;
        } 
        else if(CATS_X_Q > Buffer_X_Q[j][StripMaxX[j] -1]){ 
          StripMaxX[j] = StrX ; 
          ChargeMaxX[j]= CATS_X_Q; 
        }
      }
    }
  }
  
  // Fill up the Charge And Max field for Y
  int HitY = 0;
  for(unsigned int i = 0 ; i < sizeY ; i++ ){
    int StrY					         = m_PreTreatedData->GetCATSStripY(i);
    int NY						         = m_PreTreatedData->GetCATSDetY(i);
    double CATS_Y_Q				     = m_PreTreatedData->GetCATSChargeY(i) ;
    ChargeY.push_back(CATS_Y_Q);
    StripY.push_back(StrY);
    DetNumberY.push_back(NY);
        for(unsigned int j = 0 ; j < NumberOfCATSHit ; j++){
      if(NY == DetMaxY[j] ){
        Buffer_Y_Q[j][StrY-1]= CATS_Y_Q;
        QsumY[j]+= CATS_Y_Q;	
        HitY++;
        if(HitY==1) {
           StripMaxY[j] = StrY;
        } 
        else if(CATS_Y_Q > Buffer_Y_Q[j][StripMaxY[j] -1]){ 
          StripMaxY[j] = StrY ; 
          ChargeMaxY[j]= CATS_Y_Q; 
        }
      }
    }
  }
  
  double CalculatedStripX = 0, CalculatedStripY = 0;
  double posX = 0 , posY = 0;
  
  for(unsigned int i  = 0 ; i < NumberOfCATSHit ; i++ ){       
     // Return the position in strip unit
    double PosX =  ReconstructionFunctionX[DetMaxX[i]-1](Buffer_X_Q[i],StripMaxX[i]);
    double PosY =  ReconstructionFunctionY[DetMaxY[i]-1](Buffer_Y_Q[i],StripMaxY[i]);
      
    // Correct the position
    // Convert it in mm
    int floorX = (int) PosX;
    int floorY = (int) PosY;
    // If Reconstruction are successfull then floorX and floorY should be a valid strip number 
    if(floorX > -1 && floorX < 28 && floorY>-1 && floorY <28){ 
    PositionX.push_back(StripPositionX[DetMaxX[i]-1][floorX-1][floorY-1]+(PosX-floorX)*2.54);
    PositionY.push_back(StripPositionY[DetMaxX[i]-1][floorX-1][floorY-1]+(PosY-floorY)*2.54);
    PositionZ.push_back(StripPositionZ[DetMaxX[i]-1]);
    }
  }
  
  // At least two CATS need to gave back position in order to reconstruct on Target 
  if(PositionX.size()>1){
      double PositionOnTargetX_1;
      double PositionOnTargetY_1;
      double l = sqrt((PositionZ[0]-PositionZ[1])*(PositionZ[0]-PositionZ[1]));
      double L = - PositionZ[1];
      double t = (l+L) / l;
      PositionOnTargetX_1 = PositionX[0] + (PositionX[1] - PositionX[0]) * t ;
      PositionOnTargetY_1 = PositionY[0] + (PositionY[1] - PositionY[0]) * t ;
      if(m_TargetAngle != 0){
        double a = (PositionZ[1]-PositionZ[0])/(PositionX[1]-PositionX[0]);
        double b = PositionZ[0] - a*PositionX[0];
        PositionOnTargetX = b/(tan(m_TargetAngle*Pi/180.) - a);
        double t_new = (l + L + PositionOnTargetX*tan(m_TargetAngle*Pi/180.)) / l;
        PositionOnTargetY = PositionY[0] + (PositionY[1] - PositionY[0]) * t_new ;
      }

      else{
        PositionOnTargetX = PositionOnTargetX_1;
        PositionOnTargetY = PositionOnTargetY_1;
      }
      GetPositionOnTarget();
      GetBeamDirection();

  }
  
  // Does not meet the conditions for target position and beam direction 
  else{
      BeamDirection = TVector3 (1,0,0);
      PositionOnTargetX = -1000	;
      PositionOnTargetY = -1000	;
    }
  return;
}

///////////////////////////////////////////////////////////////////////////
//	Read stream at ConfigFile to pick-up parameters of detector (Position,...) using Token
void TCATSPhysics::ReadConfiguration(string Path){
  ifstream ConfigFile;
  ConfigFile.open(Path.c_str());
  string LineBuffer          		;
  string DataBuffer          		;

  double Ax , Bx , Cx , Dx , Ay , By , Cy , Dy , Az , Bz , Cz , Dz    	;
  TVector3 A , B , C , D                                          	;

  bool check_A = false 	;
  bool check_B = false  	;
  bool check_C = false 	;
  bool check_D = false 	;

  bool ReadingStatus = false ;


  while (!ConfigFile.eof()) 
  {
    getline(ConfigFile, LineBuffer);

    //If line is a Start Up CATS bloc, Reading toggle to true      
    if (LineBuffer.compare(0, 12, "CATSDetector") == 0) 
    {
      cout << "CATS Detector found: " << endl   ;  
      ReadingStatus = true 	       ;
    }

    //	Else don't toggle to Reading Block Status
    else ReadingStatus = false ;

    //	Reading Block
    while(ReadingStatus)
    {
      ConfigFile >> DataBuffer ;
      //	Comment Line 
      if(DataBuffer.compare(0, 1, "%") == 0) {
        ConfigFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );
      }

      //	Finding another telescope (safety), toggle out
      else if (DataBuffer.compare(0, 12, "CATSDetector") == 0) {
        cout << "Warning: Another CATS is found before standard sequence of Token, Error may have occurred in CATS definition" << endl ;
        ReadingStatus = false ;
      }

      //	Corner Position method

      else if (DataBuffer.compare(0, 6, "X1_Y1=") == 0) {
        check_A = true;
        ConfigFile >> DataBuffer ;
        Ax = atof(DataBuffer.c_str()) ;
        Ax = Ax  ;
        ConfigFile >> DataBuffer ;
        Ay = atof(DataBuffer.c_str()) ;
        Ay = Ay  ;
        ConfigFile >> DataBuffer ;
        Az = atof(DataBuffer.c_str()) ;
        Az = Az  ;

        A = TVector3(Ax, Ay, Az);
        cout << " X1 Y1 corner position : (" << A.X() << ";" << A.Y() << ";" << A.Z() << ")" << endl;
      }

      else if (DataBuffer.compare(0, 7, "X28_Y1=") == 0) {
        check_B = true;
        ConfigFile >> DataBuffer ;
        Bx = atof(DataBuffer.c_str()) ;
        Bx = Bx  ;
        ConfigFile >> DataBuffer ;
        By = atof(DataBuffer.c_str()) ;
        By = By  ;
        ConfigFile >> DataBuffer ;
        Bz = atof(DataBuffer.c_str()) ;
        Bz = Bz  ;

        B = TVector3(Bx, By, Bz);
        cout << " X28 Y1 corner position : (" << B.X() << ";" << B.Y() << ";" << B.Z() << ")" << endl;
      }

      else if (DataBuffer.compare(0, 7, "X1_Y28=") == 0) {
        check_C = true;
        ConfigFile >> DataBuffer ;
        Cx = atof(DataBuffer.c_str()) ;
        Cx = Cx  ;
        ConfigFile >> DataBuffer ;
        Cy = atof(DataBuffer.c_str()) ;
        Cy = Cy  ;
        ConfigFile >> DataBuffer ;
        Cz = atof(DataBuffer.c_str()) ;
        Cz = Cz  ;

        C = TVector3(Cx, Cy, Cz);
        cout << " X1 Y28 corner position : (" << C.X() << ";" << C.Y() << ";" << C.Z() << ")" << endl;
      }

      else if (DataBuffer.compare(0, 8, "X28_Y28=") == 0) {
        check_D = true;
        ConfigFile >> DataBuffer ;
        Dx = atof(DataBuffer.c_str()) ;
        Dx = Dx  ;
        ConfigFile >> DataBuffer ;
        Dy = atof(DataBuffer.c_str()) ;
        Dy = Dy  ;
        ConfigFile >> DataBuffer ;
        Dz = atof(DataBuffer.c_str()) ;
        Dz = Dz  ;

        D = TVector3(Dx, Dy, Dz);
        cout << " X28 Y28 corner position : (" << D.X() << ";" << D.Y() << ";" << D.Z() << ")" << endl;

      }

      //	End Corner Position Method

      /////////////////////////////////////////////////
      //	If All necessary information there, toggle out
      if (check_A && check_B && check_C && check_D)  
      { 
        ReadingStatus = false; 

        ///Add The previously define telescope

        AddCATS(	A   ,
            B   ,
            C   ,
            D   );

        check_A = false;
        check_B = false;
        check_C = false;
        check_D = false;
      }
    }  

  }
  InitializeStandardParameter();
  ReadAnalysisConfig();
}

/////////////////////////////////////////////////////////////////////
//	Activated associated Branches and link it to the private member DetectorData address
//	In this method mother Branches (Detector) AND daughter leaf (fDetector_parameter) have to be activated
void TCATSPhysics::InitializeRootInputRaw() {
  TChain* inputChain = RootInput::getInstance()->GetChain()	;
  inputChain->SetBranchStatus( "CATS" , true )			;
  inputChain->SetBranchStatus( "fCATS_*" , true )		;
  inputChain->SetBranchAddress( "CATS" , &m_EventData )           ;
}

/////////////////////////////////////////////////////////////////////
//   Activated associated Branches and link it to the private member DetectorPhysics address
//   In this method mother Branches (Detector) AND daughter leaf (parameter) have to be activated
void TCATSPhysics::InitializeRootInputPhysics() {
  TChain* inputChain = RootInput::getInstance()->GetChain();
  inputChain->SetBranchStatus( "CATS" , true );
  inputChain->SetBranchStatus( "ff" , true );
  inputChain->SetBranchStatus( "DetNumberX" , true );
  inputChain->SetBranchStatus( "StripX" , true );
  inputChain->SetBranchStatus( "ChargeX" , true );
  inputChain->SetBranchStatus( "StripMaxX" , true );
  inputChain->SetBranchStatus( "DetNumberY" , true );
  inputChain->SetBranchStatus( "StripY" , true );
  inputChain->SetBranchStatus( "ChargeY" , true );
  inputChain->SetBranchStatus( "StripMaxY" , true );
  inputChain->SetBranchStatus( "DetNumber_PositionX" , true );
  inputChain->SetBranchStatus( "DetNumber_PositionY" , true );
  inputChain->SetBranchStatus( "DetNumber_PositionZ" , true );
  inputChain->SetBranchStatus( "PositionX" , true );
  inputChain->SetBranchStatus( "PositionY" , true );
  inputChain->SetBranchStatus( "PositionZ" , true );
  inputChain->SetBranchStatus( "PositionOnTargetX" , true );
  inputChain->SetBranchStatus( "PositionOnTargetY" , true );
  inputChain->SetBranchStatus( "QsumX" , true );
  inputChain->SetBranchStatus( "QsumY" , true );
  inputChain->SetBranchAddress( "CATS" , &m_EventPhysics );

}

/////////////////////////////////////////////////////////////////////
//	Create associated branches and associated private member DetectorPhysics address
void TCATSPhysics::InitializeRootOutput(){
  TTree* outputTree = RootOutput::getInstance()->GetTree()		;
  outputTree->Branch( "CATS" , "TCATSPhysics" , &m_EventPhysics )	;
}

/////////////////////////////////////////////////////////////////////
void TCATSPhysics::AddCATS(TVector3 C_X1_Y1, TVector3 C_X28_Y1, TVector3 C_X1_Y28, TVector3 C_X28_Y28){
  m_NumberOfCATS++			;

  // remove warning
  C_X28_Y28 *= 1;

  //	Vector U on Telescope Face (paralelle to Y Strip) 
  TVector3 U = C_X28_Y1 - C_X1_Y1 				;	
  U = U.Unit()									;

  //	Vector V on Telescope Face (parallele to X Strip)
  TVector3 V = C_X1_Y28 - C_X1_Y1 				;
  V = V.Unit()									;

  //	Position Vector of Strip Center
  TVector3 StripCenter 					;
  //	Position Vector of X=1 Y=1 Strip 
  TVector3 Strip_1_1 					;		

  //	Geometry Parameters  
  double Face = 71.12 				; //mm
  double NumberOfStrip = 28 			;
  double StripPitch = Face / NumberOfStrip	; //mm

  //	Buffer object to fill Position Array
  vector<double> lineX ; 
  vector<double> lineY ; 

  vector< vector< double > >	OneDetectorStripPositionX	;
  vector< vector< double > >	OneDetectorStripPositionY	;
  double                 	OneDetectorStripPositionZ	;

  //	Moving StripCenter to 1.1 corner (strip center!) :
  Strip_1_1 = C_X1_Y1 + (U+V) * (StripPitch/2) 	;

  for( int i = 0 ; i < 28 ; i++ ){
    lineX.clear()	;
    lineY.clear()	;

    for( int j = 0 ; j < 28 ; j++ ){
      StripCenter  = Strip_1_1 + StripPitch*( i*U + j*V  )	;
      lineX.push_back( StripCenter.x() )	;
      lineY.push_back( StripCenter.y() )	;	
    }

    OneDetectorStripPositionX.push_back(lineX);
    OneDetectorStripPositionY.push_back(lineY);
  }

  OneDetectorStripPositionZ = C_X1_Y1.Z();

  StripPositionX.push_back(OneDetectorStripPositionX)	;
  StripPositionY.push_back(OneDetectorStripPositionY)	;
  StripPositionZ.push_back(OneDetectorStripPositionZ)	;

}

///////////////////////////////////////////////////////////////
void TCATSPhysics::Clear(){  
  DetNumberX.clear(); 
  StripX.clear();
  ChargeX.clear();  
  StripMaxX.clear();
  ChargeMaxX.clear();
  DetMaxX.clear();
  DetNumberY.clear(); 
  StripY.clear();
  ChargeY.clear(); 
  StripMaxY.clear();
  ChargeMaxY.clear();
  DetMaxY.clear();
  DetNumber_PositionX.clear();
  DetNumber_PositionY.clear();
  DetNumber_PositionZ.clear();
  PositionX.clear();
  PositionY.clear();
  PositionZ.clear();
  QsumX.clear();
  QsumY.clear();

  Buffer_X_Q.clear();
  Buffer_Y_Q.clear();
}

////////////////////////////////////////////////////////////////////////////
bool TCATSPhysics :: IsValidChannel(const string DetectorType, const int Detector , const int channel) {
  if(DetectorType == "X")
    return *(m_XChannelStatus[Detector-1].begin()+channel-1);

  else if(DetectorType == "Y")
    return *(m_YChannelStatus[Detector-1].begin()+channel-1);

  else return false;
}


///////////////////////////////////////////////////////////////////////////////////
void TCATSPhysics::InitializeStandardParameter(){
  //   Enable all channel and no inversion
  vector< bool > ChannelStatus;
  vector< int > InversionStatus;
  m_XChannelStatus.clear()    ;
  m_YChannelStatus.clear()    ;
  m_CATSXInversion.clear()   ;
  m_CATSYInversion.clear()   ;

  ChannelStatus.resize(28,true);
  InversionStatus.resize(28);
  for(unsigned int j = 0 ; j < InversionStatus.size() ; j++){
    InversionStatus[j] = j+1;
  }

  for(int i = 0 ; i < m_NumberOfCATS ; ++i)      {
    m_XChannelStatus[i] = ChannelStatus;
    m_YChannelStatus[i] = ChannelStatus;
    m_CATSXInversion[i] = InversionStatus;
    m_CATSYInversion[i] = InversionStatus;
  }

  return;
}   

///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::ReadAnalysisConfig(){
  bool ReadingStatus = false;

  // path to file
  string FileName = "./configs/ConfigCATS.dat";

  // open analysis config file
  ifstream AnalysisConfigFile;
  AnalysisConfigFile.open(FileName.c_str());

  if (!AnalysisConfigFile.is_open()) {
    cout << " No ConfigCATS.dat found: Default parameter loaded for Analayis " << FileName << endl;
    return;
  }
  cout << " Loading user parameter for Analysis from ConfigCATS.dat " << endl;

  // Save it in a TAsciiFile
  TAsciiFile* asciiConfig = RootOutput::getInstance()->GetAsciiFileAnalysisConfig();
  asciiConfig->AppendLine("%%% ConfigCATS.dat %%%");
  asciiConfig->Append(FileName.c_str());
  asciiConfig->AppendLine("");
  // read analysis config file
  string LineBuffer,DataBuffer,whatToDo;
  while (!AnalysisConfigFile.eof()) {
    // Pick-up next line
    getline(AnalysisConfigFile, LineBuffer);

    // search for "header"
    if (LineBuffer.compare(0, 10, "ConfigCATS") == 0) ReadingStatus = true;

    // loop on tokens and data
    while (ReadingStatus ) {

      whatToDo="";
      AnalysisConfigFile >> whatToDo;

      // Search for comment symbol (%)
      if (whatToDo.compare(0, 1, "%") == 0) {
        AnalysisConfigFile.ignore(numeric_limits<streamsize>::max(), '\n' );
      }

      else if (whatToDo == "DISABLE_CHANNEL") {
        AnalysisConfigFile >> DataBuffer;
        cout << whatToDo << "  " << DataBuffer << endl;
        int Detector = atoi(DataBuffer.substr(4,1).c_str());
        int channel = -1;
        if (DataBuffer.compare(5,4,"STRX") == 0) {
          channel = atoi(DataBuffer.substr(9).c_str());
          *(m_XChannelStatus[Detector-1].begin()+channel-1) = false;
        }

        else if (DataBuffer.compare(5,4,"STRY") == 0) {
          channel = atoi(DataBuffer.substr(9).c_str());
          *(m_YChannelStatus[Detector-1].begin()+channel-1) = false;
        }

        else cout << "Warning: detector type for CATS unknown!" << endl;

      }

      else if (whatToDo == "INVERSION") {
        AnalysisConfigFile >> DataBuffer;
        cout << whatToDo << "  " << DataBuffer;
        int Detector = atoi(DataBuffer.substr(4,1).c_str());
        int channel1 = -1;
        int channel2 = -1;
        AnalysisConfigFile >> DataBuffer;
        cout << " " << DataBuffer;
        if (DataBuffer.compare(0,4,"STRX") == 0) {
          channel1 = atoi(DataBuffer.substr(4).c_str());
          AnalysisConfigFile >> DataBuffer;
          cout << " " << DataBuffer << endl;
          channel2 = atoi(DataBuffer.substr(4).c_str());
          *(m_CATSXInversion[Detector-1].begin()+channel1-1) = channel2;
          *(m_CATSXInversion[Detector-1].begin()+channel2-1) = channel1;
        }

        else if (DataBuffer.compare(0,4,"STRY") == 0) {
          channel1 = atoi(DataBuffer.substr(4).c_str());
          AnalysisConfigFile >> DataBuffer;
          cout << " " << DataBuffer << endl;
          channel2 = atoi(DataBuffer.substr(4).c_str());
          *(m_CATSYInversion[Detector-1].begin()+channel1-1) = channel2;
          *(m_CATSYInversion[Detector-1].begin()+channel2-1) = channel1;
        }
      }

      else if (whatToDo == "RECONSTRUCTION_METHOD") {
        AnalysisConfigFile >> DataBuffer;
        cout << whatToDo << "  " << DataBuffer;
        // DataBuffer is of form CATSNX 
        // Look for the CATS Number removing the first 4 letters and the trailling letter
        string Duplicate = DataBuffer.substr(4); // Duplicate is of form NX
        Duplicate.resize(Duplicate.size()-1); // Duplicate is of form
        unsigned int CATSNumber =  atoi(Duplicate.c_str());

        // Look for the X or Y part of the Detector, Basically the last character
        string XorY(string(1,DataBuffer[DataBuffer.size()-1])) ; 

        // Get the Reconstruction Methods Name
        AnalysisConfigFile >> DataBuffer;       

        // Set the Reconstruction Methods using above information 
        SetReconstructionMethod(CATSNumber,XorY,DataBuffer);
      }

      else if (whatToDo == "CORRECTION_METHOD") {
        AnalysisConfigFile >> DataBuffer;
        cout << whatToDo << "  " << DataBuffer;
        // DataBuffer is of form CATSNX 
        // Look for the CATS Number removing the first 4 letters and the trailling letter
        string Duplicate = DataBuffer.substr(4); // Duplicate is of form NX
        Duplicate.resize(Duplicate.size()-1); // Duplicate is of form
        unsigned int CATSNumber =  atoi(Duplicate.c_str());

        // Look for the X or Y part of the Detector, Basically the last character
        string XorY(string(1,DataBuffer[DataBuffer.size()-1])) ; 

        // Get the Reconstruction Methods Name
        AnalysisConfigFile >> DataBuffer;       

        // Set the Reconstruction Methods using above information 
        SetCorrectionMethod(CATSNumber,XorY,DataBuffer);
      }

      else if (whatToDo == "CORRECTION_COEF") {
        AnalysisConfigFile >> DataBuffer;
        cout << whatToDo << "  " << DataBuffer;
        // DataBuffer is of form CATSNX 
        // Look for the CATS Number removing the first 4 letters and the trailling letter
        string Duplicate = DataBuffer.substr(4); // Duplicate is of form NX
        Duplicate.resize(Duplicate.size()-1); // Duplicate is of form
        unsigned int CATSNumber =  atoi(Duplicate.c_str());

        // Look for the X or Y part of the Detector, Basically the last character
        string XorY(string(1,DataBuffer[DataBuffer.size()-1])) ; 

        // Get the Reconstruction Methods Name
        AnalysisConfigFile >> DataBuffer;       

        // Set the Reconstruction Methods using above information 
        SetCorrectionCoef(CATSNumber,XorY,atof(DataBuffer.c_str()));
      }

      else {ReadingStatus = false;}

    }
  }
} 
///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::InitSpectra(){
  m_Spectra = new TCATSSpectra(m_NumberOfCATS);
}

///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::FillSpectra(){
  m_Spectra -> FillRawSpectra(m_EventData);
  m_Spectra -> FillPreTreatedSpectra(m_PreTreatedData);
  m_Spectra -> FillPhysicsSpectra(m_EventPhysics);
}
///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::CheckSpectra(){
  // To be done
}
///////////////////////////////////////////////////////////////////////////
void TCATSPhysics::ClearSpectra(){
  // To be done
}
///////////////////////////////////////////////////////////////////////////
map< vector<TString> , TH1*> TCATSPhysics::GetSpectra() {
  return m_Spectra->GetMapHisto();
}

/////////////////////////////////////////////////////////////////////
//	Add Parameter to the CalibrationManger
void TCATSPhysics::AddParameterToCalibrationManager()	{
  CalibrationManager* Cal = CalibrationManager::getInstance();
  for(int i = 0 ; i < m_NumberOfCATS ; i++){
    for( int j = 0 ; j < 28 ; j++){
      Cal->AddParameter("CATS", "D"+itoa(i+1)+"_X"+itoa(j+1)+"_Q","CATS_D"+itoa(i+1)+"_X"+itoa(j+1)+"_Q")	;
      Cal->AddParameter("CATS", "D"+itoa(i+1)+"_Y"+itoa(j+1)+"_Q","CATS_D"+itoa(i+1)+"_Y"+itoa(j+1)+"_Q")	;
      Cal->AddParameter("CATS", "D"+itoa(i+1)+"_X"+itoa(j+1),"CATS_D"+itoa(i+1)+"_X"+itoa(j+1))	;
      Cal->AddParameter("CATS", "D"+itoa(i+1)+"_Y"+itoa(j+1),"CATS_D"+itoa(i+1)+"_Y"+itoa(j+1))	;
    } 
  }

  return;
}	

////////////////////////////////////////////////////////////////
void TCATSPhysics::SetReconstructionMethod(unsigned int CATSNumber, string XorY, string MethodName){
  if(XorY=="X"){
    if(ReconstructionFunctionX.size() < CATSNumber)
      ReconstructionFunctionX.resize(CATSNumber);

    if(MethodName=="ASECH") ReconstructionFunctionX[CATSNumber-1] = &(AnalyticHyperbolicSecant);
    else if(MethodName=="AGAUSS") ReconstructionFunctionX[CATSNumber-1] = &(AnalyticGaussian);
    else if(MethodName=="CENTROIDE")  ReconstructionFunctionX[CATSNumber-1] = &(Centroide); 
  }

  if(XorY=="Y"){
    if(ReconstructionFunctionY.size() < CATSNumber)
      ReconstructionFunctionY.resize(CATSNumber);

    if(MethodName=="ASECH") ReconstructionFunctionY[CATSNumber-1] = &(AnalyticHyperbolicSecant);
    else if(MethodName=="AGAUSS") ReconstructionFunctionY[CATSNumber-1] = &(AnalyticGaussian);
    else if(MethodName=="CENTROIDE")  ReconstructionFunctionY[CATSNumber-1] = &(Centroide); 
  }

}


/*
/////////////////////////////////////////////////////////////////////////
double TCATSPhysics::Corrected3Points(double Position, double c) {
double Corrected_Position = 0;
int StripMax_ = StripMaxX[ff] -1;
double xmax = StripPositionX[ff][StripMax_][0] ;

Corrected_Position = (Position - xmax) / c + xmax;

return Corrected_Position;
}

/////////////////////////////////////////////////////////////////////////
double TCATSPhysics::Corrected4Points(double Position, double d) {
double Corrected_Position = 0;
double xmax = 0;
int StripMax_ = StripMaxX[ff] -1;

if(Buffer_X_Q[StripMax_+1][ff] > Buffer_X_Q[StripMax_-1][ff]) {
if(ff==0)     xmax = StripPositionX[ff][StripMax_][0] - 1.27;
else  xmax = StripPositionX[ff][StripMax_][0] + 1.27;
}

else{
if(ff==0)     xmax = StripPositionX[ff][StripMax_-1][0] - 1.27;
else  xmax = StripPositionX[ff][StripMax_-1][0]  + 1.27;
}

Corrected_Position = (Position - xmax) / d + xmax;

return Corrected_Position;
}

///////////////////////////////////////////////////////////////
double TCATSPhysics::CorrectedPositionX(double Position, double a) {
double Corrected_Position = 0;
int StripMax_ = StripMaxX[ff] -1;
double xmax = StripPositionX[ff][StripMax_][0] ;

Corrected_Position = (Position - xmax) / a + xmax;

return Corrected_Position;
}

///////////////////////////////////////////////////////////////
double TCATSPhysics::CorrectedPosition4(double Position, double b) {
double Corrected_Position = 0;
double xmax = 0;
int StripMax_ = StripMaxX[ff] -1;

if(Buffer_X_Q[StripMax_+1][ff] > Buffer_X_Q[StripMax_-1][ff]) {
if(ff ==0)     xmax = StripPositionX[ff][StripMax_][0] - 1.27;
else  xmax = StripPositionX[ff][StripMax_][0] + 1.27;
}

else{
if(ff ==0)     xmax = StripPositionX[ff][StripMax_-1][0] - 1.27;
else  xmax = StripPositionX[ff][StripMax_-1][0]  + 1.27;
}

Corrected_Position = (Position - xmax) / b + xmax;

return Corrected_Position;
}
*/
///////////////////////////////////////////////////////////////
TVector3 TCATSPhysics::GetBeamDirection(){
  TVector3 Position = TVector3 (PositionX[1]-PositionX[0] ,
      PositionY[1]-PositionY[0] ,
      PositionZ[1]-PositionZ[0] );
  Position.Unit();
  return(Position) ;	
}

///////////////////////////////////////////////////////////////
TVector3 TCATSPhysics::GetPositionOnTarget(){
  double Pi = 3.14159265;
  TVector3 Position = TVector3 (GetPositionOnTargetX() 	,
      GetPositionOnTargetY() 	,
      GetPositionOnTargetX()*tan(m_TargetAngle*Pi/180)); 
  Position.Unit();
  return(Position) ;	
}

////////////////////////////////////////////////////////////////////////
namespace LOCAL_CATS{
  ////////////////////////////////////////////////////////////////////
  double AnalyticGaussian(vector<double>& Buffer_Q,int& StripMax){
    double gauss = -1000;
    double Q[3];
    double StripPos[3];
    for(int j = 0; j<3 ; j++){
      Q[j] = 0;
      StripPos[j] = 0;
    }

    if(StripMax> 3 && StripMax< 26){
      // central value taken using the Strip with Max charge
      Q[0] = Buffer_Q[StripMax-1] ;

      // Look at the next strip on the left
      if(Buffer_Q[StripMax-2]!=-1){
        Q[1] = Buffer_Q[StripMax-2];
        StripPos[1] = StripMax-2;
      }

      // Look at the next next strip on the left
      else if(Buffer_Q[StripMax-3]!=-1){
        Q[1] = Buffer_Q[StripMax-3];
        StripPos[1] = StripMax-3;
      }

      // Look at the next next next strip on the left
      else if(Buffer_Q[StripMax-4]!=-1){
        Q[1] = Buffer_Q[StripMax-4];
        StripPos[1] = StripMax-4;
      }

      // Look at the next strip on the right
      if(Buffer_Q[StripMax]!=-1){
        Q[2] = Buffer_Q[StripMax];
        StripPos[2] = StripMax;
      }

      // Look at the next next strip on the right
      else if(Buffer_Q[StripMax+1]!=-1){
        Q[2] = Buffer_Q[StripMax+1];
        StripPos[2] = StripMax+1;
      }

      // Look at the next next next strip on the right 
      else if(Buffer_Q[StripMax+2]!=-1){
        Q[2] = Buffer_Q[StripMax+2];
        StripPos[2] = StripMax+2;
      }

    }

    double Q0_Q1 = log(Q[0]/Q[1]);
    double Q0_Q2 = log(Q[0]/Q[2]);

    double num   = Q0_Q1 * (StripPos[2]*StripPos[2] - StripPos[0]*StripPos[0]) - Q0_Q2 * (StripPos[1]*StripPos[1] - StripPos[0]*StripPos[0])  ;
    double denom = Q0_Q1 * (StripPos[2] - StripPos[0]) - Q0_Q2 * (StripPos[1] - StripPos[0]) ;

    if(denom != 0){
      gauss = 0.5*num / denom;
    }

    else{
      gauss = -1000;
    }

    return gauss;

  }

  ///////////////////////////////////////////////////////////////
  double Centroide(vector<double>& Buffer_Q, int& StripMax){
    double Centroide = 0 ;

    if(StripMax > 2 && StripMax < 27){
      int StripMax = StripMax -1 ; 
      double NumberOfPoint = 0 ;
      double ChargeTotal =0;

      for(int i = -2 ; i < 3 ; i++){
        if(Buffer_Q[StripMax+i]!=-1){ 
          Centroide += (StripMax+i)*Buffer_Q[StripMax+i] ;
          NumberOfPoint++;
          ChargeTotal+=Buffer_Q[StripMax+i];
        }
      }

      if(ChargeTotal>0) Centroide = Centroide / ChargeTotal ;

      else {
        Centroide = -1000 ; 
      } 

    }

    else{
      Centroide = -1000 ;
    }

    return Centroide ;
  }

  /////////////////////////////////////////////////////////////////////
  double AnalyticHyperbolicSecant(vector<double>& Buffer_Q, int& StripMax){
    double sech = -1000 ;

    if(StripMax > 2 && StripMax<27){		
      double vs1 = sqrt( Buffer_Q[StripMax-1]/Buffer_Q[StripMax-1+1] );
      double vs2 = sqrt( Buffer_Q[StripMax-1]/Buffer_Q[StripMax-1-1] );
      double vs3 = 0.5*( vs1 + vs2 );
      double vs4 = log( vs3 + sqrt(vs3*vs3-1.0) );
      double vs5 = (vs1 - vs2)/(2.0*sinh(vs4));	

      if(vs5<0) vs5=-vs5 ;

      double vs6 = 0.5*log( (1.0+vs5)/(1.0-vs5) ) ;

      if ( Buffer_Q[StripMax-1+1]>Buffer_Q[StripMax-1-1] ){ 
        sech = StripMax + vs6/vs4 ;
      }

      else{ 
        sech = StripMax - vs6/vs4 ;
      }	

    }

    else { 
      sech = -1000; 
    }

    return sech ;
  }












  //	transform an integer to a string
  string itoa(int value){
    std::ostringstream o;

    if (!(o << value))
      return ""	;

    return o.str();
  }

  double fCATS_X_Q(const TCATSData* m_EventData , const int i){
    return CalibrationManager::getInstance()->ApplyCalibration( "CATS/D" + itoa( m_EventData->GetCATSDetX(i) ) + "_X" + itoa( m_EventData->GetCATSStripX(i) ) + "_Q",   
        m_EventData->GetCATSChargeX(i) + gRandom->Rndm() - fCATS_Ped_X(m_EventData, i) );
  }

  double fCATS_Y_Q(const TCATSData* m_EventData , const int i){
    return CalibrationManager::getInstance()->ApplyCalibration( "CATS/D" + itoa( m_EventData->GetCATSDetY(i) ) + "_Y" + itoa( m_EventData->GetCATSStripY(i) ) + "_Q",   
        m_EventData->GetCATSChargeY(i) + gRandom->Rndm() - fCATS_Ped_Y(m_EventData, i) );
  }

  bool fCATS_Threshold_X(const TCATSData* m_EventData , const int i){
    return CalibrationManager::getInstance()->ApplyThreshold( "CATS/D" + itoa( m_EventData->GetCATSDetX(i) ) + "_X" + itoa( m_EventData->GetCATSStripX(i) ),
        m_EventData->GetCATSChargeX(i));
  }

  bool fCATS_Threshold_Y(const TCATSData* m_EventData , const int i){
    return CalibrationManager::getInstance()->ApplyThreshold( "CATS/D" + itoa( m_EventData->GetCATSDetY(i) ) + "_Y" + itoa( m_EventData->GetCATSStripY(i) ),
        m_EventData->GetCATSChargeY(i));
  }

  double fCATS_Ped_X(const TCATSData* m_EventData, const int i){
    return CalibrationManager::getInstance()->GetPedestal( "CATS/D" + itoa( m_EventData->GetCATSDetX(i) ) + "_X" + itoa( m_EventData->GetCATSStripX(i) ) );
  }

  double fCATS_Ped_Y(const TCATSData* m_EventData, const int i){
    return CalibrationManager::getInstance()->GetPedestal( "CATS/D" + itoa( m_EventData->GetCATSDetY(i) ) + "_Y" + itoa( m_EventData->GetCATSStripY(i) ) );
  }
}

