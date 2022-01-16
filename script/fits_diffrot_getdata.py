import numpy as np
from astropy.io import fits
from PIL import Image
import sys
import json

# call this like:
# py .\script\fits_diffrot_download.py "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record=18933122-18933122"

def DownloadAndSaveFITS(url):
  try:
    fimg = fits.open(url, cache=False)
    fimg.verify('silentfix')

    # file id is the 8-digit number at the end of the url
    filename = url[-8:]

    # save aligned (rot90+transpose) image as png
    Image.fromarray(np.transpose(np.rot90(fimg[0].data.astype(np.int32)))).save(filename+'.png')

    # save image header as json
    i=0
    hdr={}
    for name in fimg[0].header:
      hdr[name]=fimg[0].header[i]
      i+=1

    open(filename+'.json', 'w').write(json.dumps(hdr, indent=2))

  except Exception as error:
    print("Failed to process file {} - {}".format(url, error))
    sys.exit(1)
  except:
    print("Failed to process file {}".format(url))
    sys.exit(1)


DownloadAndSaveFITS(sys.argv[1])









