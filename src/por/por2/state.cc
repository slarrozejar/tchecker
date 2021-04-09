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

      state_t::state_t(tchecker::process_id_t processes_count) : _por_L(processes_count), _por_S(processes_count) {}

      bool tchecker::por::por2::state_t::operator==
      (tchecker::por::por2::state_t const & s) const
      {
        return (_por_L == s.por_L() && _por_S == s.por_S());
      }


      bool state_t::operator!=
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

      tchecker::process_id_t max(boost::dynamic_bitset<> const & bs)
        {
          tchecker::process_id_t max = 0;
          for (tchecker::process_id_t pid = 0; pid <bs.size(); ++pid)
            if (bs[pid])
              max = pid;
          if (! bs[max])
            throw "Cannot compute max on empty bitset";
          return max;
        }


      bool cover_leq(tchecker::por::por2::state_t const & s1,
                     tchecker::por::por2::state_t const & s2)
      {
        if (s1.por_L().none() && s2.por_L().none()) // both states in synchro phase
          return s1.por_S().is_subset_of(s2.por_S());
        else if (!s1.por_L().none() && !s2.por_L().none()) // both states in local phase
        {
          tchecker::process_id_t max_L1 = max(s1.por_L());
          tchecker::process_id_t max_L2 = max(s2.por_L());
          boost::dynamic_bitset<> tmp1 = s1.por_L();
          tmp1[max_L1] = false;
          boost::dynamic_bitset<> const & reduced_L1 = tmp1;
          boost::dynamic_bitset<> tmp2 = s2.por_L();
          tmp1[max_L2] = false;
          boost::dynamic_bitset<> const & reduced_L2 = tmp2;
          boost::dynamic_bitset<> const & local_pid1 = boost::operator-(s1.por_S(), reduced_L1);
          boost::dynamic_bitset<> const & local_pid2 = boost::operator-(s2.por_S(), reduced_L2);
          return local_pid1.is_subset_of(local_pid2) && s1.por_L().is_subset_of(s2.por_L());
        }
        return (s1 == s2);
      }

  } // end of namespace por2

  } // end of namespace por

} // end of namespace tchecker
