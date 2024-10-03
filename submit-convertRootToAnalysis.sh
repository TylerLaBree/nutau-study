run=01

./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_e_cc_250k_events.root", "../nutau-data/analysis/nu_e_cc.root")' $run nu_e_cc convertRootToAnalysis
./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_e_nc_83k_events.root", "../nutau-data/analysis/nu_e_nc.root")' $run nu_e_nc convertRootToAnalysis
./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_mu_cc_250k_events.root", "../nutau-data/analysis/nu_mu_cc.root")' $run nu_mu_cc convertRootToAnalysis
./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_mu_nc_84k_events.root", "../nutau-data/analysis/nu_mu_nc.root")' $run nu_mu_nc convertRootToAnalysis
./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_tau_cc_176k_events.root", "../nutau-data/analysis/nu_tau_cc.root")' $run nu_tau_cc convertRootToAnalysis
./submit-job.sh ./batch-root.sh 'convertRootToAnalysis.C("../nutau-data/root/nu_tau_nc_83k_events.root", "../nutau-data/analysis/nu_tau_nc.root")' $run nu_tau_nc convertRootToAnalysis
