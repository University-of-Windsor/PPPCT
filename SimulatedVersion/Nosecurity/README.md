# PPPCT Simulated Version - No privacy protection
Privacy Preserving for Parallel Clustering Transcriptomic data (PPPCT) is the first framework for doing transcriptomic data for clustering single cell mRNA-seq datasets. 
This project works with datasets of UMI count obtained from gene expression profiles. Some of these kinds of these datasets are provided in the dataset folder of this project. However you can come up with new datasets by runing data_preparation.m in Matlab. 

## How to run no security version:

Unlike other versions, in this version you do not need to install Intel sgx sdk, as this version has no privacy protection. <br/>
This version is very simple to run. Just follow the steps below.

1. Install python 3.9 and scikit learn package through <br/>

```
    pip install scikit-learn
```

2. Clone the project in your workspace and go to the root folder (cd PPPCT)

```
clone https://github.com/University-of-Windsor/PPPCT.git && cd PPPCT/SimulatedVersion/Nosecurity
```

3. make the project using the following command

```
   make SGX_MODE=SIM 
```

if you want to use debug mode or the hardware mode of the SGX substitute SIM to DEB <br/>

4. Make sure the project is built successfully.
5. Run the following command in the SimulatedVersion/Nosecurity folder<br/>

This command does clustering with the collaboration of multiple threads. It is defaulted to the middle size dataset as the input (M-Set), you can change it to other datasets. The general overview of this command is: <br/>

```
./app "dataset" "number of clusters" "number of cells" "number of genes" 
```

Because there are 3000 cells and 3000 genes in M-Set and we already know that there are 3 clusters in this dataset. The input command will be changed to the following:

```
./app dataset/comb_M.csv 3 3000 3000 
```

7. In the next step, choose the number of workers (number of threads in simulated version): e.g. 6 <br/>

## Datasets

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


