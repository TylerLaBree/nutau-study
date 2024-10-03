#include <iostream>
#include <string>
#include <vector>
#include <dirent.h> // Directory handling

// Gallery and nutools includes to read art files
#include "gallery/Event.h"
#include "gallery/Handle.h"
#include "nusimdata/SimulationBase/MCTruth.h"

// ROOT includes
#include "TFile.h"
#include "TTree.h"

// Helper function to gather .root file paths from a directory
std::vector<std::string> gatherRootFilePaths(const std::string& directoryPath) {
    std::vector<std::string> rootFilePaths;
    DIR* dir;
    struct dirent* entry;
    if ((dir = opendir(directoryPath.c_str())) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            std::string fileName(entry->d_name);
            if (fileName.find(".root", (fileName.length() - 5)) != std::string::npos) {
                rootFilePaths.push_back(directoryPath + fileName);
            }
        }
        closedir(dir);
    } else {
        std::cerr << "Could not open directory: " << directoryPath << std::endl;
    }
    return rootFilePaths;
}

// Identify decay products based on mother particle being a tau
std::vector<int> identifyDecayProducts(const simb::MCTruth& truth) {
    std::vector<int> products;
    const int tauPDG = 15;  // PDG code for Tau
    for (int i = 0; i < truth.NParticles(); ++i) {
        const simb::MCParticle& particle = truth.GetParticle(i);
        if (particle.Mother() != -1 && truth.GetParticle(particle.Mother()).PdgCode() == tauPDG) {
            products.push_back(particle.PdgCode());
        }
    }
    return products;
}

// Define tau decay modes based on PDG codes
int identifyTauDecayMode(const std::vector<int>& decayProducts) {
    std::map<std::vector<int>, int> decayModeMap = {
        {{-11, 12, 16}, 0},  // e- nu(e)~ nu(tau)
        {{-13, 14, 16}, 1},  // mu- nu(mu)~ nu(tau)
        {{-211, 111, 16}, 2},  // pi- pi0 nu(tau)
        {{-211, 16}, 3},     // pi- nu(tau)
        {{-211, 111, 111, 16}, 4},  // pi- pi0 pi0 nu(tau)
        {{-211, -211, 211, 16}, 5},  // pi- pi- pi+ nu(tau)
        {{-211, -211, 211, 111, 16}, 6}  // pi- pi- pi+ pi0 nu(tau)
    };

    for (const auto& mode : decayModeMap) {
        if (std::is_permutation(decayProducts.begin(), decayProducts.end(), mode.first.begin(), mode.first.end())) {
            return mode.second;
        }
    }
    return -1;  // Unknown decay
}

// Classify interaction type based on scattering and nuance codes
int identifyInteractionType(const simb::MCTruth& truth) {
    int interactionType = truth.GetNeutrino().InteractionType();
    int mode = truth.GetNeutrino().Mode();

    if (mode == 0) return 0; // QE
    else if (mode == 1) return 1; // Res
    else if (mode == 2) return 2; // DIS
    else if (mode == 3 || mode == 4) return 3; // Coh
    else if (mode == 10) return 4; // MEC
    return 5; // Other interactions
}

void convertArtToRoot(const std::string& inputDirectory, const std::string& outputFilePath) {
    std::vector<std::string> fileNames = gatherRootFilePaths(inputDirectory);

    if (fileNames.empty()) {
        std::cerr << "No .root files found in the specified directory: " << inputDirectory << std::endl;
        return;
    }

    gallery::Event events(fileNames);
    TFile* outputFile = new TFile(outputFilePath.c_str(), "RECREATE");
    TTree* dataTree = new TTree("NeutrinoData", "Data from ART files");

    int eventIndex;
    int initialNeutrinoFlavor;
    int isChargedCurrent;  // Now int: 0 for NC, 1 for CC
    int tauDecayMode;
    int interactionType;
    int particleCount;
    std::vector<int> particlePdgCodes;
    std::vector<int> particleStatusCodes;
    std::vector<double> particleMomentumX;
    std::vector<double> particleMomentumY;
    std::vector<double> particleMomentumZ;
    std::vector<double> particleEnergies;
    std::vector<double> particleMothers;

    dataTree->Branch("EventIndex", &eventIndex);
    dataTree->Branch("InitialNeutrinoFlavor", &initialNeutrinoFlavor);
    dataTree->Branch("IsChargedCurrent", &isChargedCurrent);
    dataTree->Branch("TauDecayMode", &tauDecayMode);
    dataTree->Branch("InteractionType", &interactionType);
    dataTree->Branch("ParticleCount", &particleCount);
    dataTree->Branch("ParticlePdgCodes", &particlePdgCodes);
    dataTree->Branch("ParticleStatusCodes", &particleStatusCodes);
    dataTree->Branch("ParticleMomentumX", &particleMomentumX);
    dataTree->Branch("ParticleMomentumY", &particleMomentumY);
    dataTree->Branch("ParticleMomentumZ", &particleMomentumZ);
    dataTree->Branch("ParticleEnergies", &particleEnergies);
    dataTree->Branch("ParticleMothers", &particleMothers);

    while (!events.atEnd()) {
        particlePdgCodes.clear();
        particleStatusCodes.clear();
        particleMomentumX.clear();
        particleMomentumY.clear();
        particleMomentumZ.clear();
        particleEnergies.clear();
        particleMothers.clear();

        gallery::Handle<std::vector<simb::MCTruth>> mcTruthHandle;
        events.getByLabel("generator", mcTruthHandle);

        if (mcTruthHandle.isValid()) {
            for (const auto& truth : *mcTruthHandle) {
                eventIndex = events.eventAuxiliary().event();
                initialNeutrinoFlavor = truth.GetNeutrino().Nu().PdgCode();
                isChargedCurrent = !truth.GetNeutrino().CCNC();
                std::vector<int> decayProducts = identifyDecayProducts(truth);
                tauDecayMode = identifyTauDecayMode(decayProducts);
                interactionType = identifyInteractionType(truth);
                particleCount = truth.NParticles();

                for (int i = 0; i < particleCount; i++) {
                    const simb::MCParticle& particle = truth.GetParticle(i);
                    particlePdgCodes.push_back(particle.PdgCode());
                    particleStatusCodes.push_back(particle.StatusCode());
                    particleMomentumX.push_back(particle.Momentum().Px());
                    particleMomentumY.push_back(particle.Momentum().Py());
                    particleMomentumZ.push_back(particle.Momentum().Pz());
                    particleEnergies.push_back(particle.Momentum().E());
                    particleMothers.push_back(particle.Mother());
                }

                dataTree->Fill();
            }
        }
        events.next(); // Move to the next event
    }
    dataTree->Write();
    outputFile->Close();
    delete outputFile;
}
