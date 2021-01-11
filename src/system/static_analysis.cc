/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include <cassert>

#include "tchecker/system/static_analysis.hh"
#include "tchecker/system/synchronization.hh"

namespace tchecker {
  
  /* process_events_map_t */
  
  process_events_map_t::process_events_map_t(tchecker::process_id_t proc_count)
  : _map(proc_count)
  {}
  
  
  void process_events_map_t::insert(tchecker::process_id_t pid, tchecker::event_id_t event_id)
  {
    assert(pid < _map.capacity());
    _map[pid].insert(event_id);
  }
  

  bool process_events_map_t::contains(tchecker::process_id_t pid, tchecker::event_id_t event_id)
  {
    if (pid >= _map.size())
      return false;
    return (_map[pid].find(event_id) != _map[pid].end());
  }
  
  
  
  
  tchecker::process_events_map_t
  weakly_synchronized_events(tchecker::range_t<tchecker::const_sync_iterator_t> const & range, tchecker::process_id_t proc_count)
  {
    tchecker::process_events_map_t weak_sync_map(proc_count);
    for (tchecker::synchronization_t const & sync : range) {
      for (tchecker::sync_constraint_t const & sc : sync.synchronization_constraints())
        if (sc.strength() == tchecker::SYNC_WEAK)
          weak_sync_map.insert(sc.pid(), sc.event_id());
    }
    return weak_sync_map;
  }
  
  
  
  
  /* location_next_syncs_t */
  
  location_next_syncs_t::location_next_syncs_t(tchecker::loc_id_t locations_count,
                                               tchecker::sync_id_t syncs_count)
  : _next_syncs_map{
    {locations_count, boost::dynamic_bitset<>(syncs_count, 0)},
    {locations_count, boost::dynamic_bitset<>(syncs_count, 0)}},
  _syncs_count(syncs_count)
  {}
  
  
  void location_next_syncs_t::add_next_sync(tchecker::sync_id_t sync_id,
                                            tchecker::loc_id_t loc_id,
                                            enum next_type_t next_type)
  {
    _next_syncs_map[next_type][loc_id][sync_id] = 1;
  }
  
  
  boost::dynamic_bitset<> const &
  location_next_syncs_t::next_syncs(tchecker::loc_id_t loc_id, enum next_type_t next_type) const
  {
    return _next_syncs_map[next_type][loc_id];
  }
  

  boost::dynamic_bitset<> &
  location_next_syncs_t::next_syncs(tchecker::loc_id_t loc_id, enum next_type_t next_type)
  {
    return _next_syncs_map[next_type][loc_id];
  }
  
  
  std::size_t location_next_syncs_t::next_sync_size() const
  {
    return _syncs_count;
  }
  

  
  /* pure_local_map_t */
  
  pure_local_map_t::pure_local_map_t(tchecker::loc_id_t loc_count, bool status)
  : _map(loc_count)
  {
    if (status)
      _map.set();
    else
      _map.reset();
  }


  bool pure_local_map_t::is_pure_local(tchecker::loc_id_t id) const
  {
    assert(id < _map.size());
    return _map[id];
  }


  void pure_local_map_t::set_pure_local(tchecker::loc_id_t id, bool status)
  {
    assert(id < _map.size());
    _map[id] = status;
  }

} // end of namespace tchecker
