/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include "tchecker/por/pure_local/ts.hh"

namespace tchecker {
  
  namespace por {
    
    namespace pure_local {
              
      bool cover_leq(tchecker::por::pure_local::state_t const & s1, tchecker::por::pure_local::state_t const & s2)
      {
        return (s1.pl_pid() == s2.pl_pid() ||
                s2.pl_pid() == tchecker::por::pure_local::no_pure_local);
      }
      
    } // end of namespace pure_local
    
  } // end of namespace por
  
} // end of namespace tchecker

