/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include "tchecker/por/cs/ts.hh"

namespace tchecker {
  
  namespace por {
    
    namespace cs {
              
      bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
      {
        // s2 allows more transitions than s1
        return (s2.rank() == tchecker::por::cs::communication) || (s1.rank() == s2.rank());
      }
      
    } // end of namespace cs
    
  } // end of namespace por
  
} // end of namespace tchecker

