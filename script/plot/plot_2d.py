import matplotlib.pyplot as plt

fig = plt.figure(num=id)
fig.canvas.set_window_title(title)

if fig.get_figheight() < 5:
  fig.set_figheight(fig.get_figheight()*2)
  fig.set_figwidth(fig.get_figwidth()*2*aspectratio)

plt.clf()
plt.jet()
im = plt.imshow(z, extent=[xmin, xmax, ymin, ymax], interpolation='bilinear' if interp else 'none', aspect='auto')
cbar = plt.colorbar(im)

if xlabel:
  plt.xlabel(xlabel)
if ylabel:
  plt.ylabel(ylabel)
if zlabel:
  cbar.set_label(zlabel)
if title:
  plt.title(title)

plt.tight_layout()
plt.draw()
plt.pause(1e-9)
