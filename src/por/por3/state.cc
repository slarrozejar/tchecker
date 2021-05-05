/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/por3/state.hh"

namespace tchecker {

  namespace por {

    namespace por3 {

      tchecker::process_id_t NO_SELECTED_PROCESS = std::numeric_limits<tchecker::process_id_t>::max();

      state_t::state_t(tchecker::process_id_t por_mem) : _por_mem(por_mem)
      {}

      bool tchecker::por::por3::state_t::operator==
      (tchecker::por::por3::state_t const & s) const
      {
        return _por_mem == s._por_mem;
      }


      bool tchecker::por::por3::state_t::operator!=
      (tchecker::por::por3::state_t const & s) const
      {
        return ! (*this == s);
      }

      tchecker::process_id_t tchecker::por::por3::state_t::por_memory() const
      {
        return _por_mem;
      }


      void tchecker::por::por3::state_t::por_memory(tchecker::process_id_t por_mem)
      {
        _por_mem = por_mem;
      }

      std::size_t hash_value(tchecker::por::por3::state_t const & s)
      {
        return s.por_memory();
      }


      int lexical_cmp(tchecker::por::por3::state_t const & s1,
                      tchecker::por::por3::state_t const & s2)
      {
        if (s1.por_memory() < s2.por_memory())
          return -1;
        if (s1.por_memory() == s2.por_memory())
          return 0;
        return 1;
      }


      bool cover_leq(tchecker::por::por3::state_t const & s1,
                     tchecker::por::por3::state_t const & s2)
      {
        // s2 simulates s1 if they have the same memory
        return (s1.por_memory() == s2.por_memory());
      }

  } // end of namespace por3

  } // end of namespace por

} // end of namespace tchecker
