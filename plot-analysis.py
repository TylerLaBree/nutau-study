import uproot
import matplotlib.pyplot as plt
import numpy as np
import awkward as ak

# File paths and settings
files = {
    'signal': '../nutau-data/analysis/nu_tau_cc.root',
    '12_nc': '../nutau-data/analysis/nu_e_nc.root',
    '14_nc': '../nutau-data/analysis/nu_mu_nc.root',
    '16_nc': '../nutau-data/analysis/nu_tau_nc.root'
}
colors = ['#EE3311', '#3161F3', '#0F8E17', '#FF9900']
labels = ['$\\tau$ CC (signal)', '$e$ NC (bkg)', '$\mu$ NC (bkg)', '$\\tau$ NC (bkg)']
branches = ['leadingPionEnergy', 'otherParticleEnergySum', 'missingTransverseMomentum']
titles = ['Initial Neutrino Energy', 'Leading Pion Energy', 'Particle Energy Sum (except leading pion)', 'Missing Transverse Momentum']

# Bins for histogramming
bins = {
    'initialNeutrinoEnergy': np.linspace(0, 20, 80),
    'leadingPionEnergy': np.logspace(-1, np.log10(5), 30),
    'otherParticleEnergySum': np.logspace(-1, 1, 25),
    'missingTransverseMomentum': np.linspace(0, 3, 25)
}

# Set up the plot
fig, axs = plt.subplots(1, 4, figsize=(24, 6))
fig.suptitle('Distribution of Variables for Different Neutrino Interactions')

# Processing each file
for i, (label, path) in enumerate(files.items()):
    with uproot.open(path) as f:
        tree = f['NeutrinoData']
        arrays = tree.arrays(filter_name=branches + ['ParticleEnergies', 'include', 'leptonCount', 'negPionCount', 'weight'], library='ak')
        include_mask = (arrays['include'] == 1) & (arrays['leptonCount'] == 0) & (arrays['negPionCount'] > 0)
        
        # Use Awkward Array to handle jagged array of energies
        initial_neutrino_energy = arrays['ParticleEnergies'][:, 0]  # Get the first energy for each event

        weights = arrays['weight']
        weights_filtered = arrays['weight'][include_mask]

        # Plot each variable
        plot_branches = ['initialNeutrinoEnergy'] + branches
        for j, branch in enumerate(plot_branches):
            if branch == 'initialNeutrinoEnergy':
                data = initial_neutrino_energy
                plot_weights = weights * 1.3 * 10**(-25)
            else:
                data = arrays[branch][include_mask]
                plot_weights = weights_filtered * 1.3 * 10**(-25)
            
            axs[j].hist(data, bins=bins.get(branch, 30), weights=plot_weights, color=colors[i], histtype="step", linewidth=1.5, label=f'{labels[i]}')
            axs[j].set_xlabel(f"{titles[j]} [GeV]")
            axs[j].set_ylabel('Events per POT kT Year')
            if branch == 'initialNeutrinoEnergy':
                axs[j].legend()
            if branch in ["otherParticleEnergySum"]:
                axs[j].set_xscale('log')
                axs[j].set_yscale('log')

plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.savefig("../nutau-data/plots/analysis-plots.svg")

