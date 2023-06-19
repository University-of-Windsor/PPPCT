# PPPCT
Privacy Preserving for Parallel Clustering Transcriptomic data (PPPCT) is the first framework for doing transcriptomic data for clustering single cell mRNA-seq datasets. 
This project works with datasets of UMI count obtained from gene expression profiles. Some of these kinds of these datasets are provided in the dataset folder of this project. However you can come up with new datasets by runing data_preparation.m in Matlab. 

## How to run:

For running this project you have to follow the following steps:
1. Install Intel SGX through the [documentation](https://github.com/intel/linux-sgx) <br/>
    for making sure that linux sgx sdk is successfully installed, we recommend running a sgx sample codes iside the sdk. <br/>
2. Install python 3.9 and scikit learn package through <br/>

```
    pip install scikit-learn
```

3. Clone the project in your workspace and go to the root folder (cd PPPCT)

```
clone https://github.com/University-of-Windsor/PPPCT.git && cd PPPCT/SimulatedVersion
```

4. make the project using the following command

```
   make SGX_MODE=SIM 
```

if you want to use debug mode or the hardware mode of the SGX substitute SIM to DEB <br/>

5. Make sure the project is built successfully.
6. Run the following command in the SimulatedVersion folder<br/>

```
    ./app "dataset" "number of clusters" "number of cells" "number of genes" 
```

This command does clustering with the collaboration of multiple threads. It is defaulted to the middle size dataset as the input (M-Set), you can change it to other datasets. The general overview of this command is: <br/>

```
./app "dataset" "number of clusters" "number of cells" "number of genes" 
```

Because there are 3000 cells and 3000 genes in M-Set and we already know that there are 3 clusters in this dataset. The input command will be changed to the following:

```
./appinitiator dataset/comb_M.csv 3 3000 3000 
```

7. In the next step, choose the number of workers (number of threads in simulated version): e.g. 6 <br/>

## Datasets

You can use all the datasets in PPPCT root folder.
Once the application give you the labels, you should run ARI computation using labels and the correct labels for that specific dataset in PPPCT's datset folder



