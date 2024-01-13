# -*- coding: utf-8 -*-
"""
Created on Sun Sep 11 12:01:08 2023

@author: aliab
"""
from sklearn.decomposition import NMF
import numpy as np
import time
import pandas as pd
 

import sys

if __name__ == "__main__":
	
    if(sys.argv[1] is not None):
        csvFile = pd.read_csv(sys.argv[1], header=None)
    else:
        csvFile=pd.read_csv('comb_S.csv', header=None)
    if(sys.argv[2] is not None):
        components=int(sys.argv[2])
    else:
        components=3
    if(sys.argv[3] is not None):
        dr = sys.argv[3]
    else:
    	dr = "nmf"
    		
    print("Running Dimensionality Reduction .... ")
    t=.5
    W2=np.log1p(csvFile)
    W3=W2 ** t
    #W=W3.T#csvFile.T
    if(dr == "nmf"):
        model = NMF(n_components=components, init='nndsvd', random_state=0)
        print("Using NMF .... ")
        W = model.fit_transform(W3.T)
    elif (dr == "tsne" ):
    	from sklearn.manifold import TSNE
    	print("Using t-SNE .... ")
    	tsne_model = TSNE(n_components=2, random_state=0)
    	W = tsne_model.fit_transform(W3.T)
    elif (dr == "ica"):
    	from sklearn.decomposition import FastICA
    	print("Using ICA .... ")
    	ica_model = FastICA(n_components=components, random_state=0)
    	W = ica_model.fit_transform(W3.T)
    elif (dr == "umap" ):
    	import umap
    	print("Using UMAP .... ")
    	umap_model = umap.UMAP(n_components=components, random_state=0)
    	W = umap_model.fit_transform(W3.T)
    else:
    	print("Dimensionality reduction method is not recognized. Please use either: nmf, tsne, umap, or ica")
    print("Running Dimensionality reduction .... Done! ")
    print("scaling .... started! ")
    std = np.std(W,axis=0)
    mean=np.mean(W,axis=0)
    mean_temp=np.mean(W,axis=0)
    while mean_temp[np.argmax(std)] > mean[np.argmax(std)]/2:
        W=np.log1p(W)
        mean_temp=np.mean(W,axis=0)
    print("W shape: ",np.shape(W));
    #print (W)
    #print("H shape: ",np.shape(H));
    df_w = pd.DataFrame(W)
    #df_H = pd.DataFrame(H)

    df_w.to_csv('W.csv',sep=',',header=False,index=False)
    print('W has been prepared and written to W.csv')
    #df_H.to_csv('H.csv',sep=',',header=False,index=False)
    #print('H has been prepared and written to H.csv')         
