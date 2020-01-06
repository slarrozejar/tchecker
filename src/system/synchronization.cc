/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include "tchecker/system/synchronization.hh"

namespace tchecker {
  
  // sync_constraint_t
  
  sync_constraint_t::sync_constraint_t(tchecker::process_id_t pid, tchecker::event_id_t event_id,
                                       tchecker::sync_strength_t strength)
  : _pid(pid),
  _event_id(event_id),
  _strength(strength)
  {}
  
  
  
  
  // synchronization_t
  
  bool synchronization_t::synchronizes(tchecker::process_id_t pid, tchecker::event_id_t event_id) const
  {
    for (tchecker::sync_constraint_t const & constr : _constraints)
      if ((constr.pid() == pid) && (constr.event_id() == event_id))
        return true;
    return false;
  }
  
} // end of namespace tchecker
