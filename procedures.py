import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os.path
from os import path

colors = {
			  'Subscription Forest':'red'
			, 'subscriptionForest':'red'
			, 'OpIndex':'cyan'
			, 'opIndex':'cyan'
			, 'TAMA':'brown'
			, 'tama':'brown'
			, 'REIN':'magenta'
			, 'rein':'magenta'
			, "siena": "green"
			, "Siena": "green"
			, "ACTree":"blue"
			, "subscriptionBTree":"blue"
		}

# Without H-Tree
def plot(csvFile, x,y, discard=[]):
    df = pd.read_table(f'./output/{csvFile}.csv', sep=",") if path.exists(f'./output/{csvFile}.csv') else pd.read_table(f'./output/#save/{csvFile}.csv', sep=",") 
    fig, ax = plt.subplots()
    ax.set(xlabel=x, ylabel=y) 

    for key, grp in df.groupby(['Algorithm']):
    	if not key in discard:
        	ax = grp.plot(ax=ax, kind='line', x=x, y=y, c=colors[key], label=key)

    plt.legend(loc='best')
    plt.show()
