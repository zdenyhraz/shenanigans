import sys
import getopt
import json
import os
import datetime
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


def SaveDataStatistics(stats, dir):
  statistics_path = "{}/statistics.json".format(dir)
  print("[Finished] Writing data statistics to {} ...".format(statistics_path))
  open(statistics_path, 'w').write(json.dumps(stats, indent=2, default=str))


def ParseArguments():
  try:
    parameters = {}
    opts, args = getopt.getopt(sys.argv[1:], "", ["name=", "outputdir=",
                               "idstart=", "idstep=", "idstride=", "idcount="])

    for opt, arg in opts:
      if opt == "--name":
        parameters["name"] = arg
      if opt == "--outputdir":
        parameters["outputdir"] = arg
      if opt == "--idstart":
        parameters["idstart"] = arg
      if opt == "--idstep":
        parameters["idstep"] = arg
      if opt == "--idstride":
        parameters["idstride"] = arg
      if opt == "--idcount":
        parameters["idcount"] = arg

    # required parameters
    parameters["name"]
    parameters["outputdir"]
    parameters["idstart"]
    parameters["idstep"]
    parameters["idstride"]
    parameters["idcount"]

    return parameters

  except Exception as error:
    print("Failed to process command line arguments {}: {} missing".format(sys.argv[1:], error))
    sys.exit(1)


if __name__ == "__main__":  # py .\script\fits_getdata.py --name "diffrot_month_5000" --outputdir "data" --idstart 18933122 --idstep 1 --idstride 25 --idcount 5000
  parameters = ParseArguments()
  stats = {"parameters": parameters}
  dir = "{}/{}".format(parameters["outputdir"], parameters["name"])
  id = int(parameters["idstart"])
  missing = []
  i = 0

  stats["download_start"] = datetime.datetime.now()
  if not os.path.isdir(dir):
    print("[Init] Creating {} directory ...".format(dir))
    os.makedirs(dir)

  while i < int(parameters["idcount"]):
    try:
      if i != 0:
        id += int(parameters["idstride"]) - int(parameters["idstep"]
                                                ) if int(parameters["idstride"]) != 0 and i % 2 == 0 else int(parameters["idstep"])
      i += 1

      path = "{}/{}".format(dir, id)
      if os.path.exists("{}.png".format(path)) and os.path.exists("{}.json".format(path)):
        print("[{:.1f}%: {} / {}] File {} already exists, skipping ...".format(float(i)/int(parameters["idcount"]*100),
                                                                               i, int(parameters["idcount"]), path))
        continue

      url = GenerateFITSUrl(id)
      print("[{:.1f}%: {} / {}] Processing file {} ...".format(float(i)/int(parameters["idcount"]*100),
            i, int(parameters["idcount"]), path))
      DownloadAndSaveFITS(url=url, path=path)

    except Exception as error:
      print("Failed to process file {}: {}".format(path, error))
      missing.append(id)
      continue

  stats["download_end"] = datetime.datetime.now()
  stats['missing'] = missing
  SaveDataStatistics(stats, dir)
