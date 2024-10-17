import matplotlib.pyplot as plt


def plot(name, aspectratio, savepath, x, ys, y2s, xlabel, ylabel, y2label, log, ycolors, y2colors, ylinestyles, y2linestyles, ylabels, y2labels):
    fig = plt.figure(num=name, figsize=[plt.rcParams["figure.figsize"][0]*aspectratio, plt.rcParams["figure.figsize"][1]])
    plt.clf()
    ax1 = fig.subplots()
    ax2 = ax1.twinx() if len(y2s) > 0 else None
    color_cycle = ax1._get_lines.prop_cycler
    lines = []

    for i in range(0, len(ys)):
        if len(ys[i]) > 0:
            lines += ax1.plot(x, ys[i],
                              color=ycolors[i] if len(ycolors) > i else next(color_cycle)['color'],
                              linestyle=ylinestyles[i] if len(ylinestyles) > i else '-', label=ylabels[i]
                              if len(ylabels) > i else '', linewidth=4)

    for i in range(0, len(y2s)):
        if len(y2s[i]) > 0:
            lines += ax2.plot(x, y2s[i],
                              color=y2colors[i] if len(y2colors) > i else next(color_cycle)['color'],
                              linestyle=y2linestyles[i] if len(y2linestyles) > i else '-', label=y2labels[i]
                              if len(y2labels) > i else '', linewidth=4)

    ax1.set_xlabel(xlabel)
    ax1.set_ylabel(ylabel)
    if ax2 and y2label:
        ax2.set_ylabel(y2label)
    if ylabels or y2labels:
        labels = [line.get_label() for line in lines]
        plt.legend(lines, labels)
    if log:
        plt.yscale("log")

    plt.title(name)
    plt.tight_layout()

    if savepath:
        print(f"Saving plot {name} to {savepath}")
        plt.savefig(savepath, bbox_inches='tight')
    else:
        plt.draw()
        plt.pause(0.001)
