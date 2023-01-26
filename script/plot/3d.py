import matplotlib.pyplot as plt
import numpy as np


def plot(name, aspectratio, savepath, z, xmin, xmax, ymin, ymax, xlabel, ylabel, zlabel, cmap):
    fig = plt.figure(num=id)
    fig.set_figwidth(fig.get_figwidth()*aspectratio)
    plt.clf()

    xsize = z.shape[0]
    ysize = z.shape[1]
    n = 101
    x, y = np.meshgrid(np.linspace(xmin, xmax, xsize), np.linspace(ymax, ymin, ysize))

    ax = fig.gca(projection='3d')
    surf = ax.plot_surface(
        x, y, z,
        cmap=cmap,
        edgecolor='none',
        linewidth=0,
        antialiased=False,
        rstride=max(int(ysize / n), 1),
        cstride=max(int(xsize / n), 1))

    ax.view_init(elev=25, azim=-130)
    ax.set_xlim(xmin, xmax)
    ax.set_ylim(ymin, ymax)
    cbar = fig.colorbar(surf, shrink=0.7, aspect=15)

    if xlabel:
        plt.xlabel(xlabel)
    if ylabel:
        plt.ylabel(ylabel)
    if zlabel:
        cbar.set_label(zlabel)

    plt.title(name)
    plt.set_cmap(cmap)
    plt.tight_layout()

    if savepath:
        print(f"Saving plot {name} to {savepath}")
        plt.savefig(savepath, bbox_inches='tight')
    else:
        plt.draw()
        plt.pause(1e-9)
