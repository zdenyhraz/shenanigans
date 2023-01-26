import matplotlib.pyplot as plt
import matplotlib.ticker as ticker


def plot(name, aspectratio, savepath, z, xmin, xmax, ymin, ymax, interp, xlabel, ylabel, zlabel, cmap):
    fig = plt.figure(num=name)
    fig.set_figwidth(fig.get_figwidth()*aspectratio)
    plt.clf()

    im = plt.imshow(z,
                    extent=[xmin, xmax, ymin, ymax],
                    interpolation='bilinear' if interp else 'none',
                    aspect='auto')

    #cbar = plt.colorbar(im, format=ticker.FuncFormatter(lambda x, pox: '{:.1e}'.format(x)))
    cbar = plt.colorbar(im)

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
