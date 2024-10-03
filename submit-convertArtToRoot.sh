run=01

./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuECC/","../nutau-data/root/nu_e_cc_250k_events.root", 1, 12)' $run nu_e_cc convertArtToRoot
./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuENC/","../nutau-data/root/nu_e_nc_83k_events.root", 0, 12)' $run nu_e_nc convertArtToRoot
./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuMuCC/","../nutau-data/root/nu_mu_cc_250k_events.root", 1, 14)' $run nu_mu_cc convertArtToRoot
./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuMuNC/","../nutau-data/root/nu_mu_nc_84k_events.root", 0, 14)' $run nu_mu_nc convertArtToRoot
./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuTauCC/","../nutau-data/root/nu_tau_cc_176k_events.root", 1, 16)' $run nu_tau_cc convertArtToRoot
./submit-job.sh ./batch-root.sh 'convertArtToRoot.C("../nutau-data/art/NuTauNC/","../nutau-data/root/nu_tau_nc_83k_events.root", 0, 16)' $run nu_tau_nc convertArtToRoot
