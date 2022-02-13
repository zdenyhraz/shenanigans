import matplotlib.pyplot as plt

plt.figure(id)

plt.imshow(z)
plt.jet()
plt.colorbar()

if xlabel:
  plt.xlabel(xlabel)
if ylabel:
  plt.ylabel(ylabel)
if title:
  plt.title(title)

plt.draw()
plt.pause(1e-9)
