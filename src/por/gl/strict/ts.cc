/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include "tchecker/por/gl/strict/ts.hh"

namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      namespace strict {
        
        bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
        {
          return ((s2.rank() == tchecker::por::gl::strict::details::all_processes_active) ||
                  (s1.rank() == s2.rank()));
        }
        
      } // end of namespace strict
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker
