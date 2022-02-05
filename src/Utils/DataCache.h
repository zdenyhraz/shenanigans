template <typename Key, typename Value>
class DataCache
{
public:
  using GetDataFunction = std::function<Value(const Key&)>;
  DataCache(const GetDataFunction& getDataFunction) : mGetDataFunction(getDataFunction) {}

  const Value Get(const Key& key)
  {
    PROFILE_EVENT();
    std::scoped_lock lock(mMutex);

    if (mData.contains(key))
    {
      LOG_TRACE("ImageCache::Get cache hit for {}", key);
      return mData[key];
    }

    LOG_TRACE("ImageCache::Get cache miss for {}", key);

    if (mData.size() < mCapacity)
    {
      mData[key] = mGetDataFunction(key);
      LOG_DEBUG("ImageCache::Get added {} to cache (capacity {}/{})", key, mData.size(), mCapacity);
      return mData[key];
    }

    return mGetDataFunction(key);
  }

  void Clear()
  {
    std::scoped_lock lock(mMutex);
    mData.clear();
  }

  void SetCapacity(usize capacity)
  {
    std::scoped_lock lock(mMutex);
    mCapacity = capacity;
  }

  void Reserve(usize capacity)
  {
    PROFILE_EVENT();
    std::scoped_lock lock(mMutex);
    mData.reserve(capacity);
    mCapacity = std::max(mCapacity, capacity);
  }

private:
  const GetDataFunction mGetDataFunction;
  std::unordered_map<Key, Value> mData;
  usize mCapacity = 100;
  std::mutex mMutex;
};
