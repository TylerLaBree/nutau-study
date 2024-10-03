#include <TFile.h>
#include <TH1D.h>
#include <iostream>

void makeWeights() {
    // File paths
    const char* inputFilePath = "../nutau-data/weight/flux_dune_neutrino_FD.root";
    const char* outputPath = "../nutau-data/weight/weights_flux.root";

    // Open the ROOT file
    TFile* inputFile = TFile::Open(inputFilePath, "READ");
    if (!inputFile) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }

    // Create a new ROOT file to store the ratio histograms
    TFile* outputFile = new TFile(outputPath, "RECREATE");

    // Retrieve histograms
    TH1D* hNumuFlux = (TH1D*)inputFile->Get("numu_flux");
    TH1D* hNumuFluxOsc = (TH1D*)inputFile->Get("numu_fluxosc");
    TH1D* hNueFluxOsc = (TH1D*)inputFile->Get("nue_fluxosc");
    TH1D* hNutauFluxOsc = (TH1D*)inputFile->Get("nutau_fluxosc");

    if (!hNumuFlux || !hNumuFluxOsc || !hNueFluxOsc || !hNutauFluxOsc) {
        std::cerr << "Error retrieving histograms from input file!" << std::endl;
        inputFile->Close();
        return;
    }

    // Create ratio histograms
    TH1D* hNumuRatio = (TH1D*)hNumuFluxOsc->Clone("numu");
    hNumuRatio->Divide(hNumuFlux);

    TH1D* hNueRatio = (TH1D*)hNueFluxOsc->Clone("nue");
    hNueRatio->Divide(hNumuFlux);

    TH1D* hNutauRatio = (TH1D*)hNutauFluxOsc->Clone("nutau");
    hNutauRatio->Divide(hNumuFlux);

    // Create two copies of each histogram with _nc and _cc appended
    TH1D* hNumuRatio_nc = (TH1D*)hNumuRatio->Clone("numu_nc");
    TH1D* hNumuRatio_cc = (TH1D*)hNumuRatio->Clone("numu_cc");

    TH1D* hNueRatio_nc = (TH1D*)hNueRatio->Clone("nue_nc");
    TH1D* hNueRatio_cc = (TH1D*)hNueRatio->Clone("nue_cc");

    TH1D* hNutauRatio_nc = (TH1D*)hNutauRatio->Clone("nutau_nc");
    TH1D* hNutauRatio_cc = (TH1D*)hNutauRatio->Clone("nutau_cc");

    // Write the ratio histograms to output file
    outputFile->cd();
    hNumuRatio_nc->Write();
    hNumuRatio_cc->Write();

    hNueRatio_nc->Write();
    hNueRatio_cc->Write();

    hNutauRatio_nc->Write();
    hNutauRatio_cc->Write();

    // Close the files
    outputFile->Close();
    inputFile->Close();
}

