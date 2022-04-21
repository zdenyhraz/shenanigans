import matplotlib.pyplot as plt
import matplotlib.ticker as ticker


def plot(id, title, aspectratio, save, z, xmin, xmax, ymin, ymax, interp, xlabel, ylabel, zlabel, cmap):
  fig = plt.figure(num=id)
  fig.set_figwidth(fig.get_figwidth()*aspectratio)
  fig.canvas.manager.set_window_title(title)
  plt.clf()

  im = plt.imshow(z,
                  extent=[xmin, xmax, ymin, ymax],
                  interpolation='bilinear' if interp else 'none',
                  aspect='auto')

  cbar = plt.colorbar(im, format=ticker.FuncFormatter(lambda x, pox: '{:.1e}'.format(x)))

  if xlabel:
    plt.xlabel(xlabel)
  if ylabel:
    plt.ylabel(ylabel)
  if zlabel:
    cbar.set_label(zlabel)

  plt.title(title)
  plt.set_cmap(cmap)
  plt.tight_layout()

  if save:
    print(f"Saving plot {title} to {save}")
    plt.savefig(save, bbox_inches='tight')
  else:
    plt.draw()
    plt.pause(1e-9)
