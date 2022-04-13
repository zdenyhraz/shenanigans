import numpy as np
import matplotlib.pyplot as plt
import matplotlib_init


def Gaussian(n, sigmax, sigmay, mux, muy):
  z = np.zeros((n, n))
  for r in range(0, z.shape[0]):
    for c in range(0, z.shape[1]):
      x = c/(z.shape[1]-1)
      y = r/(z.shape[0]-1)
      z[r][c] = 1 / (2 * np.pi * sigmax * sigmay) * np.exp(-0.5 *
                                                           (np.power(x - mux, 2) / np.power(sigmax, 2) + (np.power(y - muy, 2) / np.power(sigmay, 2))))
  return z


matplotlib_init.matplotlib_init()
n = 101
xmin = 0
xmax = 1
ymin = 0
ymax = 1
sigmax = 0.2
sigmay = 0.1
mux = 0.5
muy = 0.5
z = Gaussian(n, sigmax, sigmay, mux, muy)
x, y = np.meshgrid(np.linspace(xmin, xmax, n), np.linspace(ymax, ymin, n))
fig = plt.figure()
ax = fig.gca(projection='3d')
surf = ax.plot_surface(
    x, y, z,
    cmap="jet",
    edgecolor='none',
    linewidth=0,
    antialiased=False,
    rstride=1,
    cstride=1)

ax.view_init(elev=25, azim=-130)
ax.set_xlim(xmin, xmax)
ax.set_ylim(ymin, ymax)
cbar = fig.colorbar(surf, shrink=0.7, aspect=15)
cbar.set_label(
    r"$\frac{1}{2\pi\sigma_x\sigma_y}\exp\left(-\frac{1}{2}\left(\frac{\left(x-\mu_x\right)^2}{\sigma_x^2}+\frac{\left(y-\mu_y\right)^2}{\sigma_y^2}\right)\right)$")

plt.xlabel("x")
plt.ylabel("y")
plt.tight_layout()
plt.show()
