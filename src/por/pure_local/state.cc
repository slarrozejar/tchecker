/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/pure_local/state.hh"

namespace tchecker {
  
  namespace por {

    namespace pure_local {
    
      state_t::state_t(tchecker::process_id_t rank, tchecker::process_id_t pid)
      : tchecker::por::state_t(rank), _pl_pid(pid)
      {}
    
    
      tchecker::process_id_t state_t::pl_pid() const
      {
        return _pl_pid;
      }
    
    
      void state_t::pl_pid(tchecker::process_id_t pid)
      {
        _pl_pid = pid;
      }


      bool tchecker::por::pure_local::state_t::operator== (tchecker::por::pure_local::state_t const & s) const
      {
        return tchecker::por::state_t::operator==(s) 
                && (_pl_pid == s._pl_pid);
      }
    

      bool tchecker::por::pure_local::state_t::operator!= (tchecker::por::pure_local::state_t const & s) const
      {
        return ! (*this == s);
      }
    
    
      std::size_t hash_value(tchecker::por::pure_local::state_t const & s)
      {
        return s.pl_pid();
      }
    
    
      int lexical_cmp(tchecker::por::pure_local::state_t const & s1, tchecker::por::pure_local::state_t const & s2)
      {
        int cmp = tchecker::por::lexical_cmp(s1, s2);
        if (cmp != 0)
          return cmp;

        tchecker::process_id_t p1 = s1.pl_pid(), p2 = s2.pl_pid();
        if (p1 < p2)
          return -1;
        else if (p1 == p2)
          return 0;
        else
          return 1;
      }

    } // end of namespace pure_local
    
  } // end of namespace por
  
} // end of namespace tchecker
