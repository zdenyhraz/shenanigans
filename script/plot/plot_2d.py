import matplotlib.pyplot as plt
fig = plt.figure(num=id)
if fig.get_size_inches()[1] < 6:
  fig.set_size_inches(fig.get_size_inches()*1.5)
plt.clf()
plt.jet()
plt.imshow(z, extent=[xmin, xmax, ymin, ymax], aspect='auto')
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
