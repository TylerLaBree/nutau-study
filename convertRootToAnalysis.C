#include <cmath>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <stdexcept>
#include <TFile.h>
#include <TTree.h>
#include <TChain.h>
#include <TBranch.h>
#include <TH1D.h>

const double beamlineX = 0.0;
const double beamlineY = 0.10082778355435233;
const double beamlineZ = 0.9949038938829804;

struct Vector3D {
    double x, y, z;

    Vector3D(double x = 0.0, double y = 0.0, double z = 0.0) : x(x), y(y), z(z) {}

    double magnitude() const {
        return sqrt(x * x + y * y + z * z);
    }

    Vector3D normalized() const {
        double mag = magnitude();
        if (mag == 0) throw std::runtime_error("Attempt to normalize a zero-length vector.");
        return Vector3D(x / mag, y / mag, z / mag);
    }

    Vector3D cross(const Vector3D& other) const {
        return Vector3D(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    double dot(const Vector3D& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    double longitudinal(const Vector3D& unitVector) const {
        return std::abs(this->dot(unitVector));
    }

    double transverse(const Vector3D& unitVector) const {
        Vector3D parallelComponent = unitVector * this->dot(unitVector);
        Vector3D transverse = *this - parallelComponent;
        return transverse.magnitude();
    }

    double azimuth(const Vector3D& unitVector, const Vector3D& referenceAxis) const {
        Vector3D parallelComponentThis = unitVector * this->dot(unitVector);
        Vector3D transverseThis = *this - parallelComponentThis;

        Vector3D parallelComponentRef = unitVector * referenceAxis.dot(unitVector);
        Vector3D transverseRef = referenceAxis - parallelComponentRef;

        transverseThis = transverseThis.normalized();
        transverseRef = transverseRef.normalized();

        // Calculate the azimuth angle using atan2
        return atan2(transverseThis.dot(transverseRef.cross(unitVector)), transverseThis.dot(transverseRef));
    }

    Vector3D operator*(double scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    Vector3D operator-(const Vector3D& rhs) const {
        return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
    }

    Vector3D operator+(const Vector3D& rhs) const {
        return Vector3D(x + rhs.x, y + rhs.y, z + rhs.z);
    }
};

struct UnitVector : public Vector3D {
    UnitVector(double x, double y, double z) : Vector3D(x, y, z) {
        double mag = magnitude();
        if (mag == 0) throw std::runtime_error("Attempt to create a unit vector from a zero-length vector.");
        this->x /= mag;
        this->y /= mag;
        this->z /= mag;
    }
};

bool isNuclearCode(int pdgCode) {
    std::string codeStr = std::to_string(pdgCode);
    return codeStr.length() == 10;
}

bool isFinalState(int statusCode) {
    return (statusCode == 1 || statusCode == 15);
}

bool isVisibleAndSufficientEnergy(int pdgCode, int statusCode, double energy) {
    if (statusCode != 1 && statusCode != 15) return false;
    if (pdgCode == -12 || pdgCode == -14 || pdgCode == -16 || pdgCode == 12 || pdgCode == 14 || pdgCode == 16 || pdgCode == 2112 || isNuclearCode(pdgCode)) return false;

    switch (pdgCode) {
        case 211:
        case -211:
            return energy > 0.1; // Pions
        case 2212:
            return energy > 0.05; // Protons
        case 22:
        case 11:
        case -11:
        case 13:
        case -13:
            return energy > 0.03; // Photons, Electrons, Muons
        default:
            return true;
    }
}

// Helper function to map flavor and cc to histogram name
std::string getHistogramName(int flavor, int cc) {
    std::map<int, std::string> flavorMap = {{12, "nue"}, {14, "numu"}, {16, "nutau"}};
    std::string histName = flavorMap[flavor];
    histName += cc ? "_cc" : "_nc";
    return histName;
}

void resetCounts(int &leptonCount, int &negPionCount, int &chargedPionCount, double &leadingPionEnergy, double &otherParticleEnergySum, double &missingTransverseMomentum, double &weight) {
    leptonCount = 0;
    negPionCount = 0;
    chargedPionCount = 0;
    leadingPionEnergy = 0.0;
    otherParticleEnergySum = 0.0;
    missingTransverseMomentum = 0.0;
    weight = 0.0;
}

void convertRootToAnalysis(const char* inputFileName, const char* outputFileName) {
    // Open the weights file
    TFile* weightsFile = TFile::Open("../nutau-data/weight/weights_flux.root", "READ");
    if (!weightsFile || weightsFile->IsZombie()) {
        std::cerr << "Error opening weights file!" << std::endl;
        return;
    }

    // Open the input file
    TFile *inputFile = new TFile(inputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        std::cerr << "Error opening input file!" << std::endl;
        return;
    }

    // Get the tree from the input file
    TTree *inputTree = (TTree*)inputFile->Get("NeutrinoData");
    if (!inputTree) {
        std::cerr << "No tree found in input file!" << std::endl;
        inputFile->Close();
        return;
    }

    // Read the first entry to determine the histogram to use
    int initialNeutrinoFlavor, currentType;
    inputTree->SetBranchAddress("InitialNeutrinoFlavor", &initialNeutrinoFlavor);
    inputTree->SetBranchAddress("CurrentType", &currentType);
    inputTree->GetEntry(0);
    cout << "flavor: " << initialNeutrinoFlavor << " isCC: " << currentType << endl;
    std::string histogramName = getHistogramName(initialNeutrinoFlavor, currentType);
    TH1D* weightHist = (TH1D*)weightsFile->Get(histogramName.c_str());
    if (!weightHist) {
        std::cerr << "Histogram " << histogramName << " not found!" << std::endl;
        inputFile->Close();
        weightsFile->Close();
        return;
    }

    // Open the output file
    TFile *outputFile = new TFile(outputFileName, "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        std::cerr << "Error opening output file!" << std::endl;
        inputFile->Close();
        weightsFile->Close();
        return;
    }

    // Clone the tree structure from input file
    TTree *outputTree = inputTree->CloneTree(0); // Clone with no entries to copy the structure

    // Setup branches to read from input tree
    int eventIndex, particleCount;
    std::vector<int>* particlePdgCodes = nullptr;
    std::vector<int>* particleStatusCodes = nullptr;
    std::vector<double>* particleMomentumX = nullptr;
    std::vector<double>* particleMomentumY = nullptr;
    std::vector<double>* particleMomentumZ = nullptr;
    std::vector<double>* particleEnergies = nullptr;

    inputTree->SetBranchAddress("EventIndex", &eventIndex);
    inputTree->SetBranchAddress("ParticleCount", &particleCount);
    inputTree->SetBranchAddress("ParticlePdgCodes", &particlePdgCodes);
    inputTree->SetBranchAddress("ParticleStatusCodes", &particleStatusCodes);
    inputTree->SetBranchAddress("ParticleMomentumX", &particleMomentumX);
    inputTree->SetBranchAddress("ParticleMomentumY", &particleMomentumY);
    inputTree->SetBranchAddress("ParticleMomentumZ", &particleMomentumZ);
    inputTree->SetBranchAddress("ParticleEnergies", &particleEnergies);

    // Define variables to be calculated
    int isVisible, leptonCount, negPionCount, chargedPionCount;
    double leadingPionEnergy, otherParticleEnergySum, missingTransverseMomentum, weight;

    outputTree->Branch("isVisible", &isVisible, "isVisible/I");
    outputTree->Branch("leptonCount", &leptonCount, "leptonCount/I");
    outputTree->Branch("negPionCount", &negPionCount, "negPionCount/I");
    outputTree->Branch("chargedPionCount", &chargedPionCount, "chargedPionCount/I");
    outputTree->Branch("leadingPionEnergy", &leadingPionEnergy, "leadingPionEnergy/D");
    outputTree->Branch("otherParticleEnergySum", &otherParticleEnergySum, "otherParticleEnergySum/D");
    outputTree->Branch("missingTransverseMomentum", &missingTransverseMomentum, "missingTransverseMomentum/D");
    outputTree->Branch("weight", &weight, "weight/D");

    // Prepare vector and unit vector definitions for calculations
    UnitVector beamlineDir(beamlineX, beamlineY, beamlineZ);
    
    Long64_t nentries = inputTree->GetEntries();
    for (Long64_t i = 0; i < nentries; i++) {
        inputTree->GetEntry(i);

        resetCounts(leptonCount, negPionCount, chargedPionCount, leadingPionEnergy, otherParticleEnergySum, missingTransverseMomentum, weight);
        double initialEnergy = (*particleEnergies)[0]; // Assuming the first particle's energy is the neutrino's
        int bin = weightHist->FindBin(initialEnergy);
        weight = weightHist->GetBinContent(bin);
        
        Vector3D missingMomentum;
        for (size_t j = 0; j < particleCount; j++) {
            isVisible = isVisibleAndSufficientEnergy((*particlePdgCodes)[j], (*particleStatusCodes)[j], (*particleEnergies)[j]) ? 1 : 0;
            Vector3D particleMomentum((*particleMomentumX)[j], (*particleMomentumY)[j], (*particleMomentumZ)[j]);
            
            if (isVisible) {
                if ((*particlePdgCodes)[j] == 11 || (*particlePdgCodes)[j] == -11 || (*particlePdgCodes)[j] == 13 || (*particlePdgCodes)[j] == -13) {
                    leptonCount++;
                }
                if ((*particlePdgCodes)[j] == -211) {
                    negPionCount++;
                }
                if ((*particlePdgCodes)[j] == 211 || (*particlePdgCodes)[j] == -211) {
                    chargedPionCount++;
                }
                if ((*particlePdgCodes)[j] == -211 && (*particleEnergies)[j] > leadingPionEnergy) {
                    otherParticleEnergySum += leadingPionEnergy; // Add the previous leading to sum
                    leadingPionEnergy = (*particleEnergies)[j]; // Update leading energy
                } else {
                    otherParticleEnergySum += (*particleEnergies)[j];
                }
                missingMomentum = missingMomentum - particleMomentum;
            }
        }
        missingTransverseMomentum = missingMomentum.transverse(beamlineDir);

        outputTree->Fill();
    }

    // Write and close files
    outputTree->Write();
    outputFile->Close();
    inputFile->Close();
    weightsFile->Close();
}

