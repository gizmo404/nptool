/*****************************************************************************
 * Copyright (C) 2009-2016   this file is part of the NPTool Project         *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien Matta  contact address: a.matta@surrey.ac.uk      *
 *                                                                           *
 * Creation Date  : January 2016                                             *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class describe  Miniball simulation                                 *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *****************************************************************************/

// C++ headers
#include <sstream>
#include <cmath>
#include <limits>
//G4 Geometry object
#include "G4Tubs.hh"
#include "G4Box.hh"

//G4 sensitive
#include "G4SDManager.hh"
#include "G4MultiFunctionalDetector.hh"

//G4 various object
#include "G4Material.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"

// NPTool header
#include "Miniball.hh"
#include "CalorimeterScorers.hh"
#include "RootOutput.h"
#include "MaterialManager.hh"
#include "NPSDetectorFactory.hh"
#include "NPOptionManager.h"
// CLHEP header
#include "CLHEP/Random/RandGauss.h"

using namespace std;
using namespace CLHEP;


//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
namespace Miniball_NS{
	// Energy and time Resolution
	const double EnergyThreshold = 0.01*MeV;
	const double ResoTime = 4.5*ns ;
	const double ResoEnergy =  0.003*MeV ;
	const double ResoAngle = 5*deg;
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Miniball Specific Method
Miniball::Miniball(){
	m_Event = new TMiniballData() ;
	m_MiniballScorer = 0;
	m_ClusterDetector = 0;
	m_NumberOfPlacedVolume = 0;
	constructChamber = false;
	ClusterDetectorCounter = 0;
}

Miniball::~Miniball(){
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void Miniball::AddMiniball(double  R, double  Theta, double  Phi, double  Alpha){
	m_R.push_back(R);
	m_Theta.push_back(Theta);
	m_Phi.push_back(Phi);
	m_Alpha.push_back(Alpha);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void Miniball::BuildChamber(G4LogicalVolume* world)
{
	G4GDMLParser m_gdmlparser;
	m_gdmlparser.Read(m_GDMLPath+m_GDMLName);
	m_LogicalGDML= m_gdmlparser.GetVolume(m_GDMLWorld);

	string name;
	for (int i = 0; i < m_LogicalGDML->GetNoDaughters(); i++)
	{
		G4VPhysicalVolume* VPV = m_LogicalGDML->GetDaughter(i);
		G4LogicalVolume* LV = m_LogicalGDML->GetDaughter(i)->GetLogicalVolume();
		G4ThreeVector LTrans = VPV->GetTranslation();
		G4RotationMatrix* LRot = VPV->GetRotation();

		name = LV->GetSolid()->GetName();

		if (name != "Tessellated_Shape_6") PVPBuffer = new G4PVPlacement(LRot, LTrans, LV, name, world, false, 0 );
	}
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4AssemblyVolume* Miniball::BuildClusterDetector(double Alpha){
	//if(!m_ClusterDetector)
	{
		if(m_ClusterDetector) 
		{
			//delete m_ClusterDetector;
			m_ClusterDetector= new G4AssemblyVolume();
		}
		else 
		{
			m_ClusterDetector= new G4AssemblyVolume();
			cout << "Miniball geometry is based on Munich Group Simulation exported in GDML"<< endl;
			string basepath = getenv("NPTOOL");
			string path=basepath+"/NPSimulation/Detectors/Miniball/Miniball.gdml";
			m_gdmlparser.Read(path,false);
		}
		G4VisAttributes* Red = new G4VisAttributes(G4Color(1,0.5,0.5));
		G4VisAttributes* Green= new G4VisAttributes(G4Color(0.5,1,0.5));
		G4VisAttributes* Blue = new G4VisAttributes(G4Color(0.5,0.5,1));
		G4VisAttributes* Caps = new G4VisAttributes(G4Color(0.5,0.5,0.5,0.5));

		G4LogicalVolume* World = m_gdmlparser.GetVolume("MexpHall_log");  
		string name;
		{
			double alpha = Alpha;
			for(int i = 0 ; i < World->GetNoDaughters () ;i++){
				G4VPhysicalVolume* VPV = World->GetDaughter(i);
				name = VPV->GetLogicalVolume()->GetName();
				if(name == "cluster0_0_HPGe_A_0_det_env_log"){
					G4LogicalVolume* HPGE = VPV->GetLogicalVolume(); 
					HPGE->GetDaughter(0)->GetLogicalVolume()->SetSensitiveDetector(m_MiniballScorer);
					HPGE->SetVisAttributes(Red);
					G4RotationMatrix* Rot = VPV->GetObjectRotation(); 
					Rot->rotateZ(alpha);
					G4ThreeVector Pos = VPV->GetObjectTranslation(); 
					Pos.rotateZ(alpha);
					G4Transform3D Trans(*Rot,Pos);
					m_ClusterDetector->AddPlacedVolume(HPGE,Trans); 
					m_NumberOfPlacedVolume++;
				}
				else if(name == "cluster0_0_HPGe_B_1_det_env_log"){
					G4LogicalVolume* HPGE = VPV->GetLogicalVolume(); 
					HPGE->GetDaughter(0)->GetLogicalVolume()->SetSensitiveDetector(m_MiniballScorer);
					HPGE->SetVisAttributes(Green);
					G4RotationMatrix* Rot = VPV->GetObjectRotation(); 
					Rot->rotateZ(alpha);
					G4ThreeVector Pos = VPV->GetObjectTranslation(); 
					Pos.rotateZ(alpha);
					G4Transform3D Trans(*Rot,Pos);
					m_ClusterDetector->AddPlacedVolume(HPGE,Trans); 
					m_NumberOfPlacedVolume++;
				}
				else if(name == "cluster0_0_HPGe_C_2_det_env_log"){
					G4LogicalVolume* HPGE = VPV->GetLogicalVolume(); 
					HPGE->GetDaughter(0)->GetLogicalVolume()->SetSensitiveDetector(m_MiniballScorer);
					HPGE->SetVisAttributes(Blue);
					G4RotationMatrix* Rot = VPV->GetObjectRotation(); 
					Rot->rotateZ(alpha);
					G4ThreeVector Pos = VPV->GetObjectTranslation(); 
					Pos.rotateZ(alpha);
					G4Transform3D Trans(*Rot,Pos);
					m_ClusterDetector->AddPlacedVolume(HPGE,Trans); 
					m_NumberOfPlacedVolume++;
				}
				else if(name.compare(0,8,"cluster0")==0 || name == "nozzle_log"){
					G4LogicalVolume* Capsule= VPV->GetLogicalVolume(); 
					Capsule->SetVisAttributes(Caps);
					G4RotationMatrix* Rot = VPV->GetObjectRotation(); 
					Rot->rotateZ(alpha);
					G4ThreeVector Pos = VPV->GetObjectTranslation(); 
					Pos.rotateZ(alpha);
					G4Transform3D Trans(*Rot,Pos);
					m_ClusterDetector->AddPlacedVolume(Capsule,Trans); 
					m_NumberOfPlacedVolume++;
					Rot->rotateZ(-alpha);//TODO why the hell is this needed?!?! Don't take it out or one part of the casing doesn't rotate properly
				}
			}
		}
		//cout << m_NumberOfPlacedVolume << "\n";
		m_NumberOfPlacedVolume = 42;
	}
	ClusterDetectorHolder.push_back(m_ClusterDetector);	
	return m_ClusterDetector;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Virtual Method of NPS::VDetector class

// Read stream at Configfile to pick-up parameters of detector (Position,...)
// Called in DetecorConstruction::ReadDetextorConfiguration Method
void Miniball::ReadConfiguration(NPL::InputParser parser){
	vector<NPL::InputBlock*> blocks = parser.GetAllBlocksWithToken("Miniball");
	if(NPOptionManager::getInstance()->GetVerboseLevel())
		cout << "//// " << blocks.size() << " detectors found " << endl; 

	vector<string> token = {"R","Theta","Phi","Alpha"};
	vector<string> chamberToken = {"GDMLFilePath","GDMLFileName","GDMLWorldName"};

	for(unsigned int i = 0 ; i < blocks.size() ; i++){
		if(blocks[i]->HasTokenList(token)){
			if(NPOptionManager::getInstance()->GetVerboseLevel())
				cout << endl << "////  Miniball Cluster" << i+1 <<  endl;
			double R = blocks[i]->GetDouble("R","mm");
			double Theta = blocks[i]->GetDouble("Theta","deg");
			double Phi = blocks[i]->GetDouble("Phi","deg");
			double Alpha = blocks[i]->GetDouble("Alpha","deg");

			AddMiniball(R,Theta,Phi,Alpha);
		}
		else if (blocks[i]->HasTokenList(chamberToken)){
			string basepath = getenv("NPTOOL");

			m_GDMLPath = basepath+blocks[i]->GetString("GDMLFilePath");
			m_GDMLName = blocks[i]->GetString("GDMLFileName");
			m_GDMLWorld = blocks[i]->GetString("GDMLWorldName");

			constructChamber = true;
		}

		else{
			cout << "ERROR: check your input file formatting " << endl;
			exit(1);
		}
	}
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

// Construct detector and inialise sensitive part.
// Called After DetecorConstruction::AddDetector Method
void Miniball::ConstructDetector(G4LogicalVolume* world){
	for (unsigned short i = 0 ; i < m_R.size() ; i++) {
		//int counter = 0;
		BuildClusterDetector(m_Alpha[i]);
		//BuildClusterDetector(0);

		G4double wX = m_R[i] * sin(m_Theta[i] ) * cos(m_Phi[i] ) ;
		G4double wY = m_R[i] * sin(m_Theta[i] ) * sin(m_Phi[i] ) ;
		G4double wZ = m_R[i] * cos(m_Theta[i] ) ;
		G4ThreeVector Det_pos = G4ThreeVector(wX, wY, wZ) ;
		G4ThreeVector d = Det_pos.unit();
		Det_pos= Det_pos-d*100*mm;
		// Building Detector reference frame
		G4double ii = cos(m_Theta[i]) * cos(m_Phi[i]);
		G4double jj = cos(m_Theta[i]) * sin(m_Phi[i]);
		G4double kk = -sin(m_Theta[i]);
		G4ThreeVector Y(ii,jj,kk);
		G4ThreeVector w = Det_pos.unit();

		G4ThreeVector u = w.cross(Y);
		G4ThreeVector v = w.cross(u);
		v = v.unit();
		u = u.unit();

		G4RotationMatrix* Rot = new G4RotationMatrix(u,v,w);

		// original
		//m_ClusterDetector->MakeImprint(world,Det_pos, Rot,i+1);
		ClusterDetectorHolder[i]->MakeImprint(world,Det_pos, Rot,i+1);

		// set a nicer name ORIGINAL
		//std::vector< G4VPhysicalVolume * >::iterator it = m_ClusterDetector->GetVolumesIterator();
		std::vector< G4VPhysicalVolume * >::iterator it = ClusterDetectorHolder[i]->GetVolumesIterator();

		// original
		//it+=m_ClusterDetector->GetImprintsCount()*m_NumberOfPlacedVolume-1;
		it+= m_NumberOfPlacedVolume-1;
		// unchanged
		for(unsigned int l = 0 ; l < m_NumberOfPlacedVolume-3 ; l++){
			(*it)->SetName("Capsule");
			(*it)->SetCopyNo(i+1);
			it--;
		}
		(*it)->SetName("HPGe_A");
		(*it)->SetCopyNo(i+1);
		it--;
		(*it)->SetName("HPGe_B");
		(*it)->SetCopyNo(i+1);
		it--;
		(*it)->SetName("HPGe_C");
		(*it)->SetCopyNo(i+1);;
		
	}
	if (constructChamber) BuildChamber(world);
}
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Add Detector branch to the EventTree.
// Called After DetecorConstruction::AddDetector Method
void Miniball::InitializeRootOutput(){
	RootOutput *pAnalysis = RootOutput::getInstance();
	TTree *pTree = pAnalysis->GetTree();
	if(!pTree->FindBranch("Miniball")){
		pTree->Branch("Miniball", "TMiniballData", &m_Event) ;
	}
	pTree->SetBranchAddress("Miniball", &m_Event) ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
// Read sensitive part and fill the Root tree.
// Called at in the EventAction::EndOfEventAvtion
void Miniball::ReadSensitive(const G4Event* event){
	m_Event->Clear();

	///////////
	// Calorimeter scorer
	NPS::HitsMap<G4double*>* CaloHitMap;
	std::map<G4int, G4double**>::iterator Calo_itr;

	G4int CaloCollectionID = G4SDManager::GetSDMpointer()->GetCollectionID("MiniballScorer/Crystal");
	CaloHitMap = (NPS::HitsMap<G4double*>*)(event->GetHCofThisEvent()->GetHC(CaloCollectionID));

	// Loop on the Calo map
	for (Calo_itr = CaloHitMap->GetMap()->begin() ; Calo_itr != CaloHitMap->GetMap()->end() ; Calo_itr++){

		G4double* Info = *(Calo_itr->second);
		//(Info[0]/2.35)*((Info[0]*1.02)*pow((Info[0]*1.8),.5))
		// double Energy = RandGauss::shoot(Info[0],((Info[0]*1000*1.02/2.35)*pow((Info[0]*1000*1.8),.5)) );
		double Energy = RandGauss::shoot(Info[0],Miniball_NS::ResoEnergy);
		if(Energy>Miniball_NS::EnergyThreshold){
			double Time = RandGauss::shoot(Info[1],Miniball_NS::ResoTime);
			int DetectorNbr = (int) Info[7];
			TVector3 Angle;  
			//Angle.SetX(RandGauss::shoot(Info[5]/deg,Miniball_NS::ResoAngle));
			//Angle.SetY(RandGauss::shoot(Info[6]/deg,Miniball_NS::ResoAngle));
			//Angle.SetZ(0.);
			Angle.SetX(m_Theta[DetectorNbr-1]);
			Angle.SetY(m_Phi[DetectorNbr-1]);
			Angle.SetZ(m_Alpha[DetectorNbr-1]);

			m_Event->SetEnergy(DetectorNbr,Energy);
			m_Event->SetAngle(DetectorNbr,Angle);
			m_Event->SetTime(DetectorNbr,Time); 

			// Interraction Coordinates
			ms_InterCoord->SetDetectedPositionX(Info[2]) ;
			ms_InterCoord->SetDetectedPositionY(Info[3]) ;
			ms_InterCoord->SetDetectedPositionZ(Info[4]) ;
			ms_InterCoord->SetDetectedAngleTheta(Info[5]/deg) ;
			ms_InterCoord->SetDetectedAnglePhi(Info[6]/deg) ;
		}
	}
	// clear map for next event
	CaloHitMap->clear();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
////////////////////////////////////////////////////////////////   
void Miniball::InitializeScorers() { 
	// This check is necessary in case the geometry is reloaded
	bool already_exist = false; 
	m_MiniballScorer = CheckScorer("MiniballScorer",already_exist) ;

	if(already_exist) 
		return ;

	// Otherwise the scorer is initialised
	vector<int> level; level.push_back(1);
	G4VPrimitiveScorer* Calorimeter= new CALORIMETERSCORERS::PS_CalorimeterWithInteraction("Crystal",level, 0) ;
	//and register it to the multifunctionnal detector
	m_MiniballScorer->RegisterPrimitive(Calorimeter);
	G4SDManager::GetSDMpointer()->AddNewDetector(m_MiniballScorer) ;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
////////////////////////////////////////////////////////////////////////////////
//            Construct Method to be pass to the DetectorFactory              //
////////////////////////////////////////////////////////////////////////////////
NPS::VDetector* Miniball::Construct(){
	return  (NPS::VDetector*) new Miniball();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
////////////////////////////////////////////////////////////////////////////////
//            Registering the construct method to the factory                 //
////////////////////////////////////////////////////////////////////////////////
extern"C" {
	class proxy_nps_plastic{
		public:
			proxy_nps_plastic(){
				NPS::DetectorFactory::getInstance()->AddToken("Miniball","Miniball");
				NPS::DetectorFactory::getInstance()->AddDetector("Miniball",Miniball::Construct);
			}
	};

	proxy_nps_plastic p_nps_plastic;
}
