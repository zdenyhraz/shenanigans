import sys
import getopt
import json
import os
import numpy as np
from astropy.io import fits
from PIL import Image


def GenerateFITSUrl(id):
  return "http://netdrms01.nispdc.nso.edu/cgi-bin/netdrms/drms_export.cgi?series=hmi__Ic_45s;record={}-{}".format(
      id, id)


def GetFITS(url):
  fimg = fits.open(url, cache=False)
  fimg.verify('silentfix')
  return fimg


def AlignFITSImage(img):
  return np.transpose(np.rot90(img))


def SaveFITSImage(fimg, path):
  img = AlignFITSImage(fimg[0].data.astype(np.int32))
  Image.fromarray(img).save("{}.png".format(path))


def SaveFITSHeader(fimg, path):
  # save image header as json
  i = 0
  hdr = {}
  for name in fimg[0].header:
    hdr[name] = fimg[0].header[i]
    i += 1

  open("{}.json".format(path), 'w').write(json.dumps(hdr, indent=2))


def DownloadAndSaveFITS(url, path):
  fimg = GetFITS(url)  # download the FITS
  SaveFITSImage(fimg, path)  # save aligned (rot90+transpose) FITS image as png
  SaveFITSHeader(fimg, path)  # save FITS header as json


def ParseArguments():
  try:
    options = {}
    opts, args = getopt.getopt(sys.argv[1:], "", ["name=", "outputdir=",
                               "idstart=", "idstep=", "idstride=", "idcount="])

    for opt, arg in opts:
      if opt == "--name":
        options["name"] = arg
      if opt == "--outputdir":
        options["outputdir"] = arg
      if opt == "--idstart":
        options["idstart"] = arg
      if opt == "--idstep":
        options["idstep"] = arg
      if opt == "--idstride":
        options["idstride"] = arg
      if opt == "--idcount":
        options["idcount"] = arg

    # required options
    options["name"]
    options["outputdir"]
    options["idstart"]
    options["idstep"]
    options["idstride"]
    options["idcount"]

    return options

  except Exception as error:
    print("Failed to process command line arguments {}: {} missing".format(sys.argv[1:], error))
    sys.exit(1)


if __name__ == "__main__":  # py .\script\fits_getdata.py --name "diffrot_month" --outputdir "data" --idstart 18982248 --idstep 1 --idstride 25 --idcount 4
  options = ParseArguments()
  idstart = int(options["idstart"])
  idstep = int(options["idstep"])
  idcount = int(options["idcount"])
  idstride = int(options["idstride"])
  dir = "{}/{}".format(options["outputdir"], options["name"])
  i = 0
  id = idstart

  if not os.path.isdir(dir):
    print("[Init] Creating {} directory ...".format(dir))
    os.makedirs(dir)

  while i < idcount:
    try:
      if i != 0:
        id += idstride - idstep if idstride != 0 and i % 2 == 0 else idstep
      i += 1

      url = GenerateFITSUrl(id)
      path = "{}/{}".format(dir, id)
      print("[{} / {}] Processing file {} ...".format(i, idcount, path))
      DownloadAndSaveFITS(url=url, path=path)
    except Exception as error:
      print("Failed to process file {}: {}".format(path, error))
      continue
