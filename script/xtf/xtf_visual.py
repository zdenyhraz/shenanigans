import pyxtf
import numpy as np
import matplotlib.pyplot as plt
import os

if True:
  file = r'M:\Work\shenanigans\data\debug\ObjectDetection\xtf\sasi-S-upper-20221102-144815-l38.xtf'

  (header, packets) = pyxtf.xtf_read(file)

  # The function concatenate_channels concatenates all the individual pings for a channel, and returns it as a dense numpy array
  image = pyxtf.concatenate_channel(packets[pyxtf.XTFHeaderType.sonar], file_header=header, channel=0, weighted=False)
  print("image ", file, " shape: ",image.shape)

  upper_limit = 2 ** 14
  image.clip(0, upper_limit-1, out=image) # Clip to range (max cannot be used due to outliers) More robust methods are possible (through histograms / statistical outlier removal)
  image = np.log10(image + 0.0001) # The sonar data is logarithmic (dB), add small value to avoid log10(0)

  fig, (ax1, ax2,ax3) = plt.subplots(1,3, figsize=(30,10))
  fig.canvas.manager.set_window_title(file)
  row_sums = image.sum(axis=1)
  image_normalized = image / row_sums[:, np.newaxis]

  # row_sums = image.sum(axis=0)
  # image = image / row_sums[np.newaxis,:]

  # plt.imshow(image, cmap='gray', vmin=0, vmax=np.log10(upper_limit), aspect='auto') # waterfall-view
  ax1.imshow(image, cmap='gray', aspect='auto') # waterfall-view
  ax2.plot(row_sums[::-1],np.linspace(0,row_sums.shape[0]-1,num=row_sums.shape[0]))
  ax3.imshow(image_normalized, cmap='gray', aspect='auto') # waterfall-view

  plt.tight_layout()
  plt.show()
else:
  directory = r'M:\Work\shenanigans\data\debug\ObjectDetection\xtf'
  fig = plt.figure()
  fig.set_figwidth(fig.get_figwidth()*2)
  fig.set_figheight(fig.get_figheight()*2)

  for filename in os.listdir(directory):
    file = os.path.join(directory, filename)
    if not file.endswith('.xtf'):
      continue

    (header, packets) = pyxtf.xtf_read(file)

    # The function concatenate_channels concatenates all the individual pings for a channel, and returns it as a dense numpy array
    image = pyxtf.concatenate_channel(packets[pyxtf.XTFHeaderType.sonar], file_header=header, channel=0, weighted=False)

    upper_limit = 2 ** 14
    image.clip(0, upper_limit-1, out=image) # Clip to range (max cannot be used due to outliers) More robust methods are possible (through histograms / statistical outlier removal)
    image = np.log10(image + 0.0001) # The sonar data is logarithmic (dB), add small value to avoid log10(0)
    print("image ", file, " shape: ",image.shape)

    plt.clf()
    plt.imshow(image, cmap='gray', vmin=0, vmax=np.log10(upper_limit), aspect='auto') # waterfall-view
    plt.tight_layout()
    fig.canvas.manager.set_window_title(file)
    plt.draw()
    plt.pause(1e-9)

