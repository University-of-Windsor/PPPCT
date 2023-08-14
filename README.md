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

This command makes the worker listen on Port 8888, or whatever you prefer. The IP Address is defaulted to loopback address(127.0.0.1) you can change it, however.<br/>
7. Go to the trusted entity server (in remote/local computer) and run the following command <br/>

```
    ./appinitiator ../dataset/comb_S.csv 3 1200 1000 
```

This command does clustering with the collaboration of the remote worker(s). It is defaulted to the middle size dataset as the input (M-Set), you can change it to other datasets. The general overview of this command is: <br/>

```
./appinitiator "dataset" "number of clusters" "number of cells" "number of genes" 
```

Because there are 1200 cells and 1000 genes in M-Set and we already know that there are 3 clusters in this dataset. The input command will be changed to the following:

```
./appinitiator dataset/comb_S.csv 3 1200 1000 
```

## Datasets Change
The dataset should be a Unique Molecular Identifier (UMI) count dataset in tabular format (non-sparse). Some of the examples of these datasets are already provided in dataset folder in the root folder of this work. <br/>
If you are going to change the dataset, some changes to the codes should be applied before doing the makefile. In this work, we do not care about the number of dimensions (which are considered as the number of features in our work). You only need to update the number of cells (sample records) in both worker and server side. For this reason you should update the folowing constant values if all datatypes.h files thoughout the project. 
<br>
1- ```#define N_ROWS_PER_WORKER  1200```   <br/>
2- ```#define N_CLUSTERS         3```      <br/>
3- ```#define N_COLS             4```      <br/>

```N_ROWS_PER_WORKER``` is equal to the number of cells that each worker needs to hanle (e.g. if there are 1200 cells and 3 clusters, then ```N_ROWS_PER_WORKER ``` should be 1200). ```N_CLUSTERS``` is equal to the number of clusters. ```N_COLS``` is equal to the number of clusters plus 1.

## How to increase workers
To increase the number of workers, you should follow multiple steps: <br/>
1- copy and paste the folder ```EnclaveResponder and AppResponder``` for whatever number of workers you need. For example

``` 
cp EnclaveResponder EnclaveResponder2
cp AppResponder AppResponder2
``` 

2- Update the number of workers in ``` AppInitiator/App.cpp``` to whatever is preferred for you. For example

``` 
uint32_t num_workers = 2;
``` 
3- Update the bash script file to compile what is included in the cloned files. Open makefile in the root folder of the project and add whatever files you just cloned. For example: <br/>
```
SUB_DIR := AppInitiator AppResponder EnclaveInitiator EnclaveResponder App AppResponder2 EnclaveResponder2 
```
Finally, compile files using makefile in the root folder. Compile by following command:
```
   make SGX_MODE=SIM 
```
If you feel the extension of the real-world PPPCT is a bit confusing using socket programming. We have prepared a simulated version of this work where all of the workers are inside one envclave. In the simulated version each thread is considered as a worker and using MPI programming the coordination between threads happen. You can reach to the simulated version inside the ```SimulatedVersion``` folder. Please follow readme.md in that folder for compiling the simulation version. 

## How to compute Adjusted Rand Index (ARI)
You can use all the datasets in PPPCT root folder.
Once the application gives you the labels, you should run ARI computation using labels and the correct labels for that specific dataset in PPPCT's datset folder to compute ARI. you can use the following codes in python using scikit-learn library. Copy and paste the predicted labels in the output screen into a .txt file named ```predicted_labels.txt```, also copy and paste the true labels inside a .txt file named ```true_labels.txt``` then run the following codes in python.

```python
from sklearn import metrics

# Read true labels from file
with open("true_labels.txt", "r") as file:
    true_labels = [int(label.strip()) for label in file.readlines()]

# Read predicted labels from file
with open("predicted_labels.txt", "r") as file:
    predicted_labels = [int(label.strip()) for label in file.readlines()]

# Compute ARI
ari = metrics.adjusted_rand_score(true_labels, predicted_labels)

# Print ARI
print("Adjusted Rand Index (ARI):", ari)
```
In the next version of PPPCT we will internalize these codes into our work, so you do not need to do extra codings to get results of ARI <br/>

## Comparison
In counterparts folder, you will find the comparing methods in the paper. These methods are mostly implemented in R, where you should install R studio to run them easily. One of our counter parts is ParaDPMM, which is a linux based method, likewise PPPCT, and you can read its documentation to run through [ParaDPMM](https://github.com/tiehangd/Para_DPMM)


## In order to reproduce the work, feel free to contact me at Abbasit@uwindsor.ca


## PPPCT is under review by the journal of Computers and security. 

Copyright © 2023, Ali Abbasi Tadi <br/>
https://www.researchgate.net/profile/Ali-Abbasi-Tadi <br/>
All rights reserved.

