import numpy as np
import matplotlib.pyplot as plt
import matplotlib_config


def Gaussian(x, sigma, mu):
  return 1/(sigma*np.sqrt(2*np.pi))*np.exp(-0.5*np.power(x-mu, 2)/np.power(sigma, 2))


matplotlib_config.init()
n = 1001
x = np.linspace(0, 1, n)
linewidth = 3

plt.plot(x, Gaussian(x, 0.10, 0.5), linewidth=linewidth, label="$\sigma=0.10$, $\mu=0.5$")
plt.plot(x, Gaussian(x, 0.07, 0.5), linewidth=linewidth, label="$\sigma=0.07$, $\mu=0.5$")
plt.plot(x, Gaussian(x, 0.05, 0.5), linewidth=linewidth, label="$\sigma=0.05$, $\mu=0.5$")
plt.plot(x, Gaussian(x, 0.07, 0.2), linewidth=linewidth, label="$\sigma=0.07$, $\mu=0.2$")
plt.plot(x, Gaussian(x, 0.05, 0.7), linewidth=linewidth, label="$\sigma=0.05$, $\mu=0.7$")
plt.ylabel(r"$\frac{1}{\sigma\sqrt{2\pi}}\exp\left(-\frac{1}{2}\frac{\left(x-\mu\right)^2}{\sigma^2}\right)$")
plt.xlabel("x")
plt.legend()
plt.tight_layout()
plt.show()
