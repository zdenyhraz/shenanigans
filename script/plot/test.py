import numpy as np
import matplotlib
import matplotlib.pyplot as plt

print("Matplotlib was originally using the", matplotlib.get_backend(), "backend")
matplotlib.use('GTK3Agg')
print("Matplotlib is now using the", matplotlib.get_backend(), "backend")

x = np.linspace(0, 2*np.pi, 101)
y = np.sin(x)

plt.plot(x, y)
plt.show()
