/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include "tchecker/por/gl/ts.hh"

namespace tchecker {
  
  namespace por {
    
    namespace gl {
              
      bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2)
      {
        // s2 allows more transitions than s1
        return ((s2.rank() == tchecker::por::gl::global) ||
                ((s1.rank() != tchecker::por::gl::global) && (s2.rank() <= s1.rank())));
      }
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker

