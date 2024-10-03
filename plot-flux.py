import uproot
import matplotlib.pyplot as plt
import numpy as np

def plot_flux_histograms(file_path, hist_names, colors, labels, plot_title, output_filename):
    # Set up the plot
    fig, ax = plt.subplots(figsize=(10, 6))
    fig.suptitle(plot_title)

    with uproot.open(file_path) as f:
        # Processing each histogram
        for hist_name, color, label in zip(hist_names, colors, labels):
            histogram = f[hist_name].to_numpy()  # Retrieve histogram as numpy array
            bin_edges = histogram[1]
            bin_centers = (bin_edges[:-1] + bin_edges[1:]) / 2  # Calculate bin centers for plotting
            bin_widths = np.diff(bin_edges)  # Calculate bin widths
            densities = histogram[0] / bin_widths  # Divide bin heights by bin widths for normalization

            # Using step to create the histogram plot
            ax.step(bin_edges[:-1], densities, where='post', color=color, linewidth=1.5, label=label)

    ax.set_xlabel('Neutrino Energy [GeV]')
    ax.set_ylabel(r'Particles per GeV$^{2}$ m$^{2}$ POT')
    ax.set_xlim(0, 20)
    ax.axvline(x=3.46, color='black', linestyle='--', linewidth=1.5, label='3.46 GeV kinematic threshold')
    ax.legend()

    plt.tight_layout()
    plt.savefig(f"../nutau-data/images/{output_filename}")

# File and histogram details
file_path = '../nutau-data/rate/flux_dune_neutrino_FD.root'
hist_names = ['nutau_fluxosc', 'numu_fluxosc', 'nue_fluxosc']
colors = ['#EE3311', '#0F8E17', '#3161F3']  # Colors corresponding to 14_nc, 12_nc, and 16_cc
labels = ['$\\nu_\\tau$ flux (oscillated)', '$\\nu_\\mu$ flux (oscillated)', '$\\nu_e$ flux (oscillated)']

# Generate plot
plot_flux_histograms(file_path, hist_names, colors, labels, 'Normalized Neutrino Flux at DUNE', 'normalized-neutrino-flux-dune.png')

