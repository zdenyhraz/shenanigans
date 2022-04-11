#include <fftw3.h>

namespace fftw
{
template <typename T>
class vector
{
  T* mData;
  usize mSize;

public:
  vector(usize n)
  {
    if constexpr (std::is_same_v<T, f32>)
      mData = fftwf_alloc_real(n);
    if constexpr (std::is_same_v<T, fftwf_complex>)
      mData = fftwf_alloc_complex(n);

    if constexpr (std::is_same_v<T, f64>)
      mData = fftw_alloc_real(n);
    if constexpr (std::is_same_v<T, fftw_complex>)
      mData = fftw_alloc_complex(n);

    mSize = n;
  }

  ~vector()
  {
    if constexpr (std::is_same_v<T, f32> or std::is_same_v<T, fftwf_complex>)
      fftwf_free(mData);
    if constexpr (std::is_same_v<T, f64> or std::is_same_v<T, fftw_complex>)
      fftw_free(mData);
  }

  vector(const vector& other) = delete;

  T* data() const { return mData; }

  usize size() const { return mSize; }
};
}
