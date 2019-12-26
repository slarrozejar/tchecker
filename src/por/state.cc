/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/state.hh"

namespace tchecker {
  
  namespace por {
    
    tchecker::process_id_t const all_processes_active = std::numeric_limits<tchecker::process_id_t>::max();
    
    /* state_t */
    
    state_t::state_t()
    : _active_pid(tchecker::por::all_processes_active)
    {}
    
    
    void state_t::active_pid(tchecker::process_id_t active_pid)
    {
      _active_pid = active_pid;
    }

    
    bool tchecker::por::state_t::operator== (tchecker::por::state_t const & s) const
    {
      return (_active_pid == s._active_pid);
    }
    

    bool tchecker::por::state_t::operator!= (tchecker::por::state_t const & s) const
    {
      return ! (*this == s);
    }
    
    
   
  
    bool permissive_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return s2.all_active() || (!s1.all_active() && (s1.active_pid() == s2.active_pid()));
    }
    
    
    std::size_t hash_value(tchecker::por::state_t const & s)
    {
      return s.active_pid();
    }
    
    
    int lexical_cmp(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      if (s2.all_active())
        return (s1.all_active() ? 0 : 1);
      else
        return (s1.all_active() ? -1 : s1.active_pid() - s2.active_pid());
    }
    
  } // end of namespace por
  
} // end of namespace tchecker
