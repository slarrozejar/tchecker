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
    
    state_t::state_t(tchecker::process_id_t rank)
    : _rank(rank)
    {}
    
    
    tchecker::process_id_t state_t::rank() const
    {
      return _rank;
    }
    
    
    void state_t::rank(tchecker::process_id_t rank)
    {
      _rank = rank;
    }

    
    bool tchecker::por::state_t::operator== (tchecker::por::state_t const & s) const
    {
      return (_rank == s._rank);
    }
    

    bool tchecker::por::state_t::operator!= (tchecker::por::state_t const & s) const
    {
      return ! (*this == s);
    }
    
    
    std::size_t hash_value(tchecker::por::state_t const & s)
    {
      return s.rank();
    }
    
    
    int lexical_cmp(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
    {
      tchecker::process_id_t r1 = s1.rank(), r2 = s2.rank();
      if (r1 < r2)
        return -1;
      else if (r1 == r2)
        return 0;
      else
        return 1;
    }
    
  } // end of namespace por
  
} // end of namespace tchecker
