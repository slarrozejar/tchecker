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

      state_t::state_t(tchecker::process_id_t id)
      : _por_id(id)
      {}

      tchecker::process_id_t state_t::por_id() const
      {
        return _por_id;
      }

      void state_t::por_id(tchecker::process_id_t id)
      {
        _por_id = id;
      }

      bool tchecker::por::por1::state_t::operator==
      (tchecker::por::por1::state_t const & s) const
      {
        return (_por_id == s._por_id);
      }


      bool tchecker::por::por1::state_t::operator!=
      (tchecker::por::por1::state_t const & s) const
      {
        return ! (*this == s);
      }


      std::size_t hash_value(tchecker::por::por1::state_t const & s)
      {
        return s.por_id();
      }


      int lexical_cmp(tchecker::por::por1::state_t const & s1,
                      tchecker::por::por1::state_t const & s2)
      {
        tchecker::process_id_t pid1 = s1.por_id();
        tchecker::process_id_t pid2 = s2.por_id();
        if (pid1 < pid2)
          return -1;
        else if (pid1 == pid2)
          return 0;
        else
          return 1;
      }


      bool cover_leq(tchecker::por::por1::state_t const & s1,
                     tchecker::por::por1::state_t const & s2)
      {
        // s2 allows more transitions than s1
        return false
      }

  } // end of namespace por1

  } // end of namespace por

} // end of namespace tchecker
