import numpy as np
from astropy.io import fits
from PIL import Image
import sys

# call this like:
# py .\script\fits_diffrot_download.py "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122"

def DownloadAndSaveFITS(url):
  try:
    hdul = fits.open(url, cache=False, output_verify="ignore")

    # save aligned image as png
    Image.fromarray(np.transpose(np.rot90(hdul[0].data.astype(np.int32)))).save(url[-8:]+".png")

    # save image header
    #hdul[0].header

  except Exception as error:
    print("Failed to download file {} - {}".format(url, error))
    sys.exit(1)
  except:
    print("Failed to download file {}".format(url))
    sys.exit(1)


DownloadAndSaveFITS(sys.argv[1])









