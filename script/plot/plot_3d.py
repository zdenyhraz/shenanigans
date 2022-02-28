import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure(num=id)
fig.canvas.set_window_title(title)

if fig.get_figheight() < 5:
  fig.set_figheight(fig.get_figheight()*2)
  fig.set_figwidth(fig.get_figwidth()*2*aspectratio)

Z = np.array([np.array(row) for row in z])
xsize = Z.shape[0]
ysize = Z.shape[1]
n = 101
X = np.linspace(xmin, xmax, xsize)
Y = np.linspace(ymax, ymin, ysize)
X, Y = np.meshgrid(X, Y)

plt.clf()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(
    X, Y, Z,
    cmap=cmap,
    edgecolor='none',
    linewidth=0,
    antialiased=False,
    rstride=rstride if rstride else max(int(ysize / n), 1),
    cstride=cstride if cstride else max(int(xsize / n), 1))

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
if title:
  plt.title(title)

plt.set_cmap(cmap)
plt.tight_layout()
plt.draw()
plt.pause(1e-9)
if save:
  plt.savefig(save, bbox_inches='tight')
