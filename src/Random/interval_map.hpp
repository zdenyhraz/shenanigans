#include <map>
template <typename K, typename V>
class interval_map
{
  friend void IntervalMapTest();
  V m_valBegin;
  std::map<K, V> m_map;

public:
  // constructor associates whole range of K with val
  interval_map(V const& val) : m_valBegin(val) {}

  // Assign value val to interval [keyBegin, keyEnd).
  // Overwrite previous values in this interval.
  // Conforming to the C++ Standard Library conventions, the interval
  // includes keyBegin, but excludes keyEnd.
  // If !( keyBegin < keyEnd ), this designates an empty interval,
  // and assign must do nothing.
  void assign_orig(K const& keyBegin, K const& keyEnd, V const& val)
  {
    if (not(keyBegin < keyEnd))
      return;

    const auto beginIt = m_map.lower_bound(keyBegin);
    const auto endIt = m_map.upper_bound(keyEnd);

    // copy val before erase (if m_map.empty() then begin() == end(), so dereferencing is safe here)
    auto endVal = endIt == m_map.begin() ? m_valBegin : std::prev(endIt)->second;

    // do not insert if adjacent keys already have the same value to avoid redundant keys
    // keyBegin: if the keyBegin is the lowest key, insert only if val != m_valBegin
    // keyBegin: do not insert if previous key has the same value as val (backward redundancy with previous key)
    // keyEnd: do not insert if next key has the same value as endVal (forward redundancy with following key)
    // keyEnd: do not insert if endVal == val (backward redundancy with keyBegin)
    const bool beginIns = beginIt == m_map.begin() ? val != m_valBegin : val != std::prev(beginIt)->second;
    const bool endIns = (endIt == m_map.end() ? true : endIt->second != endVal) and endVal != val;

    // erase everything inbetween keyBegin & keyEnd (overwrite previous values in this interval)
    m_map.erase(beginIt, endIt);

    // conditionally insert/assign keyBegin & keyEnd
    if (beginIns)
      m_map.insert_or_assign(keyBegin, val);
    if (endIns)
      m_map.insert_or_assign(keyEnd, std::move(endVal));
  }

  void assign(K const& keyBegin, K const& keyEnd, V const& val)
  {
    if (not(keyBegin < keyEnd))
      return;

    const auto beginIt = m_map.upper_bound(keyBegin);
    auto it = beginIt;
    V valEnd = beginIt == m_map.begin() ? m_valBegin : std::prev(beginIt)->second;

    if (beginIt == m_map.begin() ? val != m_valBegin : val != std::prev(beginIt)->second)
    {
      it = m_map.insert_or_assign(beginIt, keyBegin, val);
      it = (it != m_map.begin() and (it->second == std::prev(it)->second)) ? m_map.erase(it) : std::next(it);
    }

    while (it != m_map.end())
    {
      if (it->first < keyEnd)
      {
        valEnd = it->second;
        it = m_map.erase(it);
      }
      else
      {
        break;
      }
    }

    if ((it == m_map.end() ? true : valEnd != it->second) and valEnd != val)
    {
      it = m_map.emplace_hint(it, keyEnd, valEnd);
      if (it != m_map.begin() and (it->second == std::prev(it)->second))
        m_map.erase(it);
    }

    if (false)
    {
      LOG_DEBUG("interval_map after [{},{})={}", keyBegin, keyEnd, val);
      for (const auto& [key, value] : m_map)
        LOG_DEBUG("interval_map[{}] = {}", key, value);
    }
  }

  // look-up of the value associated with key
  V const& operator[](K const& key) const
  {
    auto it = m_map.upper_bound(key);
    if (it == m_map.begin())
    {
      return m_valBegin;
    }
    else
    {
      return (--it)->second;
    }
  }

  const std::map<K, V>& get_map() const { return m_map; }
};

// Many solutions we receive are incorrect. Consider using a randomized test
// to discover the cases that your implementation does not handle correctly.
// We recommend to implement a test function that tests the functionality of
// the interval_map, for example using a map of int intervals to char.
