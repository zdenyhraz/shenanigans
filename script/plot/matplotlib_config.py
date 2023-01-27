import numpy as np
import matplotlib.pyplot as plt


def init():
    #matplotlib.use('GTK3Agg')
    #print("Matplotlib is using the", matplotlib.get_backend(), "backend")

    FONT_SCALE = 1.5  # 1.5 normal 2 for LaTeX
    SMALL_SIZE = 8*FONT_SCALE
    MEDIUM_SIZE = 10*FONT_SCALE
    BIGGER_SIZE = 12*FONT_SCALE
    FIGSCALE = 1.3
    FIGHEIGHT = FIGSCALE*plt.rcParamsDefault["figure.figsize"][1]
    FIGWIDTH = FIGHEIGHT*1.17

    plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
    plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
    plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
    plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
    plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
    plt.rc('legend', fontsize=SMALL_SIZE)    # legend fontsize
    plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title
    plt.rcParams["figure.figsize"] = (FIGWIDTH, FIGHEIGHT)
