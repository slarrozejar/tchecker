/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/por5/state.hh"

namespace tchecker {

  namespace por {

    namespace por5 {

      state_t::state_t()
      {}

      bool tchecker::por::por5::state_t::operator==
      (tchecker::por::por5::state_t const & s) const
      {
        return true;
      }


      bool tchecker::por::por5::state_t::operator!=
      (tchecker::por::por5::state_t const & s) const
      {
        return ! (*this == s);
      }


      std::size_t hash_value(tchecker::por::por5::state_t const & s)
      {
        return 0;
      }


      int lexical_cmp(tchecker::por::por5::state_t const & s1,
                      tchecker::por::por5::state_t const & s2)
      {
        return 0;
      }


      bool cover_leq(tchecker::por::por5::state_t const & s1,
                     tchecker::por::por5::state_t const & s2)
      {
        // s2 allows all transitions from s1
        return true;
      }

  } // end of namespace por5

  } // end of namespace por

} // end of namespace tchecker
