import pyxtf
import numpy as np
import os
from PIL import Image
from script.log import log

directory = r'M:\Work\shenanigans\data\debug\ObjectDetection\xtf'
for filename in os.listdir(directory):
    file = os.path.join(directory, filename)
    stem = os.path.basename(file).split('.')[0]
    savepath = f'M:\Work\shenanigans\data\debug\ObjectDetection\{stem}.tif'
    if not file.endswith('.xtf'):
        continue

    log.debug(f"Processing {savepath}")
    (header, packets) = pyxtf.xtf_read(file)

    # The function concatenate_channels concatenates all the individual pings for a channel, and returns it as a dense numpy array
    image = pyxtf.concatenate_channel(packets[pyxtf.XTFHeaderType.sonar], file_header=header, channel=0, weighted=True)

    upper_limit = 2 ** 14
    image.clip(0, upper_limit-1, out=image)  # Clip to range (max cannot be used due to outliers) More robust methods are possible (through histograms / statistical outlier removal)

    logeps = 0.0001
    image = np.log10(image + logeps)  # The sonar data is logarithmic (dB), add small value to avoid log10(0)
    image.clip(0, np.log10(upper_limit), out=image)

    image = (image-np.min(image))/(np.max(image)-np.min(image))  # minmax normalize
    image = image*255
    Image.fromarray(image.astype(np.uint8)).save(savepath)
