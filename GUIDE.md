Noxim_LBDR Guide
A version of Noxim supporting 2D turn models implemented using LBDR mechanism

---------

The source code of Noxim LBDR is stored in “src” folder. The user can download the code and build it, if necessary. 

All the scripts are stored in the “scripts” folder. 
In case of using Mac OS X, the executable of Noxim us stored in the “scripts” folder. In order to run Noxim using noxim_explorer, which reads the configuration from the sim.cfg file, the following steps should be taken, which can be done using the Terminal: 

cd scripts
./main_script.sh

This will create a folder “Results” in “scripts” in which the results for different traffic patterns are listed: bit reversal, butterfly, random, shuffle, transpose1 and transpose2.
The results are stored in .m (MATLAB) format. 
Also, the text version of the results are stored in the “scripts/collected_results” path for each traffic pattern. 

The results include average latency, average throughput and communication energy. 