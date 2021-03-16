/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/por1/state.hh"

namespace tchecker {

  namespace por {

    namespace por1 {

      unsigned NO_SELECTED_PROCESS = std::numeric_limits<unsigned>::max();

      state_t::state_t(unsigned por_mem) : _por_mem(por_mem)
      {}

      bool tchecker::por::por1::state_t::operator==
      (tchecker::por::por1::state_t const & s) const
      {
        return _por_mem == s._por_mem;
      }


      bool tchecker::por::por1::state_t::operator!=
      (tchecker::por::por1::state_t const & s) const
      {
        return ! (*this == s);
      }


      unsigned tchecker::por::por1::state_t::por_memory() const
      {
        return _por_mem;
      }


      void tchecker::por::por1::state_t::por_memory(unsigned por_mem)
      {
        _por_mem = por_mem;
      }


      std::size_t hash_value(tchecker::por::por1::state_t const & s)
      {
        return s.por_memory();
      }


      int lexical_cmp(tchecker::por::por1::state_t const & s1,
                      tchecker::por::por1::state_t const & s2)
      {
        if (s1.por_memory() < s2.por_memory())
          return -1;
        if (s1.por_memory() == s2.por_memory())
          return 0;
        return 1;
      }


      bool cover_leq(tchecker::por::por1::state_t const & s1,
                     tchecker::por::por1::state_t const & s2)
      {
        // s2 simulates s1 and simulation is compatible with por1 source
        // TODO: improve with s2.por_memory() == 0 when s1.por_memory() is the
        // smallest pure local process or no pure local
        return (s1.por_memory() == s2.por_memory());
      }

  } // end of namespace por1

  } // end of namespace por

} // end of namespace tchecker
