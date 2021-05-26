/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR5_STATE_HH
#define TCHECKER_POR_POR5_STATE_HH

#include <limits>

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/basictypes.hh"
#include "tchecker/utils/allocation_size.hh"

/*!
 \file state.hh
 \brief States for por5 POR
 */

namespace tchecker {

  namespace por {

    namespace por5 {

      /*!
      \class state_t
      \brief State for por5 POR
      */
      class state_t {
      public:
        /*!
        \brief Constructor
        */
        state_t();

        /*!
        \brief Equality predicate
        \param s : state
        \return true if this and s are same state
        */
        bool operator== (tchecker::por::por5::state_t const & s) const;

        /*!
        \brief Disequality predicate
        \param s : state
        \return true if this and s are different
        */
        bool operator!= (tchecker::por::por5::state_t const & s) const;
      };


      /*!
      \brief Hash
      \param s : state
      \return hash value for s
      */
      std::size_t hash_value(tchecker::por::por5::state_t const & s);


      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 has smaller state than s2, 0 if same state, >0 otherwise
      */
      int lexical_cmp(tchecker::por::por5::state_t const & s1,
                      tchecker::por::por5::state_t const & s2);


      /*!
      \brief Make state a client-server POR state
      \tparam STATE : type of state
      \note make_state_t<STATE> derives both from STATE and from tchecker::por::por5::state_t
      */
      template <class STATE>
      class make_state_t : public STATE, public tchecker::por::por5::state_t {
      public:
        using STATE::STATE;

        /*!
        \brief Constructor
        \param s : a state
        \param args : arguments to a constructor STATE::STATE(s, args...)
        */
        template <class ... ARGS>
        make_state_t(tchecker::por::por5::make_state_t<STATE> const & s,
                     ARGS && ... args)
        : STATE(s, args...), tchecker::por::por5::state_t(s)
        {}
      };


      /*!
      \brief Equality predicate
      \param s1 : state
      \param s2 : state
      \return true if s1 and s2 are equal, false otherwise
      */
      template <class STATE>
      bool operator== (tchecker::por::por5::make_state_t<STATE> const & s1,
                       tchecker::por::por5::make_state_t<STATE> const & s2)
      {
        return
        ((static_cast<tchecker::por::por5::state_t const &>(s1) ==
          static_cast<tchecker::por::por5::state_t const &>(s2))
        &&
         (static_cast<STATE const &>(s1) == static_cast<STATE const &>(s2))
        );
      }


      /*!
      \brief Disequality predicate
      \param s1 : state
      \param s2 : state
      \return false if s1 and s2 are equal, true otherwise
      */
      template <class STATE>
      bool operator!= (tchecker::por::por5::make_state_t<STATE> const & s1,
                       tchecker::por::por5::make_state_t<STATE> const & s2)
      {
        return ! (s1 == s2);
      }


      /*!
      \brief Hash
      \param s : state
      \return hash code for s
      */
      template <class STATE>
      std::size_t hash_value(tchecker::por::por5::make_state_t<STATE> const & s)
      {
        std::size_t h = tchecker::por::por5::hash_value
        (static_cast<tchecker::por::por5::state_t const &>(s));
        boost::hash_combine(h, hash_value(static_cast<STATE const &>(s)));
        return h;
      }


      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 is smaller than s2, 0 if s1 and s2 are equal, >0
      otherwise
      */
      template <class STATE>
      int lexical_cmp(tchecker::por::por5::make_state_t<STATE> const & s1,
                      tchecker::por::por5::make_state_t<STATE> const & s2)
      {
        int cmp = lexical_cmp(static_cast<STATE const &>(s1),
                              static_cast<STATE const &>(s2));
        if (cmp != 0)
          return cmp;
        return tchecker::por::por5::lexical_cmp
        (static_cast<tchecker::por::por5::state_t const &>(s1),
         static_cast<tchecker::por::por5::state_t const &>(s2));
      }


      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s2, false othewise
       */
      bool cover_leq(tchecker::por::por5::state_t const & s1,
                     tchecker::por::por5::state_t const & s2);

    } // end of namespace por5

  } // end of namespace por


  /*!
   \brief Specialization of class allocation_size_t for tchecker::por::por5::state_t
   */
  template <class STATE>
  class allocation_size_t<tchecker::por::por5::make_state_t<STATE>> {
  public:
    template <class ... ARGS>
    static constexpr std::size_t alloc_size(ARGS && ... args)
    {
      return sizeof(tchecker::por::por5::make_state_t<STATE>);
    }
  };

} // end of namespace tchecker

#endif // TCHECKER_POR_POR5_STATE_HH
