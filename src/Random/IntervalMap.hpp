#pragma once
#include <map>
#include "IntervalMap2.hpp"
#include "interval_map.hpp"

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

    const auto prevMap = mMap; // debug copy
    const auto beginIt = mMap.lower_bound(keyBegin);
    const auto endIt = mMap.upper_bound(keyEnd);
    auto endVal = std::prev(endIt)->second; // copy val before erase

    // do not insert if adjacent keys already have the same value to avoid redundant keys
    // keyBegin: always insert (update) if the keyBegin is the lowest key
    // keyBegin: do not insert if previous key has the same value as val (backward redundancy with previous key)
    // keyEnd: do not insert if next key has the same value as endVal (forward redundancy with following key)
    // keyEnd: do not insert if endVal == val (backward redundancy with keyBegin)
    const bool beginIns = beginIt == mMap.begin() ? true : std::prev(beginIt)->second != val;
    const bool endIns = (endIt == mMap.end() ? true : endIt->second != endVal) and endVal != val;

    // erase everything inbetween keyBegin & keyEnd (overwrite previous values in this interval)
    mMap.erase(beginIt, endIt);

    // conditionally insert/assign keyBegin & keyEnd
    if (beginIns)
      mMap.insert_or_assign(keyBegin, val);
    if (endIns)
      mMap.insert_or_assign(keyEnd, std::move(endVal));

    InternalTest(keyBegin, keyEnd, val, prevMap);
  }

  // look-up of the value associated with key
  const Val& operator[](const Key& key) const { return (--mMap.upper_bound(key))->second; }

  const std::map<Key, Val>& GetMap() const { return mMap; }

  void InternalTest(const Key& keyBegin, const Key& keyEnd, const Val& val, const std::map<Key, Val>& prevMap)
  {
    auto prevVal = mMap.begin()->second;
    for (const auto& [currkey, currval] : mMap)
    {
      if (currkey > 0 and currval == prevVal)
      {
        for (const auto& [keyy, vall] : prevMap)
          LOG_DEBUG("IntervalMap Before[{}] = {}", keyy, vall);
        LOG_DEBUG("> Inserting interval [{}, {}) = {}", keyBegin, keyEnd, val);
        for (const auto& [keyy, vall] : mMap)
          LOG_DEBUG("IntervalMap After[{}] = {}", keyy, vall);

        throw std::runtime_error("Adjacency test failed");
      }
      prevVal = currval;
    }
  }

  usize Size() const { return mMap.size(); }

  static void Test()
  {
    LOG_FUNCTION;
    const u8 valBegin = 123;
    IntervalMap<u8, u8> map(valBegin);
    interval_map2<u8, u8> map2(valBegin);
    interval_map<u8, u8> map_main(valBegin);
    std::array<u8, 256> values;
    std::fill(values.begin(), values.end(), valBegin);

    const int nintervals = 1e2;
    for (int i = 0; i < nintervals; ++i)
    {
      const u8 begin = rand() % 256;
      const u8 end = rand() % 256;
      const u8 val = rand() % 4;
      for (int key = begin; key < end; ++key)
        values[key] = val;
      map.Assign(begin, end, val);
      map2.assign(begin, end, val);
      map_main.assign(begin, end, val);
    }

    usize lookupfails = 0;
    for (int key = 0; key < 256; ++key)
    {
      if (map_main[key] == values[key])
      {
        // LOG_SUCCESS("IntervalMap[{}] = {} test OK", key, values[key]);
      }
      else
      {
        LOG_ERROR("interval_map[{}] = {} != {} test NOK", key, map_main[key], values[key]);
        ++lookupfails;
      }
    }

    usize adjacencyfails = 0;
    u8 prevKey = 0;
    u8 prevVal = valBegin;
    for (const auto& [key, val] : map_main.get_map())
    {
      if (val == prevVal)
      {
        LOG_ERROR("interval_map[{}/{}] = {} test NOK", key, prevKey, val);
        ++adjacencyfails;
      }
      prevKey = key;
      prevVal = val;
    }

    for (const auto& [key, value] : map_main.get_map())
      LOG_DEBUG("interval_map[{}] = {}", key, value);

    if (lookupfails == 0)
      LOG_SUCCESS("Lookup test passed!");
    else
      LOG_ERROR("Lookup test failed! ({} fails)", lookupfails);

    if (adjacencyfails == 0)
      LOG_SUCCESS("Adjacency test passed!");
    else
      LOG_ERROR("Adjacency test failed! ({} fails)", adjacencyfails);

    if (map.GetMap() == map2.get_map())
      LOG_SUCCESS("MyIntervalMap/interval_map2 internals passed! ({})", map.Size());
    else
      LOG_ERROR("MyIntervalMap/interval_map2 internals failed! ({}/{})", map.Size(), map2.size());

    if (const auto& [key, value] = *map_main.get_map().begin(); value != valBegin)
      LOG_SUCCESS("First element test passed! ({} != {})", value, valBegin);
    else
      LOG_ERROR("First element test failed!  ({} == {})", value, valBegin);
  }
};
