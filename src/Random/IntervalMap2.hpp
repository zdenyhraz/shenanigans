#pragma once
#include <map>

template <typename K, typename V>
class interval_map2
{
protected:
  std::map<K, V> m_map;

public:
  // constructor associates whole range of K with val by inserting (K_min, val)
  // into the map
  interval_map2(V const& val) { m_map.insert(m_map.end(), std::make_pair(std::numeric_limits<K>::lowest(), val)); }

  // Assign value val to interval [keyBegin, keyEnd).
  // Overwrite previous values in this interval.
  // Conforming to the C++ Standard Library conventions, the interval
  // includes keyBegin, but excludes keyEnd.
  // If !( keyBegin < keyEnd ), this designates an empty interval,
  // and assign must do nothing.
  void assign(K const& keyBegin, K const& keyEnd, V const& val)
  {
    if (!(keyBegin < keyEnd))
      return;
    auto nextInterval = --m_map.upper_bound(keyEnd);
    auto inserted1 = m_map.end();
    auto inserted2 = m_map.end();
    // assert(nextInterval != m_map.end());
    if (nextInterval->second == val)
      ++nextInterval;
    else if (nextInterval->first < keyEnd)
    {
      const V& nextValue = nextInterval->second;
      ++nextInterval;
      inserted1 = nextInterval = m_map.emplace_hint(nextInterval, keyEnd, nextValue);
    }
    try
    {
      auto prevInterval = nextInterval;
      --prevInterval;
      // assert(prevInterval != m_map.end());
      if (keyBegin < prevInterval->first)
        prevInterval = --m_map.upper_bound(keyBegin);
      // assert(prevInterval != m_map.end());
      if (!(prevInterval->second == val))
      {
        if (prevInterval->first < keyBegin)
        {
          ++prevInterval;
          inserted2 = prevInterval = m_map.emplace_hint(prevInterval, keyBegin, val);
        }
        else
        {
          prevInterval->second = val;
          if (prevInterval != m_map.begin() && !((--prevInterval)->second == val))
          {
            ++prevInterval;
          }
        }
      }
      // assert(prevInterval != m_map.end());
      m_map.erase(++prevInterval, nextInterval);
    }
    catch (...)
    {
      if (inserted1 != m_map.end())
        m_map.erase(inserted1);
      if (inserted2 != m_map.end())
        m_map.erase(inserted2);
      throw;
    }
  }

  // look-up of the value associated with key
  V const& operator[](K const& key) const { return (--m_map.upper_bound(key))->second; }

  const std::map<K, V>& get_map() const { return m_map; }

  usize size() const { return m_map.size(); }
};
