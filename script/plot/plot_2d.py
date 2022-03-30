import matplotlib.pyplot as plt

fig = plt.figure(num=id)
fig.canvas.set_window_title(title)
fig.set_figheight(fig.get_figheight())
fig.set_figwidth(fig.get_figwidth()*aspectratio)

plt.clf()
im = plt.imshow(z,
                extent=[xmin, xmax, ymin, ymax],
                interpolation='bilinear' if interp else 'none',
                aspect='auto')

cbar = plt.colorbar(im)

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
