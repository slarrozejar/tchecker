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
  
  
  
  /* location_sync_flag_t */

   location_sync_flag_t::location_sync_flag_t(tchecker::loc_id_t locations_count)
  : _flags(locations_count, 0)
  {}


  void location_sync_flag_t::sync(tchecker::loc_id_t loc_id)
  {
    assert(loc_id < _flags.size());
    _flags[loc_id] = 1;
  }
  

  bool location_sync_flag_t::has_synchronized_event(tchecker::loc_id_t loc_id) const
  {
    assert(loc_id < _flags.size());
    return (_flags[loc_id] == 1);
  }
  
} // end of namespace tchecker
