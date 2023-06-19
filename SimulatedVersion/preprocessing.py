# -*- coding: utf-8 -*-
"""
Created on Sun Sep 11 12:01:08 2022

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
        components=3;

    print("Running NMF Transformation .... ")
    W2=np.sqrt(csvFile.T)
    W3=np.log1p(W2)
    model = NMF(n_components=components+1, init='nndsvd', random_state=30)
    W = model.fit_transform(W3)
    H = model.components_
    print("Running NMF Transformation .... Done! ")

    print("W shape: ",np.shape(W));
    #print (W)
    print("H shape: ",np.shape(H));
    df_w = pd.DataFrame(W)
    df_H = pd.DataFrame(H)

    df_w.to_csv('W.csv',sep=',',header=False,index=False)
    print('W has been prepared and written to W.csv')
    df_H.to_csv('H.csv',sep=',',header=False,index=False)
    print('H has been prepared and written to H.csv')         
