class ImageCache
{
public:
  const cv::Mat Get(const std::string& path)
  {
    std::scoped_lock lock(mMutex);

    if (mData.contains(path))
    {
      LOG_TRACE("ImageCache::Get cache hit for {}", path);
      return mData[path];
    }

    LOG_TRACE("ImageCache::Get cache miss for {}", path);

    if (mData.size() < mCapacity)
    {
      mData[path] = cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
      LOG_DEBUG("ImageCache::Get added {} to cache (capacity {}/{})", path, mData.size(), mCapacity);
      return mData[path];
    }

    return cv::imread(path, cv::IMREAD_GRAYSCALE | cv::IMREAD_ANYDEPTH);
  }

  void Clear()
  {
    std::scoped_lock lock(mMutex);
    mData.clear();
  }

  void SetCapacity(usize capacity) { mCapacity = capacity; }

private:
  usize mCapacity = 50;
  std::unordered_map<std::string, cv::Mat> mData;
  std::mutex mMutex;
};