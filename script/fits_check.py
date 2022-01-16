import numpy as np
from astropy.io import fits
from matplotlib import pyplot as plt
from PIL import Image

hdul = fits.open('resources/FITS/HMI.fits', cache=False)  # open a FITS file, URL can also be used, do not cache (throw away download fits files)
hdul.verify('fix')
print(hdul)

print("---------- Header ----------")
hdr = hdul[0].header
i=0
for line in hdr:
  print("{}: {}".format(line,hdr[i]))
  i+=1


print("---------- Data ----------")
fitsdata = np.transpose(np.rot90(hdul[0].data))  # assume the first extension is an image
print(np.sqrt(fitsdata.size))
print("Value at [1000,1000]: {}".format(fitsdata[1000, 1000]))
print("Value at [1001,1001]: {}".format(fitsdata[1001, 1001]))
print("Value at [1002,1002]: {}".format(fitsdata[1002, 1002]))

Image.fromarray(fitsdata.astype(np.int32)).save('resources/FITS/HMI.png') # save as png

# check if saved png is the same as fits
imdata = np.asarray(Image.open('resources/FITS/HMI.png'))
print("Arrays equal: {}".format(np.array_equal(fitsdata,imdata)))
print("Array types: {} / {}".format(fitsdata.dtype,imdata.dtype))

print("Value at [1000,1000]: {} / {}".format(fitsdata[1000, 1000],imdata[1000, 1000]))
print("Value at [1001,1001]: {} / {}".format(fitsdata[1001, 1001],imdata[1001, 1001]))
print("Value at [1002,1002]: {} / {}".format(fitsdata[1002, 1002],imdata[1002, 1002]))

plt.figure()
plt.imshow(fitsdata)
plt.colorbar()
plt.show()

plt.figure()
plt.imshow(imdata)
plt.colorbar()
plt.show()

plt.figure()
plt.imshow(np.abs(imdata-fitsdata))
plt.colorbar()
plt.show()



if False:
  for r in range(0, fitsdata.shape[0]):
    for c in range(0, fitsdata.shape[1]):
      if not np.isnan(fitsdata[r,c]):
        if fitsdata[r,c] != imdata[r,c]:
          print("{} != {}".format(fitsdata[r,c], imdata[r,c]))



