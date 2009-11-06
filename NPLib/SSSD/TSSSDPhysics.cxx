/*****************************************************************************
 * Copyright (C) 2009   this file is part of the NPTool Project              *
 *                                                                           *
 * For the licensing terms see $NPTOOL/Licence/NPTool_Licence                *
 * For the list of contributors see $NPTOOL/Licence/Contributors             *
 *****************************************************************************/

/*****************************************************************************
 * Original Author: Adrien MATTA  contact address: matta@ipno.in2p3.fr       *
 *                                                                           *
 * Creation Date  : november 2009                                            *
 * Last update    :                                                          *
 *---------------------------------------------------------------------------*
 * Decription:                                                               *
 *  This class hold SSSD  Physics                                            *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * Comment:                                                                  *
 *                                                                           *
 *                                                                           *
 *****************************************************************************/
//	NPL
#include "TSSSDPhysics.h"
#include "RootOutput.h"
#include "RootInput.h"

//	STL
#include <iostream>
#include <sstream>
#include <fstream>
#include <limits>
#include <stdlib.h>
using namespace std;

//	ROOT
#include "TChain.h"

//	tranform an integer to a string
		string itoa(int value)
			{
			  std::ostringstream o;
			
			  if (!(o << value))
			    return ""	;
			    
			  return o.str();
			}

ClassImp(TSSSDPhysics)
///////////////////////////////////////////////////////////////////////////
TSSSDPhysics::TSSSDPhysics()
	{		
		NumberOfDetector = 0 				;
		EventData = new TSSSDData		;
		EventPhysics = this					;
	}
///////////////////////////////////////////////////////////////////////////
TSSSDPhysics::~TSSSDPhysics()
	{}
///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::Clear()
	{
		DetectorNumber	.clear()	;
		StripNumber			.clear()	;
		Energy					.clear()	;
		Time						.clear()	;
	}
///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::ReadConfiguration(string Path) 
	{
   ifstream ConfigFile           ;
   ConfigFile.open(Path.c_str()) ;
   string LineBuffer          ;
   string DataBuffer          ;

   double TLX , BLX , BRX , TRX , TLY , BLY , BRY , TRY , TLZ , BLZ , BRZ , TRZ   ;
   double Theta = 0 , Phi = 0 , R = 0 , beta_u = 0 , beta_v = 0 , beta_w = 0      ;
   bool check_A = false   ;
   bool check_B = false ;
   bool check_C = false   ;
   bool check_D = false ;

   bool check_Theta = false   ;
   bool check_Phi  = false  ;
   bool check_R     = false   ;
   bool check_beta = false  ;
   bool ReadingStatus = false ;

 while (!ConfigFile.eof()) 
 	{
      
		getline(ConfigFile, LineBuffer);

		//	If line is a Start Up ThinSi bloc, Reading toggle to true      
		  	if (LineBuffer.compare(0, 6, "ThinSi") == 0) 
		    	{
		      	cout << "Detector found: " << endl   ;        
		      	ReadingStatus = true ;
		    	}

		//	Else don't toggle to Reading Block Status
		else ReadingStatus = false ;

		//	Reading Block
		while(ReadingStatus)
			{
					// Pickup Next Word 
				ConfigFile >> DataBuffer ;

				//	Comment Line 
				if (DataBuffer.compare(0, 1, "%") == 0) {	ConfigFile.ignore ( std::numeric_limits<std::streamsize>::max(), '\n' );}

					//	Finding another telescope (safety), toggle out
				else if (DataBuffer.compare(0, 6, "ThinSi") == 0) {
					cout << "WARNING: Another Telescope is find before standard sequence of Token, Error may occured in Telecope definition" << endl ;
					ReadingStatus = false ;
				}

					 //Position method
		         else if (DataBuffer.compare(0, 3, "A=") == 0) {
		            check_A = true;
		            ConfigFile >> DataBuffer ;
		            TLX = atof(DataBuffer.c_str()) ;
		            ConfigFile >> DataBuffer ;
		            TLY = atof(DataBuffer.c_str()) ;
		            ConfigFile >> DataBuffer ;
		            TLZ = atof(DataBuffer.c_str()) ;

		         }
				
				else if (DataBuffer.compare(0, 3, "B=") == 0) {
					check_B = true;
					ConfigFile >> DataBuffer ;
					BLX = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					BLY = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					BLZ = atof(DataBuffer.c_str()) ;

				}

				else if (DataBuffer.compare(0, 3, "C=") == 0) {
					check_C = true;
					ConfigFile >> DataBuffer ;
					BRX = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					BRY = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					BRZ = atof(DataBuffer.c_str()) ;

				}

				else if (DataBuffer.compare(0, 3, "D=") == 0) {
					check_D = true;
					ConfigFile >> DataBuffer ;
					TRX = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					TRY = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					TRZ = atof(DataBuffer.c_str()) ;

				}

									
				//Angle method
				else if (DataBuffer.compare(0, 6, "THETA=") == 0) {
					check_Theta = true;
					ConfigFile >> DataBuffer ;
					Theta = atof(DataBuffer.c_str()) ;
				}

				else if (DataBuffer.compare(0, 4, "PHI=") == 0) {
					check_Phi = true;
					ConfigFile >> DataBuffer ;
					Phi = atof(DataBuffer.c_str()) ;
				}

				else if (DataBuffer.compare(0, 2, "R=") == 0) {
					check_R = true;
					ConfigFile >> DataBuffer ;
					R = atof(DataBuffer.c_str()) ;
				}


				else if (DataBuffer.compare(0, 5, "BETA=") == 0) {
					check_beta = true;
					ConfigFile >> DataBuffer ;
					beta_u = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					beta_v = atof(DataBuffer.c_str()) ;
					ConfigFile >> DataBuffer ;
					beta_w = atof(DataBuffer.c_str()) ;
				}
		      
		         	///////////////////////////////////////////////////
					//	If no Detector Token and no comment, toggle out
		         else 
		         	{ReadingStatus = false; cout << "Wrong Token Sequence: Getting out " << DataBuffer << endl ;}
		         
		         	/////////////////////////////////////////////////
		         	//	If All necessary information there, toggle out
		         
		         if ( (check_A && check_B && check_C && check_D) || (check_Theta && check_Phi && check_R && check_beta) ) 
		         	{ 
				         	ReadingStatus = false; 
				         	
				         	///Add The previously define telescope
		         			//With position method
				         	 if ((check_A && check_B && check_C && check_D) || !(check_Theta && check_Phi && check_R)) {
						            	 NumberOfDetector++;
						         }

					         //with angle method
					         else if ((check_Theta && check_Phi && check_R) || !(check_A && check_B && check_C && check_D)) {
						         		  NumberOfDetector++;
						         }
						         
						        //	Reinitialisation of Check Boolean 
						        
								check_A = false   ;
								check_B = false ;
								check_C = false   ;
								check_D = false ;

								check_Theta = false   ;
								check_Phi  = false  ;
								check_R     = false   ;
								check_beta = false  ;
								ReadingStatus = false ;
							         
		         	}
		         	
						}
					}

}


///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::AddParameterToCalibrationManager()
	{
		CalibrationManager* Cal = CalibrationManager::getInstance();
		
		for(int i = 0 ; i < NumberOfDetector ; i++)
			{
			
				for( int j = 0 ; j < 16 ; j++)
					{
						Cal->AddParameter("SSSD", "Detector"+itoa(i+1)+"_Strip"+itoa(j+1)+"_E","SSSD_Detector"+itoa(i+1)+"_Strip"+itoa(j+1)+"_E")	;
						Cal->AddParameter("SSSD", "Detector"+itoa(i+1)+"_Strip"+itoa(j+1)+"_T","SSSD_Detector"+itoa(i+1)+"_Strip"+itoa(j+1)+"_T")	;	
					}
		
			}
	}
	
///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::InitializeRootInput()
	{
		TChain* inputChain = RootInput::getInstance()->GetChain()	;
		inputChain->SetBranchStatus ( "ThinSi" 		, true )					;
		inputChain->SetBranchStatus ( "fSSSD_*" 	, true )					;
		inputChain->SetBranchAddress( "ThinSi" 		, &EventData )		;
	}	

///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::InitializeRootOutput()
	{
		TTree* outputTree = RootOutput::getInstance()->GetTree()			;
		outputTree->Branch( "SSSD" , "TSSSDPhysics" , &EventPhysics )	;
	}

///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::BuildPhysicalEvent()
	{
		BuildSimplePhysicalEvent()	;
	}

///////////////////////////////////////////////////////////////////////////
void TSSSDPhysics::BuildSimplePhysicalEvent()
	{
		if( EventData->GetEnergyMult() == EventData->GetTimeMult() )
			{
				for(unsigned int i = 0 ; i < EventData->GetEnergyMult() ; i++)
					{
					
						DetectorNumber	.push_back( EventData->GetEnergyDetectorNbr(i) )	;
						StripNumber			.push_back( EventData->GetEnergyStripNbr(i)    )	;
						
						Energy					.push_back(
							CalibrationManager::getInstance()->ApplyCalibration(	"SSSD/Detector" + itoa( EventData->GetEnergyDetectorNbr(i) ) + "_Strip" + itoa( EventData->GetEnergyStripNbr(i) ) +"_E",	
																																		EventData->GetEnergy(i) )
																			)	;
																			
						Time						.push_back(
							CalibrationManager::getInstance()->ApplyCalibration(	"SSSD/Detector" + itoa( EventData->GetEnergyDetectorNbr(i) ) + "_Strip" + itoa( EventData->GetEnergyStripNbr(i) ) +"_T",	
																																		EventData->GetTime(i) )
																			)	;
					}
			}
		
		else return ;
		
	}
