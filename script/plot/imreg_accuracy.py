import matplotlib.pyplot as plt

fig = plt.figure(num=id)
fig.set_figwidth(fig.get_figwidth()*aspectratio)
fig.canvas.manager.set_window_title(title)
plt.clf()
alpha = 0.3
linewidth = 3

ax1 = fig.subplots()

ax1.plot(x, pcs_error, color="tab:orange", label="pcs", linewidth=linewidth)
ax1.fill_between(
    x, np.array(pcs_error) - np.array(pcs_stddev),
    np.array(pcs_error) + np.array(pcs_stddev),
    color="tab:orange", alpha=alpha, linewidth=0.5*linewidth)

ax1.plot(x, ipc_error, color="tab:green", label="ipc", linewidth=linewidth)
ax1.fill_between(
    x, np.array(ipc_error) - np.array(ipc_stddev),
    np.array(ipc_error) + np.array(ipc_stddev),
    color="tab:green", alpha=alpha, linewidth=0.5*linewidth)


ax1.set_xlabel("reference shift x [px]")
ax1.set_ylabel("error [px]")
plt.legend()
plt.title(title)
if log:
  plt.yscale("log")
plt.tight_layout()
plt.draw()
plt.pause(1e-9)

if save:
  plt.savefig(save, bbox_inches='tight')
