/*
* This file is a part of the TChecker project.
*
* See files AUTHORS and LICENSE for copyright details.
*
*/

#include <limits>

#include "tchecker/por/por2/state.hh"
#include <boost/dynamic_bitset.hpp>

namespace tchecker {

  namespace por {

    namespace por2 {

      tchecker::por::por2::state_t::state_t(tchecker::process_id_t processes_count) : _por_L(processes_count), _por_S(processes_count) {}

      bool tchecker::por::por2::state_t::operator==
      (tchecker::por::por2::state_t const & s) const
      {
        return (_por_L == s.por_L() && _por_S == s.por_S());
      }


      bool tchecker::por::por2::state_t::operator!=
      (tchecker::por::por2::state_t const & s) const
      {
        return ! (*this == s);
      }

      boost::dynamic_bitset<> const & state_t::por_L() const
      {
        return _por_L;
      }

      boost::dynamic_bitset<> & state_t::por_L()
      {
        return _por_L;
      }

      boost::dynamic_bitset<> const & state_t::por_S() const
      {
        return _por_S;
      }

      boost::dynamic_bitset<> & state_t::por_S()
      {
        return _por_S;
      }

      std::size_t hash_value(tchecker::por::por2::state_t const & s)
      {
        std::size_t h = hash_value(s.por_L());
        boost::hash_combine(h, s.por_S());
        return h;
      }


      int lexical_cmp(tchecker::por::por2::state_t const & s1,
                      tchecker::por::por2::state_t const & s2)
      {
        std::size_t h1 = hash_value(s1);
        std::size_t h2 = hash_value(s2);
        if (h1 < h2)
          return -1;
        else if (h1 == h2)
          return 0;
        else 
          return 1;
      }


      bool cover_leq(tchecker::por::por2::state_t const & s1,
                     tchecker::por::por2::state_t const & s2)
      {
        // s2 allows all transitions from s1
        return (s1 == s2);
      }

  } // end of namespace por2

  } // end of namespace por

} // end of namespace tchecker