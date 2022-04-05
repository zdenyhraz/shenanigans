import matplotlib.pyplot as plt
import numpy as np


def Plot(
        id, title, aspectratio, save, x, pc_error, pc_stddev, pcs_error, pcs_stddev, ipc_error, ipc_stddev, ipco_error,
        ipco_stddev, log=False):
  fig = plt.figure(num=id)
  fig.set_figwidth(fig.get_figwidth()*aspectratio)
  fig.canvas.manager.set_window_title(title)
  plt.clf()
  alpha = 0.3
  linewidth = 3

  ax1 = fig.subplots()
  ax1.plot(x, pc_error, color="tab:orange", label="pc", linewidth=linewidth)
  ax1.fill_between(
      x, np.array(pc_error) - np.array(pc_stddev),
      np.array(pc_error) + np.array(pc_stddev),
      color="tab:orange", alpha=alpha, linewidth=0.5*linewidth)

  ax1.plot(x, pcs_error, color="tab:blue", label="pcs", linewidth=linewidth)
  ax1.fill_between(
      x, np.array(pcs_error) - np.array(pcs_stddev),
      np.array(pcs_error) + np.array(pcs_stddev),
      color="tab:blue", alpha=alpha, linewidth=0.5*linewidth)

  ax1.plot(x, ipc_error, color="tab:purple", label="ipc", linewidth=linewidth)
  ax1.fill_between(
      x, np.array(ipc_error) - np.array(ipc_stddev),
      np.array(ipc_error) + np.array(ipc_stddev),
      color="tab:purple", alpha=alpha, linewidth=0.5*linewidth)

  ax1.plot(x, ipco_error, color="tab:green", label="ipco", linewidth=linewidth)
  ax1.fill_between(
      x, np.array(ipco_error) - np.array(ipco_stddev),
      np.array(ipco_error) + np.array(ipco_stddev),
      color="tab:green", alpha=alpha, linewidth=0.5*linewidth)

  ax1.set_xlabel("reference shift x [px]")
  ax1.set_ylabel("error [px]")
  plt.legend()
  plt.title(title)
  if log:
    plt.yscale("log")
  plt.tight_layout()

  if save:
    plt.savefig(save, bbox_inches='tight')
  else:
    plt.draw()
    plt.pause(1e-9)
