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

    const auto beginUB = mMap.upper_bound(keyBegin);
    const auto endUB = mMap.upper_bound(keyEnd);
    auto endVal = std::prev(endUB)->second; // copy val before erase

    // fix adjacent intervals with equal values

    // erase everything inbetween keyBegin & keyEnd (overwrite previous values in this interval)
    mMap.erase(beginUB, endUB);

    // insert keyBegin & keyEnd (overwrite existing elements)
    mMap.insert_or_assign(keyBegin, val);
    mMap.insert_or_assign(keyEnd, std::move(endVal));
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

    usize lookupfails = 0;
    usize adjacencyfails = 0;
    u8 prev = map[0];
    for (int key = 0; key < 256; ++key)
    {
      if (map[key] == values[key])
      {
        LOG_SUCCESS("IntervalMap[{}] = {} test OK", key, values[key]);
      }
      else
      {
        LOG_ERROR("IntervalMap[{}] = {} != {} test NOK", key, map[key], values[key]);
        ++lookupfails;
      }

      if (key > 0 and map[key] == prev)
      {
        LOG_ERROR("IntervalMap[{}] = {} && IntervalMap[{}] = {} test NOK", key - 1, prev, key, map[key]);
        ++adjacencyfails;
      }
      prev = map[key];
    }

    if (lookupfails == 0)
      LOG_SUCCESS("Lookup test passed! ({} fails)", lookupfails);
    else
      LOG_ERROR("Lookup test failed! ({} fails)", lookupfails);

    if (adjacencyfails == 0)
      LOG_SUCCESS("Adjacency test passed! ({} fails)", adjacencyfails);
    else
      LOG_ERROR("Adjacency test failed! ({} fails)", adjacencyfails);
  }
};
