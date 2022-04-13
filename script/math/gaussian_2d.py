import numpy as np
import matplotlib.pyplot as plt
import matplotlib_init


def Gaussian(sigmax, sigmay, mux, muy):
  n = 101
  z = np.zeros((n, n))
  for r in range(0, z.shape[0]):
    for c in range(0, z.shape[1]):
      x = c/(z.shape[1]-1)
      y = r/(z.shape[0]-1)
      z[r][c] = 1 / (2 * np.pi * sigmax * sigmay) * np.exp(-0.5 *
                                                           (np.power(x - mux, 2) / np.power(sigmax, 2) + (np.power(y - muy, 2) / np.power(sigmay, 2))))
  return z


matplotlib_init.matplotlib_init()
xmin = 0
xmax = 1
ymin = 0
ymax = 1
sigmax = 0.1
sigmay = 0.2
mux = 0.6
muy = 0.4

im = plt.imshow(
    Gaussian(sigmax, sigmay, mux, muy),
    extent=[xmin, xmax, ymin, ymax],
    interpolation='bilinear', aspect='auto')
cbar = plt.colorbar(im)
cbar.set_label(
    r"$\frac{1}{2\pi\sigma_x\sigma_y}\exp\left(-\frac{1}{2}\left(\frac{\left(x-\mu_x\right)^2}{\sigma_x^2}+\frac{\left(y-\mu_y\right)^2}{\sigma_y^2}\right)\right)$")
plt.set_cmap("jet")
plt.xlabel("x")
plt.ylabel("y")
plt.tight_layout()
plt.show()
