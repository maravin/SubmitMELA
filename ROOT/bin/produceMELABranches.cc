#include "PhysicsTools/FWLite/interface/CommandLineParser.h"
#include "FWCore/ParameterSet/interface/FileInPath.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>

#include "TSystem.h"
#include "TMath.h"
#include "TKey.h"
#include "TLorentzVector.h"
#include "TFile.h"
#include "TROOT.h"
#include "TTree.h"
#include "TString.h"
#include "TH1.h"

#include "ZZMatrixElement/MELA/interface/Mela.h"
#include "ZZMatrixElement/MELA/interface/TUtil.hh"

using namespace std;

Mela mela(13.0, 125.6, TVar::ERROR);

void calculateME(TLorentzVector tau1, TLorentzVector tau2, TLorentzVector jet1, TLorentzVector jet2,
		 int tauCharge1, int tauCharge2,
		 float& ME_sm, float& ME_sm_ggH, float& ME_sm_WH, float& ME_sm_ZH,
		 float& ME_bkg1, float& ME_bkg2,
		 float& Q2V1, float& Q2V2, float& costheta1, float& costheta2, float& Phi, float& costhetastar, float& Phi1);

void processFile(TDirectory*  dir, optutl::CommandLineParser parser, char treeToUse[], bool trueTau = true);

// files from Tyler R. to copy directory/root file content exactly
void copyFiles( optutl::CommandLineParser parser, TFile* fOld, TFile* fNew) ;
void CopyFile(const char *fname, optutl::CommandLineParser parser);
void CopyDir(TDirectory *source,optutl::CommandLineParser parser);

int main (int argc, char* argv[]) {

  optutl::CommandLineParser parser("Input parameters");
  parser.addOption("newFile",optutl::CommandLineParser::kString,"newFile","newFile.root");
  parser.addOption("inputFile",optutl::CommandLineParser::kString,"input File");
  parser.addOption("trueTau",optutl::CommandLineParser::kBool,"use true 4-vectors of tau leptons",true);

  parser.parseArguments (argc, argv);
  std::cout << "Input commands:" 
	    << "\n -- input file: " << parser.stringValue("inputFile")
	    << "\n -- output file: " << parser.stringValue("newFile")
	    << "\n -- use true tau 4-vectors (1 - yes, 0 - no): " << parser.boolValue("trueTau") << std::endl;
	    
  char treeToUse[80] = "first";

  TFile *fProduce;
  TFile *f = new TFile(parser.stringValue("inputFile").c_str(),"READ");
  std::cout<<"Creating new outputfile"<<std::endl;
  std::string newFileName = parser.stringValue("newFile");

  fProduce = new TFile(newFileName.c_str(),"RECREATE");
  copyFiles(parser, f, fProduce);//new TFile(parser.stringValue("inputFile").c_str()+"SVFit","UPDATE");
  fProduce = new TFile(newFileName.c_str(),"UPDATE");
  std::cout<<"listing the directories================="<<std::endl;
  fProduce->ls();

  processFile(fProduce, parser,  treeToUse, parser.boolValue("trueTau"));
  f->Close();

  return 0;
}


void processFile(TDirectory*  dir, optutl::CommandLineParser parser, char treeToUse[], bool trueTau)
{
  std::cout << "Starting!!!" << std::endl;
  std::cout << "treeToUse: " << treeToUse << std::endl;
  char stringA[80]="first";

  TDirectory *dirsav = gDirectory;
  TIter next(dir->GetListOfKeys());
  TKey *key;

  dir->cd();
  while ((key = (TKey*)next())) {
    printf("Found key=%s \n",key->GetName());

    TObject *obj = key->ReadObj();
    if (obj->IsA()->InheritsFrom(TDirectory::Class())) {
      dir->cd(key->GetName());
      TDirectory *subdir = gDirectory;
      sprintf(treeToUse,"%s",key->GetName());
      processFile(subdir, parser, treeToUse, parser.boolValue("trueTau"));

      dirsav->cd();
    } else if(obj->IsA()->InheritsFrom(TTree::Class())) {

      TTree* tree = (TTree*)obj;

      float ME_sm_VBF, ME_sm_ggH, ME_sm_WH, ME_sm_ZH, ME_bkg, ME_bkg1, ME_bkg2;
      float mela_Dbkg_VBF;
      float mela_Dbkg_ggH;
      float mela_Dbkg_WH;
      float mela_Dbkg_ZH;
      
      // angles
      float Q2V1;
      float Q2V2;
      float costheta1;
      float costheta2;
      float Phi;
      float costhetastar;
      float Phi1;
      
      Float_t q_1, q_2;
      Float_t m_sv;
      Float_t pt_sv;
      Float_t eta_sv;
      Float_t phi_sv;
      Int_t   njets;
      Float_t jpt_1, jeta_1, jphi_1;
      Float_t jpt_2, jeta_2, jphi_2;
      Float_t pt_2;
      Float_t phi_2;
      Float_t eta_2;
      Float_t m_2;
      Float_t pt_1;
      Float_t phi_1;
      Float_t eta_1;
      Float_t m_1;
      
      Float_t tau1_pt;
      Float_t tau1_eta;
      Float_t tau1_phi;
      Float_t tau1_m;
      
      Float_t tau2_pt;
      Float_t tau2_eta;
      Float_t tau2_phi;
      Float_t tau2_m;
      
      
      TBranch *b_q_1;
      TBranch *b_q_2;
      TBranch *b_njets;   //!
      TBranch *b_jpt_1;   //!
      TBranch *b_jeta_1;   //!
      TBranch *b_jphi_1;   //!
      TBranch *b_jpt_2;   //!
      TBranch *b_jeta_2;   //!
      TBranch *b_jphi_2;   //!
      TBranch *b_m_sv;   //!
      TBranch *b_pt_sv;   //!
      TBranch *b_eta_sv;   //!
      TBranch *b_phi_sv;   //!
      TBranch *b_pt_2;   //!
      TBranch *b_phi_2;   //!
      TBranch *b_eta_2;   //!
      TBranch *b_m_2;   //!
      TBranch *b_pt_1;   //!
      TBranch *b_phi_1;   //!
      TBranch *b_eta_1;   //!
      TBranch *b_m_1;   //!
      TBranch *b_tau1_pt;
      TBranch *b_tau1_phi;
      TBranch *b_tau1_eta;
      TBranch *b_tau1_m;
      
      TBranch *b_tau2_pt;
      TBranch *b_tau2_phi;
      TBranch *b_tau2_eta;
      TBranch *b_tau2_m;
      
      tree->SetBranchAddress("q_1", &q_1, &b_q_1);
      tree->SetBranchAddress("q_2", &q_2, &b_q_2);
      
      tree->SetBranchAddress("tau1_pt", &tau1_pt, &b_tau1_pt);
      tree->SetBranchAddress("tau1_eta", &tau1_eta, &b_tau1_eta);
      tree->SetBranchAddress("tau1_phi", &tau1_phi, &b_tau1_phi);
      tree->SetBranchAddress("tau1_m", &tau1_m, &b_tau1_m);
      
      tree->SetBranchAddress("tau2_pt", &tau2_pt, &b_tau2_pt);
      tree->SetBranchAddress("tau2_eta", &tau2_eta, &b_tau2_eta);
      tree->SetBranchAddress("tau2_phi", &tau2_phi, &b_tau2_phi);
      tree->SetBranchAddress("tau2_m", &tau2_m, &b_tau2_m);
      
      tree->SetBranchAddress("njets", &njets, &b_njets);
      tree->SetBranchAddress("jpt_1", &jpt_1, &b_jpt_1);
      tree->SetBranchAddress("jeta_1", &jeta_1, &b_jeta_1);
      tree->SetBranchAddress("jphi_1", &jphi_1, &b_jphi_1);
      tree->SetBranchAddress("jpt_2", &jpt_2, &b_jpt_2);
      tree->SetBranchAddress("jeta_2", &jeta_2, &b_jeta_2);
      tree->SetBranchAddress("jphi_2", &jphi_2, &b_jphi_2);
      tree->SetBranchAddress("m_sv", &m_sv, &b_m_sv);
      tree->SetBranchAddress("pt_sv", &pt_sv, &b_pt_sv);
      tree->SetBranchAddress("eta_sv", &eta_sv, &b_eta_sv);
      tree->SetBranchAddress("phi_sv", &phi_sv, &b_phi_sv);
      tree->SetBranchAddress("pt_1", &pt_1, &b_pt_1);
      tree->SetBranchAddress("phi_1", &phi_1, &b_phi_1);
      tree->SetBranchAddress("eta_1", &eta_1, &b_eta_1);
      tree->SetBranchAddress("m_1", &m_1, &b_m_1);
      tree->SetBranchAddress("pt_2", &pt_2, &b_pt_2);
      tree->SetBranchAddress("phi_2", &phi_2, &b_phi_2);
      tree->SetBranchAddress("eta_2", &eta_2, &b_eta_2);
      tree->SetBranchAddress("m_2", &m_2, &b_m_2);

      // new branches that will need to be filled
      vector<TBranch*> newBranches;
      newBranches.push_back(tree->Branch("Dbkg_VBF", &mela_Dbkg_VBF));
      newBranches.push_back(tree->Branch("Dbkg_ggH", &mela_Dbkg_ggH));
      newBranches.push_back(tree->Branch("Dbkg_WH", &mela_Dbkg_WH));
      newBranches.push_back(tree->Branch("Dbkg_ZH", &mela_Dbkg_ZH));
      // ME
      newBranches.push_back(tree->Branch("ME_sm_VBF", &ME_sm_VBF));
      newBranches.push_back(tree->Branch("ME_sm_ggH", &ME_sm_ggH));
      newBranches.push_back(tree->Branch("ME_sm_WH", &ME_sm_WH));
      newBranches.push_back(tree->Branch("ME_sm_ZH", &ME_sm_ZH));
      newBranches.push_back(tree->Branch("ME_bkg", &ME_bkg));
      newBranches.push_back(tree->Branch("ME_bkg1", &ME_bkg1));
      newBranches.push_back(tree->Branch("ME_bkg2", &ME_bkg2));
      
      // angles
      newBranches.push_back(tree->Branch("Q2V1", &Q2V1));
      newBranches.push_back(tree->Branch("Q2V2", &Q2V2));
      newBranches.push_back(tree->Branch("costheta1", &costheta1));
      newBranches.push_back(tree->Branch("costheta2", &costheta2));
      newBranches.push_back(tree->Branch("Phi", &Phi));
      newBranches.push_back(tree->Branch("costhetastar", &costhetastar));
      newBranches.push_back(tree->Branch("Phi1", &Phi1));
      
      float mjj = 0;
      newBranches.push_back(tree->Branch("mjj", &mjj));

      Long64_t nentries = tree->GetEntries();

      Long64_t nbytes = 0, nb = 0;
      for (Long64_t jentry=0; jentry<nentries;jentry++) {
	if ( jentry % 100000 == 0 ) std::cout << jentry << std::endl;
	nb = tree->GetEntry(jentry);   nbytes += nb;
	
	mjj = 0;
	ME_sm_VBF = -100; // ME for SM process VBF H->tt
	ME_sm_ggH = -100; // ME for ggH + 2 jets
	ME_sm_WH  = -100; // ME for WH (W->jj)
	ME_sm_ZH  = -100; // ME for ZH (Z->jj)
	
	ME_bkg1 = -100;   // ME for Z+2jets with leading jet being first, trailing second
	ME_bkg2 = -100;   // ME for Z+2jets with trailing jet being first, leading second
	ME_bkg  = -100;   // Sum of the two above (what we need to use)
	
	mela_Dbkg_VBF = -100; // ME_sm_VBF / (ME_sm_VBF + ME_bkg) <- normalized probability to separate H->tt and Z->tt
	mela_Dbkg_ggH = -100; // same for ggH and Ztt
	mela_Dbkg_WH  = -100; // same for WH and Ztt
	mela_Dbkg_ZH  = -100; // same for ZH and Ztt
	
	// angles inputs to MELA
	Q2V1 = -100;
	Q2V2 = -100;
	costheta1    = -100;
	costheta2    = -100;
	Phi          = -100;
	costhetastar = -100;
	Phi1         = -100;
	
	if (njets>=2){
	  
	  TLorentzVector tau1, tau2;
	  tau1.SetPtEtaPhiM(tau1_pt, tau1_eta, tau1_phi, tau1_m);
	  tau2.SetPtEtaPhiM(tau2_pt, tau2_eta, tau2_phi, tau2_m);
	  
	  // jet 4-vectors
	  TLorentzVector jet1(0, 0, 1e-3, 1e-3), jet2(0, 0, 1e-3, 1e-3), higgs(0, 0, 0, 0),
	    blank1(0, 0, 0, 0);
	  jet1.SetPtEtaPhiM(jpt_1, jeta_1, jphi_1, 0);
	  jet2.SetPtEtaPhiM(jpt_2, jeta_2, jphi_2, 0);
	  mjj = (jet1 +  jet2).M();
	  
	  // tau lepton 4-vectors
	  TLorentzVector pDaughters1, pDaughters2;
	  
	  TLorentzVector visTau1, visTau2;
	  visTau1.SetPtEtaPhiM(pt_1, eta_1, phi_1, m_1);
	  visTau2.SetPtEtaPhiM(pt_2, eta_2, phi_2, m_2);
	  
	  if ( !trueTau ) {
	    pDaughters1 = visTau1;
	    pDaughters2 = visTau2;
	  } else {
	    pDaughters1 = tau1;
	    pDaughters2 = tau2;
	  }
	  
	  
	  
	  // Determine the signs of the tau leptons      
	  int tauCharge1 = q_1;
	  int tauCharge2 = q_2;
	  
	  if ( tau1.DeltaR(visTau1) > tau1.DeltaR(visTau2) )
	    tauCharge1 = q_2;
	  if ( tau2.DeltaR(visTau2) > tau2.DeltaR(visTau1) ) 
	    tauCharge2 = q_1;
	  
	  calculateME(pDaughters1, pDaughters2, jet1, jet2, tauCharge1, tauCharge2,
		      ME_sm_VBF, ME_sm_ggH, ME_sm_WH, ME_sm_ZH, ME_bkg1, ME_bkg2,
		      Q2V1, Q2V2, costheta1, costheta2, Phi, costhetastar, Phi1);
	  
	  
	  ME_bkg = ME_bkg1 + ME_bkg2;
	  
	  mela_Dbkg_VBF = ME_sm_VBF / ( ME_sm_VBF + ME_bkg);
	  mela_Dbkg_ggH = ME_sm_ggH / ( ME_sm_ggH + ME_bkg);
	  mela_Dbkg_WH  = ME_sm_WH / ( ME_sm_WH + ME_bkg);
	  mela_Dbkg_ZH  = ME_sm_ZH / ( ME_sm_ZH + ME_bkg);
	}
	// Fill new branches
	for(auto branchToFill : newBranches) branchToFill->Fill();

      } // loop over events

      dir->cd();
      tree->Write("", TObject::kOverwrite);
      strcpy(treeToUse, stringA);

    }
  }
}

void calculateME(TLorentzVector tau1, TLorentzVector tau2, TLorentzVector jet1, TLorentzVector jet2,
                 int tauCharge1, int tauCharge2,
                 float& ME_sm_VBF, float& ME_sm_ggH, float& ME_sm_WH, float& ME_sm_ZH,
                 float& ME_bkg1, float& ME_bkg2,
                 float& Q2V1, float& Q2V2, float& costheta1, float& costheta2, float& Phi, float& costhetastar, float& Phi1) {

  /*
  std::cout << "Input " << std::endl;
  std::cout << "\t tau1: " << tau1.Pt() << "\t" << tau1.Eta() << "\t" << tau1.Phi() << "\t" << tau1.M() << std::endl;
  std::cout << "\t tau2: " << tau2.Pt() << "\t" << tau2.Eta() << "\t" << tau2.Phi() << "\t" << tau2.M() << std::endl;
  std::cout << "\t jet1: " << jet1.Pt() << "\t" << jet1.Eta() << "\t" << jet1.Phi() << "\t" << jet1.M() << std::endl;
  std::cout << "\t jet2: " << jet2.Pt() << "\t" << jet2.Eta() << "\t" << jet2.Phi() << "\t" << jet2.M() << std::endl;
  std::cout << "And charges: " << tauCharge1 << "\t" << tauCharge2 << std::endl;
  std::cout << std::endl;
  */

  // tau lepton 4-vectors
  SimpleParticleCollection_t daughters;
  daughters.push_back(SimpleParticle_t(15*tauCharge1, tau1)); 
  daughters.push_back(SimpleParticle_t(15*tauCharge2, tau2));

  SimpleParticleCollection_t associated;
  associated.push_back(SimpleParticle_t(0, jet1));
  associated.push_back(SimpleParticle_t(0, jet2));
  
  // SM Higgs hypothesis ===========================================================
  mela.setCandidateDecayMode(TVar::CandidateDecay_ff);
  mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);
  mela.setProcess(TVar::HSMHiggs, TVar::JHUGen, TVar::JJVBF);
  mela.computeProdP(ME_sm_VBF, false);

  mela.computeVBFAngles(Q2V1, Q2V2, costheta1, costheta2, Phi, costhetastar, Phi1);
  
  //get the ggH + 2jets hypothesis
  mela.setProcess(TVar::SelfDefine_spin0, TVar::JHUGen, TVar::JJQCD);
  mela.selfDHggcoupl[0][gHIGGS_GG_2][0] = 1;
  mela.computeProdP(ME_sm_ggH, false);
  
  //get the WH + 2 jets hypothesis
  mela.setProcess(TVar::SelfDefine_spin0, TVar::JHUGen, TVar::Had_WH);
  mela.selfDHzzcoupl[0][gHIGGS_VV_1][0] = 1;
  mela.computeProdP(ME_sm_WH, false);
  
  //get the ZH + 2 jets hypothesis
  mela.setProcess(TVar::SelfDefine_spin0, TVar::JHUGen, TVar::Had_ZH);
  mela.selfDHzzcoupl[0][gHIGGS_VV_1][0] = 1;
  mela.computeProdP(ME_sm_ZH, false);
  
  // Z+2jets
  mela.setProcess(TVar::bkgZJets, TVar::MCFM, TVar::JJQCD);
  mela.computeProdP(ME_bkg1, false);
  
  associated.clear();
  associated.push_back(SimpleParticle_t(0, jet2));
  associated.push_back(SimpleParticle_t(0, jet1));
  mela.setInputEvent(&daughters, &associated, (SimpleParticleCollection_t*)0, false);
  mela.setProcess(TVar::bkgZJets, TVar::MCFM, TVar::JJQCD);
  mela.computeProdP(ME_bkg2, false);
  mela.resetInputEvent();
  return;
}

//Thank you Renee Brun :)                                                                                                                                                  
void CopyDir(TDirectory *source, optutl::CommandLineParser parser) {
  //copy all objects and subdirs of directory source as a subdir of the current directory   
  TDirectory *savdir = gDirectory;
  TDirectory *adir = savdir;
  if(source->GetName()!=parser.stringValue("inputFile")){
    adir = savdir->mkdir(source->GetName());
    std::cout<<"Source name is not outputfile name"<<std::endl;
    adir->cd();
  }
  else{
    //adir = savdir->mkdir("input");                                                     
    adir->cd();
  }

  //loop on all entries of this directory                                                                                                                                                               
  TKey *key;
  TIter nextkey(source->GetListOfKeys());
  while ((key = (TKey*)nextkey())) {
    const char *classname = key->GetClassName();
    TClass *cl = gROOT->GetClass(classname);
    if (!cl) continue;
    if (cl->InheritsFrom(TDirectory::Class())) {
      source->cd(key->GetName());
      TDirectory *subdir = gDirectory;
      adir->cd();
      CopyDir(subdir,parser);
      adir->cd();
    } else if (cl->InheritsFrom(TTree::Class())) {
      TTree *T = (TTree*)source->Get(key->GetName());
      adir->cd();
      TTree *newT = T->CloneTree(-1,"fast");
      newT->Write();
    } else {
      source->cd();
      TObject *obj = key->ReadObj();
      adir->cd();
      obj->Write();
      delete obj;
    }
  }
  adir->SaveSelf(kTRUE);
  savdir->cd();
}

void CopyFile(const char *fname, optutl::CommandLineParser parser) {
  //Copy all objects and subdirs of file fname as a subdir of the current directory                                                                                                                     
  TDirectory *target = gDirectory;
  TFile *f = TFile::Open(fname);
  if (!f || f->IsZombie()) {
    printf("Cannot copy file: %s\n",fname);
    target->cd();
    return;
  }
  target->cd();
  CopyDir(f,parser);
  delete f;
  target->cd();
}
void copyFiles( optutl::CommandLineParser parser, TFile* fOld, TFile* fNew)
{
  //prepare files to be copied                                                                                                         
  if(gSystem->AccessPathName(parser.stringValue("inputFile").c_str())) {
    gSystem->CopyFile("hsimple.root", parser.stringValue("inputFile").c_str());
  }

  fNew->cd();
  CopyFile(parser.stringValue("inputFile").c_str(),parser);
  fNew->ls();
  fNew->Close();

}


