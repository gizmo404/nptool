TGraph* thdae;
TGraphErrors* gdd;
double minX = 1e30;
double minC = 1e30;
void NumericalMinimization(const char * minName ,const char *algoName);
double Chi2(const double* parameter);
////////////////////////////////////////////////////////////////////////////////
void CS(){
  TFile* file = new TFile("macro/Kine.root","READ");
  TH2F* h2 = (TH2F*)file->FindObjectAny("hEXT2");

  file = new TFile("SharcEfficiency_28Mgdd.root","READ");
  TH1D* eff = (TH1D*) file->FindObjectAny("SolidAngleCM");

  file = new TFile("Mg28dd_dae.root","READ");
  thdae = (TGraph*) file->FindObjectAny("Graph");

  file = new TFile("Mg28dd_pap.root","READ");
  TGraph* thpap = (TGraph*) file->FindObjectAny("Graph");
  thpap->SetLineColor(kGreen-3);

  file = new TFile("Mg28dd_loh.root","READ");
  TGraph* thloh = (TGraph*) file->FindObjectAny("Graph");
  thloh->SetLineColor(kMagenta-3);

  NPL::Reaction dd("28Mg(d,d)28Mg@222");
  NPL::Reaction pp("28Mg(p,p)28Mg@222");

  TGraph* kdd = dd.GetKinematicLine3();
  TGraph* kpp = pp.GetKinematicLine3();
  TGraph* tdd = dd.GetELabVersusThetaCM();
  TGraph* tpp = pp.GetELabVersusThetaCM();

  const double* xdd= kdd->GetX();
  const double* ydd =  kdd->GetY();
  kdd= new TGraph(kdd->GetN(),ydd,xdd);

  const double* xpp= kpp->GetX();
  const double* ypp =  kpp->GetY();
  kpp= new TGraph(kpp->GetN(),ypp,xpp);

  const double* xdd1= tdd->GetX();
  const double* ydd1 =  tdd->GetY();
  tdd= new TGraph(tdd->GetN()/2,xdd1,ydd1);

  const double* xpp2= tpp->GetX();
  const double* ypp2 =  tpp->GetY();
  tpp= new TGraph(tpp->GetN()/2,xpp2,ypp2);

  TH1D* hdd = new TH1D("hdd","hdd",300,0,180);
  TH1D* hpp = new TH1D("hpp","hpp",300,0,180);

  gdd = new TGraphErrors();
  TGraphErrors* gpp = new TGraphErrors();

  TF1* f = new TF1("f","gaus(0)+gaus(3)+pol4(6)",0,90);
  f->SetNpx(1000);
  f->SetParameter(0,20);
  f->SetParameter(2,1);

  f->SetParameter(3,20);
  f->SetParameter(5,1);

  f->SetParameter(6,3);
  f->SetParameter(7,-0.01);

  f->SetParLimits(0,0,50);
  f->SetParLimits(3,0,50);

  f->SetParLimits(2,1,2);
  f->SetParLimits(5,1,2);

  TF1* g = new TF1("g","gaus(0)",0,90);

  unsigned int step = 100;
  double eps = 30./step;
  cout << "EPSILON = " << eps <<" MeV" <<  endl;
  double E=0;
  TH1D* p; 
  double* r;
  for(unsigned int i = 0 ; i < step ; i++){
    E = i*eps;

    double lTCMd = tdd->Eval(E-eps*0.5);
    double uTCMd = tdd->Eval(E+eps*0.5);

    p = h2->ProjectionX(Form("p%i",i),h2->GetYaxis()->FindBin(E-eps*0.5),h2->GetYaxis()->FindBin(E+eps*0.5));
    if(p->GetEntries()>300){
      double Td = kdd->Eval(E);
      double Tp = kpp->Eval(E);
      double TCMd = tdd->Eval(E);
      double TCMp = tpp->Eval(E);

      p->Rebin(4);

      p->Draw();

      f->SetParameter(1,Td);
      f->SetParameter(4,Tp);

      f->SetParLimits(1,Td,Td);
      f->SetParLimits(4,Tp,Tp);  
      double pi,po;
      f->GetParLimits(3,pi,po);
      p->Fit(f,"RBFQ");
      gPad->Update();

      r = f->GetParameters();
      g->SetParameter(0,r[0]);
      g->SetParameter(1,r[1]);
      g->SetParameter(2,r[2]);

      double dd = g->Integral(r[1]-3*r[2],r[1]+3*r[2]);
      double ddo = dd ;
      double dde = sqrt(dd);
      double mynorm = 1;
      double EFF = eff->Integral(eff->FindBin(uTCMd),eff->FindBin(lTCMd))/abs(uTCMd-lTCMd);
      dd = mynorm*dd / eff->GetBinContent(eff->FindBin(TCMd));
      dde = mynorm*dde / eff->GetBinContent(eff->FindBin(TCMd));
      double ddx = 0.5*abs(uTCMd-lTCMd);
      if(dd<1e50 && ddo>10  && abs(Td-Tp)>1){
        hdd->Fill(tdd->Eval(E),dd);
        gdd->SetPoint(gdd->GetN(),TCMd,dd);
        gdd->SetPointError(gdd->GetN()-1,ddx,dde);
      }

      g->SetParameter(0,r[3]);
      g->SetParameter(1,r[4]);
      g->SetParameter(2,r[5]);
      double pp = g->Integral(0,90);
      double ppe = sqrt(pp); 

      hpp->Fill(tpp->Eval(E),pp);
      gpp->SetPoint(gpp->GetN(),TCMp,pp);
      gpp->SetPointError(gpp->GetN()-1,0,ppe);
//      gPad->WaitPrimitive();
    }
  }
  
  new TCanvas();
  gdd->Draw("ap");
  gdd->GetXaxis()->SetTitle("#theta_{CM} (deg)");
  gdd->GetYaxis()->SetTitle("d#sigma/d#Omega (mb/sr)");
  gdd->GetYaxis()->SetRangeUser(1,1e5);
  thdae->Draw("c");
  thpap->Draw("c");
  thloh->Draw("c");

  gPad->SetLogy();
  NumericalMinimization("Minuit2","Fumili2");
}

////////////////////////////////////////////////////////////////////////////////
double Chi2(const double* parameter){
  double Chi2 = 0 ;
  for (int i = 0; i < gdd->GetN() ; i++) {
    if(gdd->GetX()[i]<33 && gdd->GetX()[i]>15 && gdd->GetY()[i]>1){
		   double chi = ( thdae->Eval( gdd->GetX()[i] ) - parameter[0]*gdd->GetY()[i]) / (parameter[0]*gdd->GetEY()[i] );
       Chi2 += chi*chi ;
    }
  }
  if(Chi2<minC){
    minC= Chi2;
    minX= parameter[0];
  }
  return Chi2;
}

////////////////////////////////////////////////////////////////////////////////
void NumericalMinimization(const char * minName ,const char *algoName){
  ROOT::Math::Minimizer* min = 
    ROOT::Math::Factory::CreateMinimizer(minName, algoName);

  // set tolerance , etc...
  min->SetMaxFunctionCalls(100000); // for Minuit/Minuit2 
  min->SetPrecision(1e-6);
  min->SetValidError(true);
  min->SetPrintLevel(1);

  // create funciton wrapper for minimizer
  // a IMultiGenFunction type 
  ROOT::Math::Functor f(&Chi2,1); 

  min->SetFunction(f);
  double* parameter = new double[1];
  parameter[0] = 1e-4;
  // Set the free variables to be minimized!
  min->SetLimitedVariable(0,"Normalistation",parameter[0],1e-5,1e-5,1);

  // do the minimization
  min->Minimize(); 

  const double *xs = min->X();
  std::cout << "Minimum = " << xs[0] <<  " but MinX = " << minX << std::endl;

  for(int i = 0; i <  gdd->GetN() ; i++ ){
    gdd->SetPoint(i,gdd->GetX()[i],minX*gdd->GetY()[i]);
    gdd->SetPointError(i,gdd->GetEX()[i],minX*gdd->GetEY()[i]);

  }
  return 0;
}
