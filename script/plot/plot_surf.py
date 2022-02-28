import matplotlib.pyplot as plt
import numpy as np

fig = plt.figure(num=id)
fig.canvas.set_window_title(title)

if fig.get_figheight() < 5:
  fig.set_figheight(fig.get_figheight()*2)
  fig.set_figwidth(fig.get_figwidth()*2*aspectratio)

plt.clf()

Z = np.array([np.array(row) for row in z])
X = np.linspace(xmin, xmax, len(z[0]))
Y = np.linspace(ymin, ymax, len(z))
X, Y = np.meshgrid(X, Y)

ax = fig.gca(projection='3d')
surf = ax.plot_surface(X, Y, Z, cmap=cmap, edgecolor='none', antialiased=True)
cbar = fig.colorbar(surf, shrink=0.7, aspect=15)

if xlabel:
  plt.xlabel(xlabel)
if ylabel:
  plt.ylabel(ylabel)
if zlabel:
  cbar.set_label(zlabel)
if title:
  plt.title(title)
if save:
  plt.savefig(save, bbox_inches='tight')

plt.set_cmap(cmap)
plt.tight_layout()
plt.draw()
plt.pause(1e-9)
