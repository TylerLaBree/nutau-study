import uproot
import matplotlib.pyplot as plt
import numpy as np
import awkward as ak

def plot_initial_neutrino_energy(files_data, output_filename):
    # Bins for the histogram of initial neutrino energy
    bins = np.linspace(0, 20, 80)
    # Set up the figure and axes for multiplots
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))  # two plots side by side

    for ax_idx, (plot_title, files, colors, labels) in enumerate(files_data):
        ax = axes[ax_idx]
        ax.set_title(plot_title)
        
        # Processing each file
        for i, (label, path) in enumerate(files.items()):
            with uproot.open(path) as f:
                tree = f['NeutrinoData']
                # Filtering based on 'include' branch
                arrays = tree.arrays(filter_name=['ParticleEnergies', 'weight', 'include'], library='ak')
                # Apply the filter 'include == 1'
                filtered_arrays = arrays[arrays['include'] == 1]
                initial_neutrino_energy = filtered_arrays['ParticleEnergies'][:, 0]  # First energy for each event after filtering
                weights = filtered_arrays['weight'] * 1.3e-25  # Scaling factor

                ax.hist(initial_neutrino_energy, bins=bins, weights=weights, color=colors[i], histtype="step", linewidth=1.5, label=f'{labels[i]}')
                ax.set_xlabel('Initial Neutrino Energy [GeV]')
                ax.set_ylabel('Events per POT kT Year')
                ax.legend()

    plt.tight_layout()
    plt.savefig(f"../nutau-data/images/{output_filename}")

# Data for plots
data = [
    ('Event rate (hadronic decay)', {
        'signal': '../nutau-data/machado/new/nu_16_cc.root',
        '12_nc': '../nutau-data/machado/new/nu_12_nc.root',
        '14_nc': '../nutau-data/machado/new/nu_14_nc.root',
        '16_nc': '../nutau-data/machado/new/nu_16_nc.root'
    }, ['#EE3311', '#3161F3', '#0F8E17', '#FF9900'], ['$\\tau$ CC (signal)', '$e$ NC (bkg)', '$\mu$ NC (bkg)', '$\\tau$ NC (bkg)']),

    ('Event rate (leptonic decay)', {
        'signal': '../nutau-data/machado/new/nu_16_cc.root',
        '12_cc': '../nutau-data/machado/new/nu_12_cc.root',
        '14_cc': '../nutau-data/machado/new/nu_14_cc.root'
    }, ['#EE3311', '#3161F3', '#0F8E17'], ['$\\tau$ CC (signal)', '$e$ CC (bkg)', '$\mu$ CC (bkg)'])
]

# Generate multiplots
plot_initial_neutrino_energy(data, 'event-rates-combined.png')

