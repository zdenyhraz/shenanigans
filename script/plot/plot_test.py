import numpy as np
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use('Qt5Agg')
print("Matplotlib is using the", matplotlib.get_backend(), "backend")

x = np.linspace(0, 2*np.pi, 101)
y = np.sin(x)

plt.plot(x, y)
plt.show()
