import pyxtf
import numpy as np
import matplotlib.pyplot as plt


file = r'M:\Work\shenanigans\data\debug\ObjectDetection\xtf\sasi-S-upper-20221102-144815-l38.xtf'
(header, packets) = pyxtf.xtf_read(file)
fig, axs = plt.subplots(1, 3, figsize=(33, 8))
fig.canvas.manager.set_window_title(file)

# The function concatenate_channels concatenates all the individual pings for a channel, and returns it as a dense numpy array
image = pyxtf.concatenate_channel(packets[pyxtf.XTFHeaderType.sonar], file_header=header, channel=0, weighted=True)

plot = axs[0].imshow(image, cmap='gray', aspect='auto')
axs[0].title.set_text("raw")
plt.colorbar(plot)

upper_limit = 2 ** 14
image.clip(0, upper_limit-1, out=image)  # Clip to range (max cannot be used due to outliers) More robust methods are possible (through histograms / statistical outlier removal)

plot = axs[1].imshow(image, cmap='gray', aspect='auto')
axs[1].title.set_text(f"clip to {upper_limit}")
plt.colorbar(plot)

logeps = 0.0001
image = np.log10(image + logeps)  # The sonar data is logarithmic (dB), add small value to avoid log10(0)
image.clip(0, np.log10(upper_limit), out=image)

plot = axs[2].imshow(image, cmap='gray', aspect='auto')
axs[2].title.set_text(f"clip + log10 (eps={logeps})")
plt.colorbar(plot)

plt.tight_layout()
plt.show()
