import matplotlib.pyplot as plt

FONT_SCALE = 2.2
SMALL_SIZE = 8*FONT_SCALE
MEDIUM_SIZE = 10*FONT_SCALE
BIGGER_SIZE = 12*FONT_SCALE

plt.rc('font', size=SMALL_SIZE)          # controls default text sizes
plt.rc('axes', titlesize=SMALL_SIZE)     # fontsize of the axes title
plt.rc('axes', labelsize=MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('ytick', labelsize=SMALL_SIZE)    # fontsize of the tick labels
plt.rc('legend', fontsize=SMALL_SIZE)    # legend fontsize
plt.rc('figure', titlesize=BIGGER_SIZE)  # fontsize of the figure title

fig = plt.figure(num=id)
fig.canvas.set_window_title(title)

if fig.get_figheight() < 5:
  fig.set_figheight(fig.get_figheight()*2)
  fig.set_figwidth(fig.get_figwidth()*2*aspectratio)

plt.clf()
plt.jet()
plt.imshow(z, extent=[xmin, xmax, ymin, ymax], interpolation='bilinear' if interp else 'none',
           aspect='auto')
cbar = plt.colorbar()

if xlabel:
  plt.xlabel(xlabel)
if ylabel:
  plt.ylabel(ylabel)
if zlabel:
  cbar.set_label(zlabel)
if title:
  plt.title(title)

plt.draw()
plt.pause(1e-9)
