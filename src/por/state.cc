/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include "tchecker/por/state.hh"

namespace tchecker {
  
  namespace por {
    
    /* state_t */
    
    state_t::state_t(tchecker::process_id_t active_pid)
    : _active_pid(active_pid)
    {}
    
    
    void state_t::active_pid(tchecker::process_id_t pid)
    {
      _active_pid = pid;
    }
    
    
    
    bool operator== (tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return s1.active_pid() == s2.active_pid();
    }
    
    
    bool operator!= (tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return ! (s1 == s2);
    }
    
    
    std::size_t hash_value(tchecker::por::state_t const & s)
    {
      return s.active_pid();
    }
    
    
    int lexical_cmp(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      if (s1.active_pid() < s2.active_pid())
        return -1;
      else if (s1.active_pid() == s2.active_pid())
        return 0;
      return 1;
    }
    
  } // end of namespace por
  
} // end of namespace tchecker
