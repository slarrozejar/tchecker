/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/cs/state.hh"

namespace tchecker {
  
  namespace por {

    namespace cs {
    
      state_t::state_t(tchecker::process_id_t pid)
      : _por_active_pid(pid)
      {}
    
    
      tchecker::process_id_t state_t::por_active_pid() const
      {
        return _por_active_pid;
      }
    
    
      void state_t::por_active_pid(tchecker::process_id_t pid)
      {
        _por_active_pid = pid;
      }

    
      bool tchecker::por::cs::state_t::operator== 
      (tchecker::por::cs::state_t const & s) const
      {
        return (_por_active_pid == s._por_active_pid);
      }
    

      bool tchecker::por::cs::state_t::operator!= 
      (tchecker::por::cs::state_t const & s) const
      {
        return ! (*this == s);
      }
    
    
      std::size_t hash_value(tchecker::por::cs::state_t const & s)
      {
        return s.por_active_pid();
      }
    
    
      int lexical_cmp(tchecker::por::cs::state_t const & s1, 
                      tchecker::por::cs::state_t const & s2)
      {
        tchecker::process_id_t pid1 = s1.por_active_pid();
        tchecker::process_id_t pid2 = s2.por_active_pid();
        if (pid1 < pid2)
          return -1;
        else if (pid1 == pid2)
          return 0;
        else
          return 1;
      }


      bool cover_leq(tchecker::por::cs::state_t const & s1, 
                     tchecker::por::cs::state_t const & s2)
      {
        // s2 allows more transitions than s1
        return (s2.por_active_pid() == tchecker::por::cs::communication
                || s1.por_active_pid() == s2.por_active_pid());
      }

    } // end of namespace cs
    
  } // end of namespace por
  
} // end of namespace tchecker
