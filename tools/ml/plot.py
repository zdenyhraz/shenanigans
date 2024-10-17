import matplotlib.pyplot as plt

FONT_SCALE = 1.3  # 1.5 normal 2 for LaTeX
SMALL_SIZE = 8*FONT_SCALE
MEDIUM_SIZE = 10*FONT_SCALE
BIGGER_SIZE = 12*FONT_SCALE
FIGSCALE = 1.3
FIGHEIGHT = FIGSCALE*plt.rcParamsDefault["figure.figsize"][1]
FIGWIDTH = FIGHEIGHT*1.2

plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=MEDIUM_SIZE)   # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title
plt.rcParams["figure.figsize"] = (FIGWIDTH, FIGHEIGHT)


def create_fig(name, num_rows=1, num_cols=1, aspect_ratio=1, position=None, sharex=False, sizeMultiplier=1):
    if num_rows == 1 and num_cols == 1:
        fig = plt.figure(num=name, figsize=[plt.rcParams["figure.figsize"][0]*aspect_ratio*sizeMultiplier, plt.rcParams["figure.figsize"][1]*sizeMultiplier])
        axs = []
    else:
        fig, axs = plt.subplots(num_rows, num_cols, num=name, figsize=[
            plt.rcParams["figure.figsize"][0]*aspect_ratio*num_cols, plt.rcParams["figure.figsize"][1]*num_rows], sharex=sharex)
    if position:
        import matplotlib
        backend = matplotlib.get_backend()
        if backend == 'TkAgg':
            fig.canvas.manager.window.wm_geometry("+%d+%d" % position)
        elif backend == 'WXAgg':
            fig.canvas.manager.window.SetPosition(position)
        else:
            # This works for QT and GTK
            # You can also use window.setGeometry
            fig.canvas.manager.window.move(position)

    return fig, axs


def fig_size(fig):
    return fig.get_size_inches()*fig.dpi


def fig_width(fig):
    return fig.get_size_inches()[0]*fig.dpi


def fig_height(fig):
    return fig.get_size_inches()[1]*fig.dpi
