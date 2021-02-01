/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/gl/state.hh"

namespace tchecker {
  
  namespace por {

    namespace gl {
    
      state_t::state_t(tchecker::process_id_t rank)
      : _por_rank(rank)
      {}
    
    
      tchecker::process_id_t state_t::por_rank() const
      {
        return _por_rank;
      }
    
    
      void state_t::por_rank(tchecker::process_id_t rank)
      {
        _por_rank = rank;
      }

    
      bool tchecker::por::gl::state_t::operator== 
      (tchecker::por::gl::state_t const & s) const
      {
        return (_por_rank == s._por_rank);
      }
    

      bool tchecker::por::gl::state_t::operator!= 
      (tchecker::por::gl::state_t const & s) const
      {
        return ! (*this == s);
      }
    
    
      std::size_t hash_value(tchecker::por::gl::state_t const & s)
      {
        return s.por_rank();
      }
    
    
      int lexical_cmp(tchecker::por::gl::state_t const & s1, 
                      tchecker::por::gl::state_t const & s2)
      {
        tchecker::process_id_t pid1 = s1.por_rank();
        tchecker::process_id_t pid2 = s2.por_rank();
        if (pid1 < pid2)
          return -1;
        else if (pid1 == pid2)
          return 0;
        else
          return 1;
      }

      bool cover_leq(tchecker::por::gl::state_t const & s1, 
                     tchecker::por::gl::state_t const & s2)
      {
        // s2 allows more transitions than s1
        return (s2.por_rank() == tchecker::por::gl::global ||
                (s1.por_rank() != tchecker::por::gl::global && 
                 s2.por_rank() <= s1.por_rank())
               );
      }

    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker
