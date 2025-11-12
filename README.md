# PA-GPSR
Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks

Link: https://ieeexplore.ieee.org/document/8638929

This repository provides PA-GPSR for ns-3. Besides that, we have implemented two competitors (MM-GPSR and an updated code of [GPSR](https://code.google.com/archive/p/ns3-gpsr/)) also available in this repository.

## Supported ns-3 Versions

- **ns-3.40** ✅ (Latest - Recommended)
- **ns-3.23** (Original version)

# Installation for ns-3.40 (CMake-based)

**Note**: This version has been updated to support ns-3.40 with CMake build system. For detailed migration information, see [MIGRATION_TO_NS3.40.md](MIGRATION_TO_NS3.40.md).

## Quick Start

1. Install all required dependencies for ns-3. You can find the commands [here](https://www.nsnam.org/wiki/Installation).

2. Download and install ns-3.40:

```bash
# Download ns-3.40
wget https://www.nsnam.org/releases/ns-allinone-3.40.tar.bz2
tar xjf ns-allinone-3.40.tar.bz2
cd ns-allinone-3.40/ns-3.40

# Clone this repository
cd ~
git clone https://github.com/CSVNetLab/PA-GPSR

# Copy modules to contrib directory
cd ~/ns-allinone-3.40/ns-3.40
cp -r ~/PA-GPSR/src/location-service contrib/
cp -r ~/PA-GPSR/src/pagpsr contrib/
cp -r ~/PA-GPSR/src/gpsr contrib/
cp -r ~/PA-GPSR/src/mmgpsr contrib/

# Copy example program
cp ~/PA-GPSR/examples/pagpsr-main.cc scratch/

# Configure and build
./ns3 configure --enable-examples --enable-tests
./ns3 build
```

3. Verify installation:

```bash
./ns3 show modules | grep -E "location-service|pagpsr|gpsr|mmgpsr"
```

4. Run the example:

```bash
./ns3 run "scratch/pagpsr-main --size=30 --time=200 --algorithm=pagpsr"
```

## Key Changes for ns-3.40

- **Build System**: Changed from waf to CMake
- **WiFi Configuration**: Updated to use `WifiMacHelper` and `OcbWifiMac`
- **MAC Layer**: Removed dependency on deprecated `AdhocWifiMac`
- **CMakeLists.txt**: Added for each module

For complete details, see [MIGRATION_TO_NS3.40.md](MIGRATION_TO_NS3.40.md).

---

# Installation for ns-3.23 (Original - waf-based)
1. Install all required dependencies required by ns-3. You can find the commands [here](https://www.nsnam.org/wiki/Installation).
2. Download ns3.

```
 wget http://www.nsnam.org/release/ns-allinone-3.23.tar.bz2
 tar xjf ns-allinone-3.23.tar.bz2
 cd ns-allinone-3.23
 git clone https://github.com/CSVNetLab/PA-GPSR
 cp -a PA-GPSR/src/. ns-3.23/src/
 cp -a PA-GPSR/examples/. ns-3.23/scratch/
 cp -a PA-GPSR/results/ ns-3.23/
 cp -a PA-GPSR/figures/ ns-3.23/
 cp -a PA-GPSR/scripts/ ns-3.23/
 cp PA-GPSR/main.sh ns-3.23/

 cd ns-3.23
 CXXFLAGS="-Wall -std=c++0x" ./waf configure --enable-examples
 ./waf
 sudo ./waf install
```
# Running

On the terminal, execute the command: 

```
sh main.sh
```

You can change the parameters of simulation through this file. The parameters are:

- nCores -> **Number of parallel executions bounded by the number of cores that your computer has.**
- algorithms -> **Algorithm name that you want to simulate. Can be pagpsr, gpsr and mmgpsr.**
- seeds -> **A vector of random seeds (size = 30) to perform the simulation.**
- speed -> **Maximum speed of vehicles.**
- cbrconn -> **Vector containing the numbers of source-destination pairs.**

How to reference PA-GPSR?
============
Please use the following bibtex :

# Citation
``` bibtex

@ARTICLE{silva2019improvement, 
author={A. {Silva} and N. {Reza} and A. {Oliveira}}, 
journal={IEEE Access}, 
title={Improvement and Performance Evaluation of GPSR-Based Routing Techniques for Vehicular Ad Hoc Networks}, 
year={2019}, 
volume={7}, 
number={}, 
pages={21722-21733}, 
keywords={Routing;Ad hoc networks;Routing protocols;Safety;Wireless communication;Delays;GPSR;Routing protocol;VANETs;SUMO;NS-3}, 
doi={10.1109/ACCESS.2019.2898776}, 
ISSN={2169-3536}, 
month={Feb},}
```
