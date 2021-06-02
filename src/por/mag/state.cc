/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/mag/state.hh"

namespace tchecker {

  namespace por {

    namespace mag {

      state_t::state_t()
      {}

      bool tchecker::por::mag::state_t::operator==
      (tchecker::por::mag::state_t const & s) const
      {
        return true;
      }


      bool tchecker::por::mag::state_t::operator!=
      (tchecker::por::mag::state_t const & s) const
      {
        return ! (*this == s);
      }


      std::size_t hash_value(tchecker::por::mag::state_t const & s)
      {
        return 0;
      }


      int lexical_cmp(tchecker::por::mag::state_t const & s1,
                      tchecker::por::mag::state_t const & s2)
      {
        return 0;
      }


      bool cover_leq(tchecker::por::mag::state_t const & s1,
                     tchecker::por::mag::state_t const & s2)
      {
        // s2 allows all transitions from s1
        return true;
      }

  } // end of namespace mag

  } // end of namespace por

} // end of namespace tchecker
