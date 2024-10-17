import matplotlib.pyplot as plt


def plot(name, aspectratio, savepath, z, xmin, xmax, ymin, ymax, interp, xlabel, ylabel, zlabel, cmap):
    fig = plt.figure(num=name, figsize=[plt.rcParams["figure.figsize"][0]*aspectratio, plt.rcParams["figure.figsize"][1]])
    plt.clf()

    im = plt.imshow(z,
                    extent=[xmin, xmax, ymin, ymax],
                    interpolation='bilinear' if interp else 'none',
                    aspect='auto')

    cbar = plt.colorbar(im, orientation='vertical', aspect=15, extend='both', pad=0.025)

    if xlabel:
        plt.xlabel(xlabel)
    if ylabel:
        plt.ylabel(ylabel)

    plt.title(name)
    plt.set_cmap(cmap)
    plt.tight_layout()

    if savepath:
        print(f"Saving plot {name} to {savepath}")
        plt.savefig(savepath, bbox_inches='tight')
    else:
        plt.draw()
        plt.pause(0.001)
