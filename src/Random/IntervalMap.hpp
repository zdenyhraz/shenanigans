#pragma once
#include <map>

template <typename Key, typename Val>
class IntervalMap
{
protected:
  std::map<Key, Val> mMap;

public:
  // constructor associates whole range of K with val by inserting (K_min, val)
  // into the map
  IntervalMap(const Val& val) { mMap.insert(mMap.end(), std::make_pair(std::numeric_limits<Key>::lowest(), val)); }

  // Assign value val to interval [keyBegin, keyEnd).
  // Overwrite previous values in this interval.
  // Conforming to the C++ Standard Library conventions, the interval
  // includes keyBegin, but excludes keyEnd.
  // If !( keyBegin < keyEnd ), this designates an empty interval,
  // and assign must do nothing.
  void Assign(const Key& keyBegin, const Key& keyEnd, const Val& val)
  {
    if (not(keyBegin < keyEnd))
      return;

    const auto endVal = (--mMap.upper_bound(keyEnd))->second;

    // erase everything inbetween
    mMap.erase(mMap.upper_bound(keyBegin), mMap.upper_bound(keyEnd));

    // insert keyBegin & keyEnd, overwrite if already exists
    mMap[keyBegin] = val;
    mMap[keyEnd] = endVal;
  }

  // look-up of the value associated with key
  const Val& operator[](const Key& key) const { return (--mMap.upper_bound(key))->second; }

  void Print() const
  {
    for (const auto& [key, val] : mMap)
      LOG_INFO("IntervalMap[{}] = {}", key, val);
  }

  static void Test()
  {
    LOG_FUNCTION;
    IntervalMap<u8, u8> map(0);
    std::array<u8, 256> values{};

    const int n = 1e7;
    for (int i = 0; i < n; ++i)
    {
      const u8 begin = rand() % 256;
      const u8 end = rand() % 256;
      const u8 val = rand() % 256;
      for (int key = begin; key < end; ++key)
        values[key] = val;
      map.Assign(begin, end, val);

      if (n <= 1000)
        LOG_DEBUG("[{}/{}] Adding interval [{}, {}) = {}", i + 1, n, begin, end, val);
    }

    usize fails = 0;
    for (int key = 0; key < 256; ++key)
    {
      if (map[key] == values[key])
      {
        LOG_SUCCESS("IntervalMap[{}] = {} test OK", key, values[key]);
      }
      else
      {
        LOG_ERROR("IntervalMap[{}] = {} != {} test NOK", key, map[key], values[key]);
        ++fails;
      }
    }
    if (fails == 0)
      LOG_SUCCESS("Test passed! ({} fails)", fails);
    else
      LOG_ERROR("Test failed! ({} fails)", fails);
  }
};
