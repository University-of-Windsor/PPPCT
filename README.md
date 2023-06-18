# PPPCT
Privacy Preserving for Parallel Clustering Transcriptomic data (PPPCT) is the first framework for doing transcriptomic data for clustering single cell mRNA-seq datasets. 
This project works with datasets of UMI count obtained from gene expression profiles. Some of these kinds of these datasets are provided in the dataset folder of this project. However you can come up with new datasets by runing data_preparation.m in Matlab. 

## How to run:
For running this project you have to follow the following steps:
1. Install Intel SGX through the [documentation](https://github.com/intel/linux-sgx)
2. Install python 3.9 and scikit learn package through <br/>
    pip install scikit-learn
3. Clone the project in your workspace and go to the root folder (cd PPPCT)
4. make the project using the following command
    <br/> make SGX_MODE=SIM <br/>
    if you want to use debug mode or the hardware mode of the SGX substitute SIM to DEB <br/>
5. Once the project is built successfully, go to the folder "bin" in the root folder of the project. 
6. Run the following command in the worker<br/>
    ./appresponder <br/>
    This command makes the worker listen on Port 8888, or whatever you prefer <br/> 
    The IP Address is defaulted to loopback address(127.0.0.1) you can change it, however.<br/>
7. Go to the data owner server (in remote/local computer) and run the following command <br/>
    ./appinitiator ../dataset/comb_M.csv 3 3000 3000 <br/>
This command does clustering with the collaboration of the remote worker(s). It adopts the middle size dataset as the input, you can change it to other datasets. The general overview of this command is: <br/>
./appinitiator "dataset" "number of clusters" "number of cells" "number of genes" <br/>

## Datasets

## How to increase workers



Copyright © 2023, Ali Abbasi Tadi
https://www.researchgate.net/profile/Ali-Abbasi-Tadi
All rights reserved.

