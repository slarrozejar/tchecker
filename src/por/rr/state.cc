/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/rr/state.hh"

namespace tchecker {

  namespace por {

    namespace rr {

      tchecker::process_id_t NO_SELECTED_PROCESS = std::numeric_limits<tchecker::process_id_t>::max();

      state_t::state_t(tchecker::process_id_t por_mem, 
                      tchecker::process_id_t mixed_local) : _por_mem(por_mem),
      _mixed_local(mixed_local)
      {}

      bool tchecker::por::rr::state_t::operator==
      (tchecker::por::rr::state_t const & s) const
      {
        return _por_mem == s._por_mem && _mixed_local == s._mixed_local;
      }


      bool tchecker::por::rr::state_t::operator!=
      (tchecker::por::rr::state_t const & s) const
      {
        return ! (*this == s);
      }

      tchecker::process_id_t tchecker::por::rr::state_t::por_memory() const
      {
        return _por_mem;
      }


      void tchecker::por::rr::state_t::por_memory(tchecker::process_id_t por_mem)
      {
        _por_mem = por_mem;
      }

            tchecker::process_id_t tchecker::por::rr::state_t::mixed_local() const
      {
        return _mixed_local;
      }


      void tchecker::por::rr::state_t::mixed_local(tchecker::process_id_t mixed_local)
      {
        _mixed_local = mixed_local;
      }


      std::size_t hash_value(tchecker::por::rr::state_t const & s)
      {
        std::size_t seed = 0;
        boost::hash_combine(seed, s.por_memory());
        boost::hash_combine(seed, s.mixed_local());

        return seed;
      }

      int lexical_cmp(tchecker::por::rr::state_t const & s1,
                      tchecker::por::rr::state_t const & s2)
      {
        if (s1.por_memory() < s2.por_memory())
          return -1;
        if (s1.por_memory() == s2.por_memory()) {
          if (s1.mixed_local() < s2.mixed_local())
            return -1;
          if (s1.mixed_local() == s2.mixed_local())
            return 0;
          return 1;
        }
        return 1;
      }

      bool cover_leq(tchecker::por::rr::state_t const & s1,
                     tchecker::por::rr::state_t const & s2)
      {
        // s2 allows all transitions from s1
        return (s1.por_memory() == s2.por_memory() && s1.mixed_local() == s2.mixed_local());
      }

  } // end of namespace rr

  } // end of namespace por

} // end of namespace tchecker
