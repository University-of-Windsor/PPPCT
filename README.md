# PPPCT
This project is the first framework for doing transcriptomic data for clustering single cell RNA-seq datasets. 

## How to run:
=========================
For running this project you have to follow the following steps:
1- Install Intel SGX through the documentation https://github.com/intel/linux-sgx
2- Install python 3.9 and scikit learn package through 
    pip install scikit-learn
3- Clone the project in your workspace
4- run make SGX_MODE=SIM
once the project is built successfully, go the root folder bin in the root folder of the project. 
5-Run the following command in the worker
    ./appresponder
6- Go to the data owner server (in remote/local computer) and run the following command
    ./appinitiator ../dataset/comb_M.csv 3 3000 3000

## Datasets

## How to increase workers



Copyright © 2023, Ali Abbasi Tadi
https://www.researchgate.net/profile/Ali-Abbasi-Tadi
All rights reserved.

