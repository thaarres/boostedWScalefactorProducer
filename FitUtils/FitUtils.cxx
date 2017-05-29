#include "FitUtils.h"

void fit_mj_single_MC(RooWorkspace* workspace, const std::string & fileName, const std::string & label, const std::string & model,const std::string & channel,const std::string & wtagger_label){

	std::cout<<" Fit mj single MC sample"<<fileName<<" "<<label<<"  "<<model<<std::endl;
	//import variable and dataset
	RooRealVar* rrv_mass_j  = workspace->var("rrv_mass_j");
	RooDataSet* rdataset_mj = (RooDataSet*) workspace->data(("rdataset4fit"+label+"_"+channel+"_mj").c_str());

	// make the extended model  
	std::vector<std::string>* constraint_list = new std::vector<std::string>(); 
	RooExtendPdf* model_pdf = MakeExtendedModel(workspace,label,model,"_mj",channel,wtagger_label,constraint_list);

	RooFitResult* rfresult = model_pdf->fitTo(*rdataset_mj,RooFit::Save(1),RooFit::SumW2Error(kTRUE),RooFit::Extended(kTRUE), RooFit::Minimizer("Minuit2"),RooFit::Verbose(kFALSE));
	rfresult               = model_pdf->fitTo(*rdataset_mj,RooFit::Save(1),RooFit::SumW2Error(kTRUE),RooFit::Extended(kTRUE), RooFit::Minimizer("Minuit2"),RooFit::Verbose(kFALSE));
	rfresult               = model_pdf->fitTo(*rdataset_mj,RooFit::Save(1),RooFit::SumW2Error(kTRUE),RooFit::Extended(kTRUE), RooFit::Minimizer("Minuit2"));
  

	std::cout<<""<<std::endl;std::cout<<""<<std::endl;
	std::cout<<"PRINTING FIT RESULT!!!!!!!"<<std::endl;
	rfresult->Print();
	std::cout<<""<<std::endl;std::cout<<""<<std::endl;


	//##### apply the correction of the mean and sigma from the ttbar control sample to the STop, TTbar and VV
	RooArgSet* parameters_list = model_pdf->getParameters(*rdataset_mj);
	TIter par = parameters_list->createIterator();
	par.Reset();

	// par=parameters_list.createIterator();
	// par.Reset();
	// param=par.Next()
	// while (param):
	//      if (TString(label).Contains("VV") or TString(label).Contains("STop") or TString(label).Contains("TTbar")) and (not (options.fitwtaggersim or options.fitwtagger)):
	//          #param.Print();
	//          if TString(param.GetName()).Contains("rrv_mean1_gaus"):
	//              param.setRange(param.getMin()+self.mean_shift, param.getMax()+self.mean_shift);
	//              param.setVal(param.getVal()+self.mean_shift);
	//              #param.Print(); raw_input("mean"+label);
	//          if TString(param.GetName()).Contains("rrv_deltamean_gaus"):
	//              param.setRange(param.getMin()-self.mean_shift, param.getMax()-self.mean_shift);
	//              param.setVal(param.getVal()-self.mean_shift);
	//              #param.Print(); raw_input("mean"+label);
	//          if TString(param.GetName()).Contains("rrv_sigma1_gaus"):
	//              param.setVal(param.getVal()*self.sigma_scale);
	//              param.setRange(param.getMin()*self.sigma_scale, param.getMax()*self.sigma_scale);
	//              #param.Print(); raw_input("sigma"+label);
	//          if TString(param.GetName()).Contains("rrv_scalesigma_gaus"):
	//              param.setRange(param.getMin()/self.sigma_scale, param.getMax()/self.sigma_scale);
	//              param.setVal(param.getVal()/self.sigma_scale);
	//              #param.Print(); raw_input("sigma"+label);
	//      param=par.Next()
  
  
	workspace->import(*model_pdf);
	workspace->import(*rfresult);
  
	std::cout<<" Plot name = " << (label+" fitted by "+model).c_str() << std::endl;

	// Plot the result
	RooPlot* mplot = rrv_mass_j->frame(RooFit::Title((label+" fitted by "+model).c_str()), RooFit::Bins(int(rrv_mass_j->getBins())));
	mplot->GetYaxis()->SetRangeUser(0,mplot->GetMaximum()*1.2);
	rdataset_mj->plotOn(mplot,RooFit::MarkerSize(1.5),RooFit::DataError(RooAbsData::SumW2),RooFit::XErrorSize(0),RooFit::Invisible());
  

	// ## draw the error band for an extend pdf
	draw_error_band_extendPdf(rdataset_mj,model_pdf,rfresult,mplot,2,"L");
	model_pdf->plotOn(mplot,RooFit::Name( "Gaussian comp. 1" ),RooFit::Components("gaus1*"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed+3));
	model_pdf->plotOn(mplot,RooFit::Name( "Gaussian comp. 2" ),RooFit::Components("gaus2*"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed+4));
	model_pdf->plotOn(mplot,RooFit::Name( "ErfExp comp." ),RooFit::Components("erfExp*"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed+3));
	model_pdf->plotOn(mplot,RooFit::Name( "Exp. comp." ),RooFit::Components("exp*"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed+2));
	model_pdf->plotOn(mplot,RooFit::Name( "Chebychev comp." ),RooFit::Components("cheb*"),RooFit::LineStyle(kDashed),RooFit::LineColor(kRed+3));
	model_pdf->plotOn(mplot,RooFit::Name((model).c_str() )); // remove RooFit.VLines() in order to get right pull in the 1st bin

	//## re-draw the dataset
	rdataset_mj->plotOn(mplot,RooFit::MarkerSize(1.),RooFit::Name( (label).c_str() ), RooFit::DataError(RooAbsData::SumW2), RooFit::XErrorSize(1));
	TLegend* leg1 = legend4Plot(mplot,1,-0.2,0.15,0.00,0.,0,channel);
	mplot->addObject(leg1);

	//## Get the pull
	RooPlot* mplot_pull = get_ratio(rrv_mass_j,rdataset_mj,model_pdf,rfresult,0,1);
	mplot->GetYaxis()->SetRangeUser(0,mplot->GetMaximum()*1.2);
	mplot->GetYaxis()->SetTitle(" MC events / 5 GeV");
  
	//## CALCULATE CHI2
	RooDataHist* datahist = rdataset_mj->binnedClone((std::string(rdataset_mj->GetName())+"_binnedClone").c_str(),(std::string(rdataset_mj->GetName())+"_binnedClone").c_str());
	int Nbin = int(rrv_mass_j->getBins());
	RooArgList rresult_param = rfresult->floatParsFinal();
	int nparameters =  rresult_param.getSize();
	RooAbsReal* ChiSquare = model_pdf->createChi2(*datahist,RooFit::Extended(kTRUE),RooFit::DataError(RooAbsData::Poisson));
	float chi_over_ndf= ChiSquare->getVal()/(Nbin - nparameters);

	//## Add Chisquare to mplot_pull
	TString Name ; Name.Form("#chi^{2}/ndf = %0.2f ",float(chi_over_ndf));
	TLatex* cs = new TLatex(0.75,0.8,Name.Data());
	cs->SetNDC();
	cs->SetTextSize(0.12);
	cs->AppendPad("same");
	mplot_pull->addObject(cs);
   
	TString command; 
	command.Form("mkdir -p plots/plots_%s_%s_MCfits/",channel.c_str(),wtagger_label.c_str());
	system(command.Data());

	command.Form("plots/plots_%s_%s_MCfits/",channel.c_str(),wtagger_label.c_str());
	draw_canvas_with_pull(mplot,mplot_pull,new RooArgList(*parameters_list),std::string(command.Data()),label+fileName,model,channel,0,0,GetLumi());
  
	workspace->var(("rrv_number"+label+"_"+channel+"_mj").c_str())->setVal(workspace->var(("rrv_number"+label+"_"+channel+"_mj").c_str())->getVal()*workspace->var(("rrv_scale_to_lumi"+label+"_"+channel).c_str())->getVal());
	workspace->var(("rrv_number"+label+"_"+channel+"_mj").c_str())->setError(workspace->var(("rrv_number"+label+"_"+channel+"_mj").c_str())->getError()*workspace->var(("rrv_scale_to_lumi"+label+"_"+channel).c_str())->getVal());
}

// method to do fail and pass fit for the scale factor evaluation                                                                                                                      
void ScaleFactorTTbarControlSampleFit(RooWorkspace* workspace, std::map<std::string,std::string > mj_shape, std::map<std::string,int> color_palet, std::vector<std::string>* constraintlist_data, std::vector<std::string>* constraintlist_MC, const std::string & label, const std::string & channel, const std::string & wtagger, const double & ca8_ungroomed_pt_min, const double & ca8_ungroomed_pt_max){
  
  //TODO! Add method to calculate tt SF on the go!!!
  
  float ttSF = 0.667028210351; // powheg80X

  std::cout<<"############################## Pass: dataset #################################### "<<std::endl;

  RooRealVar* rrv_mass_j = workspace->var("rrv_mass_j");
  RooDataSet* rdataset_data_mj  = (RooDataSet*) workspace->data(("rdataset_data"+ label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj = (RooDataSet*) workspace->data(("rdataset_TTbar"+ label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_STop_mj  = (RooDataSet*) workspace->data(("rdataset_STop"+  label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_VV_mj    = (RooDataSet*) workspace->data(("rdataset_VV"+    label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_WJets_mj = (RooDataSet*) workspace->data(("rdataset_WJets0"+label+"_"+channel+"_mj").c_str());
  // RooDataSet* rdataset_WJets_mj = (RooDataSet*) workspace->data(("rdataset_QCD"+label+"_"+channel+"_mj").c_str());
  //Plot in merged and unmerged comp
  RooDataSet* rdataset_TTbar_mj_merged   = (RooDataSet*) workspace->data(("rdataset_TTbar_realW"+ label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_unmerged = (RooDataSet*) workspace->data(("rdataset_TTbar_fakeW"+ label+"_"+channel+"_mj").c_str());
  
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_STop_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_VV_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_WJets_mj);
  // change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_QCD_mj);
  //Plot in merged and unmerged comp
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj_merged);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj_unmerged);
  
  std::cout<<"############################### Pass: Histograms for plotting reasons  ################################### "<<std::endl;
  RooAbsPdf* model_histpdf_TTbar = workspace->pdf((std::string(rdataset_TTbar_mj->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_STop  = workspace->pdf((std::string(rdataset_STop_mj->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_VV    = workspace->pdf((std::string(rdataset_VV_mj->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_WJets = workspace->pdf((std::string(rdataset_WJets_mj->GetName())+"_histpdf").c_str());
  // RooAbsPdf* model_histpdf_QCD   = workspace->pdf((std::string(rdataset_QCD_mj->GetName())+"_histpdf").c_str());
  //Plot in merged and unmerged comp
   RooAbsPdf* model_histpdf_TTbar_merged   = workspace->pdf((std::string(rdataset_TTbar_mj_merged->GetName())+"_histpdf").c_str());
   RooAbsPdf* model_histpdf_TTbar_unmerged = workspace->pdf((std::string(rdataset_TTbar_mj_unmerged->GetName())+"_histpdf").c_str());
   

  RooRealVar* number_TTbar = new RooRealVar(("rrv_number_TTbar"+label+"_"+channel).c_str(),("rrv_number_TTbar"+label+"_"+channel).c_str(),rdataset_TTbar_mj->sumEntries()*ttSF);
  RooRealVar* number_STop  = new RooRealVar(("rrv_number_STop"+label+"_"+channel).c_str(),("rrv_number_STop"+label+"_"+channel).c_str(),rdataset_STop_mj->sumEntries());
  RooRealVar* number_VV    = new RooRealVar(("rrv_number_VV"+label+"_"+channel).c_str(),("rrv_number_VV"+label+"_"+channel).c_str(),rdataset_VV_mj->sumEntries());
  RooRealVar* number_WJets = new RooRealVar(("rrv_number_WJets"+label+"_"+channel).c_str(),("rrv_number_WJets"+label+"_"+channel).c_str(),rdataset_WJets_mj->sumEntries());
  // RooRealVar* number_QCD   = new RooRealVar(("rrv_number_QCD"+label+"_"+channel).c_str(),("rrv_number_QCD"+label+"_"+channel).c_str(),rdataset_QCD_mj->sumEntries());
  //Plot in merged and unmerged comp
  RooRealVar* number_TTbar_merged   = new RooRealVar(("rrv_number_TTbar_realW"+label+"_"+channel).c_str(),("rrv_number_TTbar_realW"+label+"_"+channel).c_str(),rdataset_TTbar_mj_merged->sumEntries()*ttSF);
  RooRealVar* number_TTbar_unmerged = new RooRealVar(("rrv_number_TTbar_fakeW"+label+"_"+channel).c_str(),("rrv_number_TTbar_fakeW"+label+"_"+channel).c_str(),rdataset_TTbar_mj_unmerged->sumEntries()*ttSF);

  // RooAddPdf* model_TTbar_STop_VV_WJets = new RooAddPdf(("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),RooArgList(*model_histpdf_TTbar,*model_histpdf_STop,*model_histpdf_VV,*model_histpdf_WJets),RooArgList(*number_TTbar,*number_STop,*number_VV,*number_WJets));
  // RooAddPdf* model_TTbar_STop_VV_WJets = new RooAddPdf(("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),RooArgList(*model_histpdf_TTbar_merged,*model_histpdf_TTbar_unmerged,*model_histpdf_STop,*model_histpdf_VV,*model_histpdf_WJets,*model_histpdf_QCD),RooArgList(*number_TTbar_merged,*number_TTbar_unmerged,*number_STop,*number_VV,*number_WJets,*number_QCD));
  RooAddPdf* model_TTbar_STop_VV_WJets = new RooAddPdf(("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str(),RooArgList(*model_histpdf_TTbar_merged,*model_histpdf_TTbar_unmerged,*model_histpdf_STop,*model_histpdf_VV,*model_histpdf_WJets),RooArgList(*number_TTbar_merged,*number_TTbar_unmerged,*number_STop,*number_VV,*number_WJets));
  
  workspace->import(*model_TTbar_STop_VV_WJets);
  
  std::cout<<"############################## Fail: dataset #################################### "<<std::endl;
  RooDataSet* rdataset_data_mj_fail  = (RooDataSet*) workspace->data(("rdataset_data"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_fail = (RooDataSet*) workspace->data(("rdataset_TTbar"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_STop_mj_fail  = (RooDataSet*) workspace->data(("rdataset_STop"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_VV_mj_fail    = (RooDataSet*) workspace->data(("rdataset_VV"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_WJets_mj_fail = (RooDataSet*) workspace->data(("rdataset_WJets0"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  // RooDataSet* rdataset_QCD_mj_fail = (RooDataSet*) workspace->data(("rdataset_QCD"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  //Plot in merged and unmerged comp
  RooDataSet* rdataset_TTbar_mj_fail_merged   = (RooDataSet*) workspace->data(("rdataset_TTbar_realW"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_fail_unmerged = (RooDataSet*) workspace->data(("rdataset_TTbar_fakeW"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());

  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_STop_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_VV_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_WJets_mj_fail);
  // change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_QCD_mj_fail);
  //Plot in merged and unmerged comp
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj_fail_merged);
  change_dataset_to_histpdf(workspace,rrv_mass_j,rdataset_TTbar_mj_fail_unmerged);

  std::cout<<"############################### Fail: model ################################### "<<std::endl;
  RooAbsPdf* model_histpdf_TTbar_fail = workspace->pdf((std::string(rdataset_TTbar_mj_fail->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_STop_fail  = workspace->pdf((std::string(rdataset_STop_mj_fail->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_VV_fail    = workspace->pdf((std::string(rdataset_VV_mj_fail->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_WJets_fail = workspace->pdf((std::string(rdataset_WJets_mj_fail->GetName())+"_histpdf").c_str());
  // RooAbsPdf* model_histpdf_QCD_fail   = workspace->pdf((std::string(rdataset_QCD_mj_fail->GetName())+"_histpdf").c_str());
  //Plot in merged and unmerged comp
   RooAbsPdf* model_histpdf_TTbar_fail_merged   = workspace->pdf((std::string(rdataset_TTbar_mj_fail_merged->GetName())+"_histpdf").c_str());
   RooAbsPdf* model_histpdf_TTbar_fail_unmerged = workspace->pdf((std::string(rdataset_TTbar_mj_fail_unmerged->GetName())+"_histpdf").c_str());

  RooRealVar* number_TTbar_fail  = new RooRealVar(("rrv_number_TTbar_fail"+label+"_"+channel).c_str(),("rrv_number_TTbar_fail"+label+"_"+channel).c_str(),rdataset_TTbar_mj_fail->sumEntries()*ttSF);
  RooRealVar* number_STop_fail   = new RooRealVar(("rrv_number_STop_fail"+label +"_"+channel).c_str(),("rrv_number_STop_fail"+label+"_"+channel).c_str(),rdataset_STop_mj_fail->sumEntries());
  RooRealVar* number_VV_fail     = new RooRealVar(("rrv_number_VV_fail"+label   +"_"+channel).c_str(),("rrv_number_VV_fail"+label+"_"+channel).c_str(),rdataset_VV_mj_fail->sumEntries());
  RooRealVar* number_WJets_fail  = new RooRealVar(("rrv_number_WJets_fail"+label+"_"+channel).c_str(),("rrv_number_WJets_fail"+label+"_"+channel).c_str(),rdataset_WJets_mj_fail->sumEntries());
  // RooRealVar* number_QCD_fail    = new RooRealVar(("rrv_number_QCD_fail"+label+"_"+channel).c_str()  ,("rrv_number_QCD_fail"+label+"_"+channel).c_str(),rdataset_QCD_mj_fail->sumEntries());
  //Plot in merged and unmerged comp
  RooRealVar* number_TTbar_fail_merged    = new RooRealVar(("rrv_number_TTbar_fail_realW"+label+"_"+channel).c_str(),("rrv_number_TTbar_fail_realW"+label+"_"+channel).c_str(),rdataset_TTbar_mj_fail_merged->sumEntries()*ttSF);
  RooRealVar* number_TTbar_fail_unmerged  = new RooRealVar(("rrv_number_TTbar_fail_fakeW"+label+"_"+channel).c_str(),("rrv_number_TTbar_fail_fakeW"+label+"_"+channel).c_str(),rdataset_TTbar_mj_fail_unmerged->sumEntries()*ttSF);
  
  // RooAddPdf* model_TTbar_STop_VV_WJets_fail = new RooAddPdf(("model_TTbar_STop_VV_WJets_fail"+label+"_"+channel).c_str(),("model_TTbar_STop_VV_WJets_fail"+label+"_"+channel).c_str(),RooArgList(*model_histpdf_TTbar_fail,*model_histpdf_STop_fail,*model_histpdf_VV_fail,*model_histpdf_WJets_fail), RooArgList(*number_TTbar_fail,*number_STop_fail,*number_VV_fail,*number_WJets_fail));
  RooAddPdf* model_TTbar_STop_VV_WJets_fail = new RooAddPdf(("model_TTbar_STop_VV_WJets_fail"+label+"_"+channel).c_str(),("model_TTbar_STop_VV_WJets_fail"+label+"_"+channel).c_str(),RooArgList(*model_histpdf_TTbar_fail_merged,*model_histpdf_TTbar_fail_unmerged,*model_histpdf_STop_fail,*model_histpdf_VV_fail,*model_histpdf_WJets_fail), RooArgList(*number_TTbar_fail_merged,*number_TTbar_fail_unmerged,*number_STop_fail,*number_VV_fail,*number_WJets_fail));
  workspace->import(*model_TTbar_STop_VV_WJets_fail);
  
  
  double scale_number_TTbar_STop_VV_WJets      = (rdataset_TTbar_mj_merged->sumEntries()*ttSF+rdataset_TTbar_mj_unmerged->sumEntries()*ttSF+rdataset_STop_mj->sumEntries()+rdataset_VV_mj->sumEntries()+rdataset_WJets_mj->sumEntries())/(rdataset_data_mj->sumEntries()+rdataset_data_mj_fail->sumEntries());
  double scale_number_TTbar_STop_VV_WJets_fail = (rdataset_TTbar_mj_fail_merged->sumEntries()*ttSF+rdataset_TTbar_mj_fail_unmerged->sumEntries()*ttSF+rdataset_STop_mj_fail->sumEntries()+rdataset_VV_mj_fail->sumEntries()+rdataset_WJets_mj_fail->sumEntries())/( rdataset_data_mj->sumEntries()+rdataset_data_mj_fail->sumEntries());

  RooRealVar* rrv_scale_number_TTbar_STop_VV_WJets = new RooRealVar(("rrv_scale_number_TTbar_STop_VV_WJets"+label).c_str(),("rrv_scale_number_TTbar_STop_VV_WJets"+label).c_str(),scale_number_TTbar_STop_VV_WJets);
  RooRealVar* rrv_scale_number_TTbar_STop_VV_WJets_fail = new RooRealVar(("rrv_scale_number_TTbar_STop_VV_WJets_fail"+label).c_str(),("rrv_scale_number_TTbar_STop_VV_WJets_fail"+label).c_str(),scale_number_TTbar_STop_VV_WJets_fail);
  workspace->import(*rrv_scale_number_TTbar_STop_VV_WJets);
  workspace->import(*rrv_scale_number_TTbar_STop_VV_WJets_fail);


  /// All the shape parameters and normalization are fixed                                                                                                                          
  std::cout<<"############################### Pass: Single MC model ################################### "<<std::endl;
  RooAbsPdf* model_STop  = get_STop_mj_Model(workspace,"_STop"+label,mj_shape["STop"],channel);
  RooAbsPdf* model_VV    = get_VV_mj_Model(workspace,"_VV"+label,mj_shape["VV"],channel);
  RooAbsPdf* model_WJets = get_General_mj_Model(workspace,"_WJets0"+label,mj_shape["WJets0"],channel);
  // RooAbsPdf* model_QCD   = get_General_mj_Model(workspace,"_QCD"+label,mj_shape["QCD"],channel);

  std::cout<<"############################### Fail: Single MC model ################################### "<<std::endl;
  RooAbsPdf* model_STop_fail  = get_STop_mj_Model(workspace,"_STop"+label+"_failtau2tau1cut",mj_shape["STop_fail"],channel);
  RooAbsPdf* model_VV_fail    = get_VV_mj_Model(workspace,"_VV"+label+"_failtau2tau1cut",mj_shape["VV_fail"],channel);
  RooAbsPdf* model_WJets_fail = get_General_mj_Model(workspace,"_WJets0"+label+"_failtau2tau1cut",mj_shape["WJets0_fail"],channel);
  // RooAbsPdf* model_QCD_fail   = get_General_mj_Model(workspace,"_QCD"+label+"_failtau2tau1cut",mj_shape["QCD_fail"],channel);

  /// Model for unmatched events passing and failing the cut --> ErfExp                                                                                                              
  std::cout<<"############################### Pass: TTbar in Data Bkg ################################### "<<std::endl;
  RooAbsPdf* model_bkg_data = MakeExtendedModel(workspace,"_bkg_data"+label,mj_shape["bkg_data"],"_mj",channel,wtagger,constraintlist_data);
  std::cout<<"############################### Fail: TTbar in Data Bkg ################################### "<<std::endl;
  RooAbsPdf* model_bkg_data_fail = MakeExtendedModel(workspace,"_bkg_data"+label+"_failtau2tau1cut",mj_shape["bkg_data_fail"],"_mj",channel,wtagger,constraintlist_data);
  // Model for matched events passing and failing the cut --> 2Gaus_ttbar and GausChebychev_ttbar_failtau2tau1cut                                                                   
  std::cout<<"############################### Pass: Data ttbar resonant component ################################### "<<std::endl;
  RooAbsPdf* model_ttbar_data = MakeModelTTbarControlSample(workspace,"_ttbar_data"+label,mj_shape["signal_data"],"_mj",channel,wtagger,label,constraintlist_data);    
  std::cout<<"############################### Fail: Data ttbar resonant component ################################### "<<std::endl;
  RooAbsPdf* model_ttbar_data_fail = MakeModelTTbarControlSample(workspace,"_ttbar_data"+label+"_failtau2tau1cut",mj_shape["signal_data_fail"],"_mj",channel,wtagger,label,constraintlist_data);


  std::cout<<"############################### Total Data Pdf Fail ################################### "<<std::endl;
  RooAbsPdf* model_data_fail = new RooAddPdf(("model_data"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str(),("model_data+"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str(),RooArgList(*model_ttbar_data_fail,*model_bkg_data_fail,*model_STop_fail,*model_VV_fail,*model_WJets_fail));
  std::cout<<"############################### Total Data Pdf Pass ################################### "<<std::endl;
  RooAbsPdf* model_data      = new RooAddPdf(("model_data"+label+"_"+channel).c_str(),("model_data"+label+"_"+channel).c_str(), RooArgList(*model_ttbar_data,*model_bkg_data,*model_STop,*model_VV,*model_WJets));

  workspace->import(*model_data);
  workspace->import(*model_data_fail);

  
  /// fit TotalMC                                                                                                                                                                       

  std::cout<< " ############################### Pass: MC TTbar Bkg ################################### "<<std::endl;
  RooAbsPdf* model_bkg_TotalMC = MakeExtendedModel(workspace,"_bkg_TotalMC"+label,mj_shape["bkg_mc"],"_mj",channel,wtagger,constraintlist_MC);
  std::cout<< " ############################### Fail: MC TTbar Bkg ################################### "<<std::endl;
  RooAbsPdf* model_bkg_TotalMC_fail = MakeExtendedModel(workspace,"_bkg_TotalMC"+label+"_failtau2tau1cut",mj_shape["bkg_mc_fail"],"_mj",channel,wtagger,constraintlist_MC);
  std::cout<< " ############################### Pass: MC ttbar Resonant ################################### "<<std::endl;
  RooAbsPdf* model_ttbar_TotalMC  = MakeModelTTbarControlSample(workspace,"_ttbar_TotalMC"+label,mj_shape["signal_mc"],"_mj",channel,wtagger,label,constraintlist_MC ); 
  std::cout<< " ############################### Fail: MC ttbar Resonant ################################### "<<std::endl;
  RooAbsPdf* model_ttbar_TotalMC_fail  = MakeModelTTbarControlSample(workspace,"_ttbar_TotalMC"+label+"_failtau2tau1cut",mj_shape["signal_mc_fail"],"_mj",channel,wtagger,label,constraintlist_MC);                                                                                                                      
  std::cout<< " ############################### Fail: MC total PDF ################################### "<<std::endl;
  RooAddPdf* model_TotalMC_fail = new RooAddPdf(("model_TotalMC"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str(),("model_TotalMC"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str(),RooArgList(*model_ttbar_TotalMC_fail,*model_bkg_TotalMC_fail,*model_STop_fail,*model_VV_fail,*model_WJets_fail));
  std::cout<< " ############################### Pass: MC total PDF ################################### "<<std::endl;
  RooAddPdf* model_TotalMC = new RooAddPdf(("model_TotalMC"+label+"_"+channel).c_str(),("model_TotalMC"+label+"_"+channel).c_str(),RooArgList(*model_ttbar_TotalMC,*model_bkg_TotalMC,*model_STop,*model_VV,*model_WJets));
 
  workspace->import(*model_TotalMC_fail);
  workspace->import(*model_TotalMC);

}


///-------------------------------------------
void DrawScaleFactorTTbarControlSample(RooWorkspace* workspace, std::map<std::string,int> color_palet, const std::string & label, const std::string & channel, const std::string & wtagger,const double & ca8_ungroomed_pt_min, const double & ca8_ungroomed_pt_max, const std::string & sample){
  
  float ttSF = 0.667028210351;
  std::cout<< "Using tt scalefactor of " << ttSF << std::endl;

  RooRealVar* rrv_mass_j = workspace->var("rrv_mass_j");

  RooDataSet* rdataset_data_mj  = (RooDataSet*) workspace->data(("rdataset_data"+label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj = (RooDataSet*) workspace->data(("rdataset_TTbar"+label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_STop_mj  = (RooDataSet*) workspace->data(("rdataset_STop"+label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_VV_mj    = (RooDataSet*) workspace->data(("rdataset_VV"+label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_WJets_mj = (RooDataSet*) workspace->data(("rdataset_WJets0"+label+"_"+channel+"_mj").c_str());
  // RooDataSet* rdataset_QCD_mj   = (RooDataSet*) workspace->data(("rdataset_WJets0"+label+"_"+channel+"_mj").c_str());
  //Plot in merged and unmerged comp
  RooDataSet* rdataset_TTbar_mj_merged     = (RooDataSet*) workspace->data(("rdataset_TTbar_realW"+label+"_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_unmerged   = (RooDataSet*) workspace->data(("rdataset_TTbar_fakeW"+label+"_"+channel+"_mj").c_str());
  
  
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_STop_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_VV_mj);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_WJets_mj);
  // change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_QCD_mj);
  //Plot in merged and unmerged comp
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj_merged);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj_unmerged);
 
  RooAbsPdf* model_histpdf_STop  = workspace->pdf((std::string(rdataset_STop_mj->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_VV    = workspace->pdf((std::string(rdataset_VV_mj->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_WJets = workspace->pdf((std::string(rdataset_WJets_mj->GetName())+"_histpdf").c_str());
  // RooAbsPdf* model_histpdf_WJets = workspace->pdf((std::string(rdataset_QCD_mj->GetName())+"_histpdf").c_str());
  //Plot in merged and unmerged comp
  RooAbsPdf* model_histpdf_TTbar_merged    = workspace->pdf((std::string(rdataset_TTbar_mj_merged->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_TTbar_unmerged  = workspace->pdf((std::string(rdataset_TTbar_mj_unmerged->GetName())+"_histpdf").c_str());

  RooAbsPdf* model_TTbar_STop_VV_WJets = workspace->pdf(("model_TTbar_STop_VV_WJets"+label+"_"+channel).c_str());

  //dataset fail tau2tau1 cut
  RooDataSet* rdataset_data_mj_fail   = (RooDataSet*) workspace->data(("rdataset_data"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_fail  = (RooDataSet*) workspace->data(("rdataset_TTbar"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_STop_mj_fail   = (RooDataSet*) workspace->data(("rdataset_STop"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_VV_mj_fail     = (RooDataSet*) workspace->data(("rdataset_VV"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_WJets_mj_fail  = (RooDataSet*) workspace->data(("rdataset_WJets0"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  // RooDataSet* rdataset_QCD_mj_fail    = (RooDataSet*) workspace->data(("rdataset_QCD"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  //Plot in merged and unmerged comp
  RooDataSet* rdataset_TTbar_mj_fail_merged    = (RooDataSet*) workspace->data(("rdataset_TTbar_realW"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  RooDataSet* rdataset_TTbar_mj_fail_unmerged  = (RooDataSet*) workspace->data(("rdataset_TTbar_fakeW"+label+"_"+"failtau2tau1cut_"+channel+"_mj").c_str());
  
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_STop_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_VV_mj_fail);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_WJets_mj_fail);
  // change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_QCD_mj_fail);
  //Plot in merged and unmerged comp
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj_fail_merged);
  change_dataset_to_histpdf(workspace,rrv_mass_j, rdataset_TTbar_mj_fail_unmerged);

  RooAbsPdf* model_histpdf_STop_fail  = workspace->pdf((std::string(rdataset_STop_mj_fail->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_VV_fail    = workspace->pdf((std::string(rdataset_VV_mj_fail->GetName())+"_histpdf").c_str());
  RooAbsPdf* model_histpdf_WJets_fail = workspace->pdf((std::string(rdataset_WJets_mj_fail->GetName())+"_histpdf").c_str());
  // RooAbsPdf* model_histpdf_QCD_fail   = workspace->pdf((std::string(rdataset_QCD_mj_fail->GetName())+"_histpdf").c_str());
  //Plot in merged and unmerged comp
   RooAbsPdf* model_histpdf_TTbar_merged_fail   = workspace->pdf((std::string(rdataset_TTbar_mj_fail_merged->GetName())+"_histpdf").c_str());
   RooAbsPdf* model_histpdf_TTbar_unmerged_fail = workspace->pdf((std::string(rdataset_TTbar_mj_fail_unmerged->GetName())+"_histpdf").c_str());
  

  RooAbsPdf* model_TTbar_STop_VV_WJets_fail = workspace->pdf(("model_TTbar_STop_VV_WJets_fail"+label+"_"+channel).c_str());

  
  double scale_number_TTbar_STop_VV_WJets      = (rdataset_TTbar_mj_merged->sumEntries()*ttSF+rdataset_TTbar_mj_unmerged->sumEntries()*ttSF+rdataset_STop_mj->sumEntries()+rdataset_VV_mj->sumEntries() +rdataset_WJets_mj->sumEntries())/( rdataset_data_mj->sumEntries()+rdataset_data_mj_fail->sumEntries() );
  double scale_number_TTbar_STop_VV_WJets_fail = (rdataset_TTbar_mj_fail_merged->sumEntries()*ttSF+rdataset_TTbar_mj_fail_unmerged->sumEntries()*ttSF+rdataset_STop_mj_fail->sumEntries()+rdataset_VV_mj_fail->sumEntries() +rdataset_WJets_mj_fail->sumEntries())/( rdataset_data_mj->sumEntries()+rdataset_data_mj_fail->sumEntries() );

  
  std::cout<<"SCALE NR. PASS = " << scale_number_TTbar_STop_VV_WJets << std::endl;
  std::cout<<"SCALE NR. FAIL = " << scale_number_TTbar_STop_VV_WJets_fail << std::endl;
  RooAbsPdf* model_data_fail = workspace->pdf(("model_data"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str());
  RooAbsPdf* model_data = workspace->pdf(("model_data"+label+"_"+channel).c_str());

  RooCategory* category_p_f = workspace->cat(("category_p_f"+label+"_"+channel).c_str());

  RooSimultaneous* simPdf_data = new RooSimultaneous(("simPdf_data"+label+"_"+channel).c_str(),("simPdf_data"+label+"_"+channel).c_str(),*category_p_f);
  simPdf_data->addPdf(*model_data,"pass");
  simPdf_data->addPdf(*model_data_fail,"fail");
  RooDataSet* combData_p_f_data = (RooDataSet*) workspace->data(("combData_p_f_data"+label+"_"+channel).c_str());

  simPdf_data->Print();
  combData_p_f_data->Print();

  RooAbsPdf* model_TotalMC_fail = workspace->pdf(("model_TotalMC"+label+"_"+"failtau2tau1cut"+"_"+channel).c_str());
  RooAbsPdf* model_TotalMC      = workspace->pdf(("model_TotalMC"+label+"_"+channel).c_str());

  RooSimultaneous* simPdf_TotalMC = new RooSimultaneous(("simPdf_TotalMC"+label+"_"+channel).c_str(),("simPdf_TotalMC"+label+"_"+channel).c_str(),*category_p_f);
  simPdf_TotalMC->addPdf(*model_TotalMC,"pass");
  simPdf_TotalMC->addPdf(*model_TotalMC_fail,"fail");
  RooDataSet* combData_p_f_TotalMC = (RooDataSet*) workspace->data(("combData_p_f_TotalMC"+label+"_"+channel).c_str());

  RooPlot* xframe_data = rrv_mass_j->frame( RooFit::Bins(int(rrv_mass_j->getBins())));
  RooPlot* xframe_data_fail = rrv_mass_j->frame( RooFit::Bins(int(rrv_mass_j->getBins())));
  xframe_data->GetYaxis()->SetTitle(" Events / (5 GeV)");
  xframe_data_fail->GetYaxis()->SetTitle(" Events / (5 GeV)");
  xframe_data     ->GetYaxis()->SetTitleOffset(1.39);
  xframe_data_fail->GetYaxis()->SetTitleOffset(1.39);
  
  //#############################################################
  ///Plot total pass MC normalizing it to data
  //#############################################################
  

  TString cut ;
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::pass",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data,RooFit::Name("data_invisible"),RooFit::Cut(cut.Data()),RooFit::MarkerSize(1.5), RooFit::DataError(RooAbsData::SumW2), RooFit::XErrorSize(0) );
  
  std::cout<< "PLOTTING SIM. FIT! PASS: " << std::endl;

  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar"), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s,%s,%s,%s,%s",model_histpdf_TTbar_merged->GetName(),model_histpdf_TTbar_unmerged->GetName(),model_histpdf_STop->GetName(),model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_realW"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar_realW"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s,%s,%s,%s",model_histpdf_TTbar_unmerged->GetName(),model_histpdf_STop->GetName(),model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_fakeW"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar_fakeW"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  
  
  cut.Form("%s,%s,%s",model_histpdf_STop->GetName(),model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("STop"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["STop"]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s,%s",model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("VV"),RooFit::Components(cut.Data()), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["VV"]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s",model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("WJets"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["WJets"]), RooFit::LineColor(kBlack), RooFit::VLines());

  // //pass plots
//   model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_line_invisible"), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
//   cut.Form("%s,%s,%s",model_histpdf_STop->GetName(), model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
//   model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("STop_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
//   cut.Form("%s,%s",model_histpdf_VV->GetName(),model_histpdf_WJets->GetName());
//   model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("VV_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
//   cut.Form("%s",model_histpdf_WJets->GetName());
//   model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("WJets_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());


  //pass plots
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_line_invisible"), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s,%s,%s,%s",model_histpdf_TTbar_merged->GetName(),model_histpdf_TTbar_unmerged->GetName(),model_histpdf_STop->GetName(), model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_realW_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s,%s,%s",model_histpdf_TTbar_unmerged->GetName(),model_histpdf_STop->GetName(), model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("TTbar_fakeW_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s,%s",model_histpdf_STop->GetName(), model_histpdf_VV->GetName(), model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("STop_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s",model_histpdf_VV->GetName(),model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("VV_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s",model_histpdf_WJets->GetName());
  model_TTbar_STop_VV_WJets->plotOn(xframe_data,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets),RooFit::Name("WJets_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  
 
 
  // plot data again
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::pass",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data,RooFit::Name("data"), RooFit::Cut(cut.Data()), RooFit::MarkerSize(1.5), RooFit::DataError(RooAbsData::Poisson), RooFit::XErrorSize(0));

  // plot mc fit function
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::pass",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  simPdf_TotalMC->plotOn(xframe_data,RooFit::Name("MC fit"),RooFit::Slice(*category_p_f,"pass"), RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_TotalMC),RooFit::NormRange("controlsample_fitting_range"), RooFit::LineStyle(kSolid), RooFit::LineColor(kRed));
  cut.Form("model_bkg_TotalMC_%s_mj,model_STop_%s_mj,model_VV_%s_mj,model_WJets0_%s_mj",channel.c_str(),channel.c_str(),channel.c_str(),channel.c_str());
  simPdf_TotalMC->plotOn(xframe_data,RooFit::Name("mc fit bkg_invisible"),RooFit::Slice(*category_p_f,"pass"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_TotalMC),RooFit::NormRange("controlsample_fitting_range"), RooFit::Components(cut.Data()), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));

  // plot data fit function
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::pass",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data,RooFit::Name("data_invisible"),RooFit::Cut(cut.Data()),RooFit::MarkerSize(1.5),RooFit::DataError(RooAbsData::SumW2),RooFit::XErrorSize(0));

  simPdf_data->plotOn(xframe_data,RooFit::Name("Data fit"),RooFit::Slice(*category_p_f,"pass"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_data),RooFit::NormRange("controlsample_fitting_range"), RooFit::LineStyle(kSolid), RooFit::LineColor(kBlue));
  cut.Form("model_bkg_data_%s_mj,model_STop_%s_mj,model_VV_%s_mj,model_WJets0_%s_mj",channel.c_str(),channel.c_str(),channel.c_str(),channel.c_str());
  simPdf_data->plotOn(xframe_data,RooFit::Name("dat fit bkg_invisible"),RooFit::Slice(*category_p_f,"pass"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_data),RooFit::NormRange("controlsample_fitting_range"), RooFit::Components(cut.Data()), RooFit::LineStyle(kDashed), RooFit::LineColor(kBlue));



  //#############################################################
  ///Plot total fail MC normalizing it to data
  //#############################################################
  std::cout<< "PLOTTING SIM. FIT! FAIL: " << std::endl;
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::fail",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data_fail,RooFit::Name("data_invisible"), RooFit::Cut(cut.Data()), RooFit::MarkerSize(1.5), RooFit::DataError(RooAbsData::SumW2),RooFit::XErrorSize(0));

  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar"), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  
  cut.Form("%s,%s,%s,%s,%s",model_histpdf_TTbar_merged_fail->GetName(),model_histpdf_TTbar_unmerged_fail->GetName(),model_histpdf_STop_fail->GetName(),model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar_realW"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar_realW"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s,%s,%s,%s",model_histpdf_TTbar_unmerged_fail->GetName(),model_histpdf_STop_fail->GetName(),model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar_fakeW"),RooFit::Components(cut.Data()),RooFit::DrawOption("F"), RooFit::FillColor(color_palet["TTbar_fakeW"+label]), RooFit::LineColor(kBlack), RooFit::VLines());
  
  
  cut.Form("%s,%s,%s",model_histpdf_STop_fail->GetName(),model_histpdf_VV_fail->GetName(),model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("STop"),RooFit::Components(cut.Data()), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["STop"]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s,%s",model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("VV"),RooFit::Components(cut.Data()), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["VV"]), RooFit::LineColor(kBlack), RooFit::VLines());
  cut.Form("%s", model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("WJets"),RooFit::Components(cut.Data()), RooFit::DrawOption("F"), RooFit::FillColor(color_palet["WJets"]), RooFit::LineColor(kBlack), RooFit::VLines());

  //solid line
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar_line_invisible"), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s,%s,%s,%s",model_histpdf_TTbar_merged_fail->GetName(),model_histpdf_TTbar_unmerged_fail->GetName(),model_histpdf_STop_fail->GetName(), model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar_realW_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s,%s,%s",model_histpdf_TTbar_unmerged_fail->GetName(),model_histpdf_STop_fail->GetName(), model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("TTbar_fakeW_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
 
 
  cut.Form("%s,%s,%s",model_histpdf_STop_fail->GetName(), model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("STop_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s,%s",model_histpdf_VV_fail->GetName(), model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("VV_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());
  cut.Form("%s", model_histpdf_WJets_fail->GetName());
  model_TTbar_STop_VV_WJets_fail->plotOn(xframe_data_fail,RooFit::Normalization(scale_number_TTbar_STop_VV_WJets_fail),RooFit::Name("WJets_line_invisible"),RooFit::Components(cut.Data()), RooFit::LineColor(kBlack), RooFit::LineWidth(2), RooFit::VLines());


  //fail plots -> plot MC fit
  
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::fail",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data_fail,RooFit::Name("data_invisible"),RooFit::Cut(cut.Data()),RooFit::MarkerSize(1.5),RooFit::DataError(RooAbsData::SumW2),RooFit::XErrorSize(0));
  simPdf_TotalMC->plotOn(xframe_data_fail,RooFit::Name("MC fit")    ,RooFit::Slice(*category_p_f,"fail"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_TotalMC),RooFit::NormRange("controlsample_fitting_range"), RooFit::LineStyle(kSolid), RooFit::LineColor(kRed));
  cut.Form("model_bkg_TotalMC_failtau2tau1cut_%s_mj,model_STop_failtau2tau1cut_%s_mj,model_VV_failtau2tau1cut_%s_mj,model_WJets0_failtau2tau1cut_%s_mj",channel.c_str(),channel.c_str(),channel.c_str(),channel.c_str());
  simPdf_TotalMC->plotOn(xframe_data_fail,RooFit::Name("MC fit bkg"),RooFit::Slice(*category_p_f,"fail"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_TotalMC),RooFit::NormRange("controlsample_fitting_range"), RooFit::Components(cut.Data()), RooFit::LineColor(kRed), RooFit::LineStyle(kDashed));

  //fail plots -> plot data fit
  cut.Form("category_p_f%s_%s==category_p_f%s_%s::fail",label.c_str(),channel.c_str(),label.c_str(),channel.c_str());
  combData_p_f_data->plotOn(xframe_data_fail,RooFit::Name("data_invisible"),RooFit::Cut(cut.Data()),RooFit::MarkerSize(1.5),RooFit::DataError(RooAbsData::SumW2),RooFit::XErrorSize(0));

  simPdf_data->plotOn(xframe_data_fail,RooFit::Name("Data fit"),RooFit::Slice(*category_p_f,"fail"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_data),RooFit::NormRange("controlsample_fitting_range"),RooFit::LineStyle(kSolid), RooFit::LineColor(kBlue));
  simPdf_data->plotOn(xframe_data_fail,RooFit::Name("data_fit_invisible"),RooFit::Slice(*category_p_f,"fail"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_data),RooFit::NormRange("controlsample_fitting_range"));
  cut.Form("model_bkg_data_failtau2tau1cut_%s_mj,model_STop_failtau2tau1cut_%s_mj,model_VV_failtau2tau1cut_%s_mj,model_WJets0_failtau2tau1cut_%s_mj",channel.c_str(),channel.c_str(),channel.c_str(),channel.c_str());
  simPdf_data->plotOn(xframe_data_fail,RooFit::Name("data fit bkg"),RooFit::Slice(*category_p_f,"fail"),RooFit::ProjWData(RooArgSet(*category_p_f),*combData_p_f_data),RooFit::NormRange("controlsample_fitting_range"), RooFit::Components(cut.Data()),RooFit::LineStyle(kDashed), RooFit::LineColor(kBlue));

  // //signal window
//   TLine* lowerLine = new TLine(65,0.,65,xframe_data->GetMaximum()*0.7); lowerLine->SetLineWidth(2); lowerLine->SetLineColor(kBlack); lowerLine->SetLineStyle(9);
//   TLine* upperLine = new TLine(105,0.,105,xframe_data->GetMaximum()*0.7); upperLine->SetLineWidth(2); upperLine->SetLineColor(kBlack); upperLine->SetLineStyle(9);
//   xframe_data->addObject(lowerLine); xframe_data->addObject(upperLine);
//   lowerLine = new TLine(65,0.,65,xframe_data_fail->GetMaximum()*0.7); lowerLine->SetLineWidth(2); lowerLine->SetLineColor(kBlack); lowerLine->SetLineStyle(9);
//   upperLine = new TLine(105,0.,105,xframe_data_fail->GetMaximum()*0.7); upperLine->SetLineWidth(2); upperLine->SetLineColor(kBlack); upperLine->SetLineStyle(9);
//   xframe_data_fail->addObject(lowerLine); xframe_data_fail->addObject(upperLine);

  // TLegend* leg_data = legend4Plot(xframe_data,0,-0.2,0.07,0.04,0.,1,channel);
  // xframe_data->addObject(leg_data);
  // TLegend* leg_data_fail = legend4Plot(xframe_data_fail,0,-0.2,0.07,0.04,0.,1,channel);
  // xframe_data_fail->addObject(leg_data);
  
 
  // TLegend*  theLeg = new TLegend(0.1564883,0.7073171,0.9205686,0.8968293,"","NDC");
  TLegend*  theLeg = new TLegend(0.3885213,0.6640827,0.9774937,0.8992248,"","NDC");
  theLeg->SetName("theLegend");
  // theLeg->SetNColumns(3);
  theLeg->SetNColumns(2);
  theLeg->SetFillColor(0);
  theLeg->SetFillStyle(0);
  theLeg->SetBorderSize(0);
  theLeg->SetLineColor(0);
  theLeg->SetLineWidth(0);
  theLeg->SetLineStyle(0);
  theLeg->SetTextSize(0.039);
  theLeg->SetTextFont(42);
  // theLeg->AddEntry(xframe_data->findObject("data")       ,"CMS data"            ,"PLE");
 //  theLeg->AddEntry(xframe_data->findObject("TTbar_realW"),"t#bar{t} (merged)"   ,"F");
 //  theLeg->AddEntry(xframe_data->findObject("WJets")      ,"W+jets"              ,"F");
 //  theLeg->AddEntry(xframe_data->findObject("Data fit")   ,"Data fit"            ,"L");
 //  theLeg->AddEntry(xframe_data->findObject("TTbar_fakeW"),"t#bar{t} (unmerged)" ,"F");
 //  theLeg->AddEntry(xframe_data->findObject("VV")         ,"WW/WZ/ZZ"            ,"F");
 //  theLeg->AddEntry(xframe_data->findObject("MC fit")     ,"MC fit"              ,"L");
 //  theLeg->AddEntry(xframe_data->findObject("STop")       ,"Single top"          ,"F");
  theLeg->AddEntry(xframe_data->findObject("data")       ,"CMS data"            ,"PLE"); 
  theLeg->AddEntry(xframe_data->findObject("TTbar_fakeW"),"t#bar{t} (unmerged)" ,"F");
  theLeg->AddEntry(xframe_data->findObject("Data fit")   ,"Data fit"            ,"L");
  theLeg->AddEntry(xframe_data->findObject("STop")       ,"Single top"          ,"F");
  theLeg->AddEntry(xframe_data->findObject("MC fit")     ,"MC fit"              ,"L");
  theLeg->AddEntry(xframe_data->findObject("WJets")      ,"W+jets"              ,"F");
  theLeg->AddEntry(xframe_data->findObject("TTbar_realW"),"t#bar{t} (merged)"   ,"F"); 
  theLeg->AddEntry(xframe_data->findObject("VV")         ,"WW/WZ/ZZ"            ,"F");
  
  xframe_data->addObject(theLeg);
  xframe_data_fail->addObject(theLeg);


  std::string tmp_channel = "el";
  if(workspace->var(("rrv_mean1_gaus_ttbar_data"+label+"_"+"el").c_str())) tmp_channel = "el";
  else tmp_channel = "mu";

  RooRealVar* rrv_mean_gaus_data     = workspace->var(("rrv_mean1_gaus_ttbar_data"+label+"_"+tmp_channel).c_str());
  RooRealVar* rrv_sigma_gaus_data    = workspace->var(("rrv_sigma1_gaus_ttbar_data"+label+"_"+tmp_channel).c_str());
  RooRealVar* rrv_mean_gaus_TotalMC  = workspace->var(("rrv_mean1_gaus_ttbar_TotalMC"+label+"_"+tmp_channel).c_str());
  RooRealVar* rrv_sigma_gaus_TotalMC = workspace->var(("rrv_sigma1_gaus_ttbar_TotalMC"+label+"_"+tmp_channel).c_str());

  if(rrv_mean_gaus_TotalMC){
    TString latex ; latex.Form("Mean_{MC } = %3.1f #pm %2.1f",rrv_mean_gaus_TotalMC->getVal(),rrv_mean_gaus_TotalMC->getError());
    TLatex* tl_MC_mean  = new TLatex(0.25 ,0.62,latex.Data());
    latex.Form("Sigma_{MC }= %2.1f #pm %2.1f",rrv_sigma_gaus_TotalMC->getVal(),rrv_sigma_gaus_TotalMC->getError());
    TLatex* tl_MC_sigma = new TLatex(0.25 ,0.57,latex.Data());
    tl_MC_mean->SetNDC(); tl_MC_sigma->SetNDC();
    tl_MC_mean->SetTextSize(0.03);
    tl_MC_sigma->SetTextSize(0.03);
    // xframe_data.addObject(tl_MC_mean);
    // xframe_data.addObject(tl_MC_sigma);

  }
  if(rrv_mean_gaus_data){
    TString latex ; latex.Form("Mean_{data} = %3.1f #pm %2.1f",rrv_mean_gaus_data->getVal(),rrv_mean_gaus_data->getError());
    TLatex* tl_data_mean  = new TLatex(0.25 ,0.62,latex.Data());
    latex.Form("Sigma_{data}= %2.1f #pm %2.1f",rrv_sigma_gaus_data->getVal(),rrv_sigma_gaus_data->getError());
    TLatex* tl_data_sigma = new TLatex(0.25 ,0.57,latex.Data());
    tl_data_mean->SetNDC(); tl_data_sigma->SetNDC();
    tl_data_mean->SetTextSize(0.03);
    tl_data_sigma->SetTextSize(0.03);
    // xframe_data.addObject(tl_data_mean); xframe_data.addObject(tl_data_sigma);
  }

  xframe_data->GetYaxis()->SetRangeUser(1e-2,xframe_data->GetMaximum()*1.4);
  xframe_data_fail->GetYaxis()->SetRangeUser(1e-2,xframe_data_fail->GetMaximum()*1.4);

  TString nameDir;  nameDir.Form("plots/%s_totalFit/",wtagger.c_str());
  TString namePlot; namePlot.Form("TotalFit_%s_%s",label.c_str(),wtagger.c_str());
  draw_canvas(xframe_data,std::string(nameDir),std::string(namePlot),channel,GetLumi(),0,1,0);
  namePlot.Form("TotalFit_%s_%s_fail",label.c_str(),wtagger.c_str());
  draw_canvas(xframe_data_fail,std::string(nameDir),std::string(namePlot),channel,GetLumi(),0,1,0);

}
