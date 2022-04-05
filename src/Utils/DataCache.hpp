#pragma once

template <typename Key, typename Value>
class DataCache
{
public:
  using GetDataFunction = std::function<Value(const Key&)>;
  explicit DataCache(const GetDataFunction& getDataFunction) : mGetDataFunction(getDataFunction) {}

  void SetGetDataFunction(const GetDataFunction& getDataFunction) { mGetDataFunction = getDataFunction; }

  const Value Get(const Key& key)
  {
    PROFILE_FUNCTION;
    std::scoped_lock lock(mMutex);

    if (mData.contains(key))
      return mData[key];

    if (mData.size() < mCapacity)
    {
      PROFILE_SCOPE(GetData);
      mData[key] = mGetDataFunction(key);
      LOG_DEBUG("ImageCache::Get added {} to cache (capacity {}/{})", key, mData.size(), mCapacity);
      return mData[key];
    }

    return mGetDataFunction(key);
  }

  void Clear()
  {
    PROFILE_FUNCTION;
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
    PROFILE_FUNCTION;
    std::scoped_lock lock(mMutex);
    mData.reserve(capacity);
    mCapacity = std::max(mCapacity, capacity);
  }

private:
  GetDataFunction mGetDataFunction;
  std::unordered_map<Key, Value> mData;
  usize mCapacity = 100;
  std::mutex mMutex;
};
