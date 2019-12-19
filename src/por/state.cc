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
    
    state_t::state_t(tchecker::process_id_t rank)
    : _rank(rank)
    {}
    
    
    void state_t::rank(tchecker::process_id_t pid)
    {
      _rank = pid;
    }
    
    
    
    bool operator== (tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return s1.rank() == s2.rank();
    }
    
    
    bool operator!= (tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return ! (s1 == s2);
    }
    
    
    bool operator<= (tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      return s1.rank() <= s2.rank();
    }
    
    
    std::size_t hash_value(tchecker::por::state_t const & s)
    {
      return s.rank();
    }
    
    
    int lexical_cmp(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      if (s1.rank() < s2.rank())
        return -1;
      else if (s1.rank() == s2.rank())
        return 0;
      return 1;
    }
    
  } // end of namespace por
  
} // end of namespace tchecker
