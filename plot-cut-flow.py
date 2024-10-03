import uproot
import numpy as np
import matplotlib.pyplot as plt

# File paths
signal_file = '../nutau-data/machado/new/nu_16_cc.root'

background_files = [
    '../nutau-data/machado/new/nu_12_nc.root',
    '../nutau-data/machado/new/nu_14_nc.root',
    '../nutau-data/machado/new/nu_16_nc.root'
]

# Branches to read
branches = [
    'leptonCount',
    'negPionCount',
    'leadingPionEnergy',
    'otherParticleEnergySum',
    'missingTransverseMomentum',
    'weight'
]

# Read signal data
with uproot.open(signal_file) as file:
    NeutrinoData = file['NeutrinoData']
    signal_data = NeutrinoData.arrays(branches, library='np')

# Read background data
background_data_list = []

for bkg_file in background_files:
    with uproot.open(bkg_file) as file:
        NeutrinoData = file['NeutrinoData']
        bkg_data = NeutrinoData.arrays(branches, library='np')
        background_data_list.append(bkg_data)

# Concatenate background data
def concatenate_data(data_list):
    concatenated_data = {}
    for branch in branches:
        concatenated_data[branch] = np.concatenate([data[branch] for data in data_list])
    return concatenated_data

background_data = concatenate_data(background_data_list)

# Initialize variables
num_total_signal_events = np.sum(signal_data['weight'])
num_total_background_events = np.sum(background_data['weight'])

signal_cut_mask = np.ones(len(signal_data['weight']), dtype=bool)
background_cut_mask = np.ones(len(background_data['weight']), dtype=bool)

signal_efficiencies = []
background_efficiencies = []
efficiency_ratios = []

# Define cuts
cuts = [
    ("leptonCount == 0", lambda data: data['leptonCount'] == 0),
    ("negPionCount > 0", lambda data: data['negPionCount'] > 0),
    ("leadingPionEnergy > 0.25 GeV", lambda data: data['leadingPionEnergy'] > 0.25),
    ("otherParticleEnergySum > 0.6 GeV", lambda data: data['otherParticleEnergySum'] > 0.6),
    ("missingTransverseMomentum < 1.0 GeV", lambda data: data['missingTransverseMomentum'] < 1.0)
]

# Loop over cuts
for cut_name, cut_func in cuts:
    if cut_func is not None:
        signal_cut_mask &= cut_func(signal_data)
        background_cut_mask &= cut_func(background_data)
    
    # Calculate efficiencies
    signal_weight_after_cuts = np.sum(signal_data['weight'][signal_cut_mask])
    signal_efficiency = signal_weight_after_cuts / num_total_signal_events
    signal_efficiencies.append(signal_efficiency)
    
    background_weight_after_cuts = np.sum(background_data['weight'][background_cut_mask])
    background_efficiency = background_weight_after_cuts / num_total_background_events
    background_efficiencies.append(background_efficiency)
    
    if background_efficiency != 0:
        efficiency_ratio = signal_efficiency / background_efficiency
    else:
        efficiency_ratio = np.inf  # Handle division by zero
    efficiency_ratios.append(efficiency_ratio)

# Plotting
cut_labels = [
    r'$N_{lep} = 0$',
    r'$N_{\pi^-} > 0$',
    r'$\pi_{lead}^- > 0.25\ {GeV}$',
    r'$\sum E_{other} > 0.6\ {GeV}$',
    r'$p_T^{miss} < 1.0\ {GeV}$'
]

plt.figure(figsize=(6, 6))
plt.step(range(len(cut_labels)), efficiency_ratios, where="mid")
plt.xticks(range(len(cut_labels)), cut_labels, rotation=20, ha='right')
plt.xlabel('Cuts Applied')
plt.ylabel('Efficiency Ratio (Signal / Background)')
plt.title('Efficiency Ratio vs. Cumulative Cuts')
plt.tight_layout()
plt.savefig("../nutau-data/images/cut-flow.svg")

