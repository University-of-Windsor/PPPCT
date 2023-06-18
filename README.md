# PPPCT
Privacy Preserving for Parallel Clustering Transcriptomic data (PPPCT) is the first framework for doing transcriptomic data for clustering single cell mRNA-seq datasets. 
This project works with datasets of UMI count obtained from gene expression profiles. Some of these kinds of these datasets are provided in the dataset folder of this project. However you can come up with new datasets by runing data_preparation.m in Matlab. 

## How to run:

For running this project you have to follow the following steps:
1. Install Intel SGX through the [documentation](https://github.com/intel/linux-sgx) <br/>
2. Install python 3.9 and scikit learn package through <br/>
```
    pip install scikit-learn
```
3. Clone the project in your workspace and go to the root folder (cd PPPCT)
```
clone https://github.com/University-of-Windsor/PPPCT.git && cd PPPCT
```
4. make the project using the following command
```
   make SGX_MODE=SIM 
```
    if you want to use debug mode or the hardware mode of the SGX substitute SIM to DEB <br/>

5. Once the project is built successfully, go to the folder "bin" in the root folder of the project. 
6. Run the following command in the EnclaveResponder(equal to worker in the paper)<br/>
```
    ./appresponder
```
    This command makes the worker listen on Port 8888, or whatever you prefer <br/> 
    The IP Address is defaulted to loopback address(127.0.0.1) you can change it, however.<br/>
7. Go to the trusted entity server (in remote/local computer) and run the following command <br/>
```
    ./appinitiator ../dataset/comb_M.csv 3 3000 3000 <br/>
```
This command does clustering with the collaboration of the remote worker(s). It is defaulted to the middle size dataset as the input (M-Set), you can change it to other datasets. The general overview of this command is: <br/>
```
./appinitiator "dataset" "number of clusters" "number of cells" "number of genes" <br/>
```
Because there are 3000 cells and 3000 genes in M-Set and we already know that there are 3 clusters in this dataset. The input command will be changed to the following:
```
./appinitiator dataset/comb_M.csv 3 3000 3000 <br/>
```
## Datasets
The dataset should be a Unique Molecular Identifier (UMI) count dataset in tabular format (non-sparse). Some of the examples of these datasets are already provided in dataset folder in the root folder of this work. <br/>
If you are going to change the dataset, some changes to the codes should be applied before doing the makefile. In this work, we do not care about the number of dimensions (which are considered as the number of features in our work). You only need to update the number of cells (sample records) in both worker and server side. For this reason you should update the folowing constant values if all datatypes.h files thoughout the project. 
<br>
1- #define N_ROWS_PER_WORKER  3000   <br/>
2- #define N_CLUSTERS         3      <br/>
3- #define N_COLS             4      <br/>

N_ROWS_PER_WORKER is equal to the number of cells. N_CLUSTERS is equal to the number of clusters. N_COLS is equal to the number of clusters plus 1.

## How to increase workers

  

Copyright © 2023, Ali Abbasi Tadi
https://www.researchgate.net/profile/Ali-Abbasi-Tadi
All rights reserved.

