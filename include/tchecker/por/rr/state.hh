/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_RR_STATE_HH
#define TCHECKER_POR_RR_STATE_HH

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
 \brief States for rr POR
 */

namespace tchecker {

  namespace por {

    namespace rr {

      /*!< POR memory for stats with no selected process */
      extern tchecker::process_id_t NO_SELECTED_PROCESS;

      /*!
      \class state_t
      \brief State for rr POR
      */
      class state_t {
      public:
        /*!
        \brief Constructor
        */
        state_t(tchecker::process_id_t por_mem = NO_SELECTED_PROCESS, 
                tchecker::process_id_t mixed_local = NO_SELECTED_PROCESS);

        /*!
        \brief Equality predicate
        \param s : state
        \return true if this and s are same state
        */
        bool operator== (tchecker::por::rr::state_t const & s) const;

        /*!
        \brief Disequality predicate
        \param s : state
        \return true if this and s are different
        */
        bool operator!= (tchecker::por::rr::state_t const & s) const;

        /*!
        \brief Access to POR memory
        \return POR memory of this state
        */
        tchecker::process_id_t por_memory() const;

        /*!
        \brief Set POR memory
        \param por_mem : POR memory
        \post this state's POR memory has been set to por_mem
        */
        void por_memory(tchecker::process_id_t por_mem);

        /*!
        \brief Access to POR memory
        \return mixed local of this state
        */
        tchecker::process_id_t mixed_local() const;

        /*!
        \brief Set POR memory
        \param por_mem : POR memory
        \post this state's POR memory has been set to mixed_local
        */
        void mixed_local(tchecker::process_id_t mixed_local);

      private:
        tchecker::process_id_t _por_mem;   /*!< POR memory */
        tchecker::process_id_t _mixed_local;   /*!< POR mem for mixed states */
      };


      /*!
      \brief Hash
      \param s : state
      \return hash value for s
      */
      std::size_t hash_value(tchecker::por::rr::state_t const & s);


      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 has smaller state than s2, 0 if same state, >0 otherwise
      */
      int lexical_cmp(tchecker::por::rr::state_t const & s1,
                      tchecker::por::rr::state_t const & s2);


      /*!
      \brief Make state a client-server POR state
      \tparam STATE : type of state
      \note make_state_t<STATE> derives both from STATE and from tchecker::por::rr::state_t
      */
      template <class STATE>
      class make_state_t : public STATE, public tchecker::por::rr::state_t {
      public:
        using STATE::STATE;

        /*!
        \brief Constructor
        \param s : a state
        \param args : arguments to a constructor STATE::STATE(s, args...)
        */
        template <class ... ARGS>
        make_state_t(tchecker::por::rr::make_state_t<STATE> const & s,
                     ARGS && ... args)
        : STATE(s, args...), tchecker::por::rr::state_t(s)
        {}
      };


      /*!
      \brief Equality predicate
      \param s1 : state
      \param s2 : state
      \return true if s1 and s2 are equal, false otherwise
      */
      template <class STATE>
      bool operator== (tchecker::por::rr::make_state_t<STATE> const & s1,
                       tchecker::por::rr::make_state_t<STATE> const & s2)
      {
        return
        ((static_cast<tchecker::por::rr::state_t const &>(s1) ==
          static_cast<tchecker::por::rr::state_t const &>(s2))
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
      bool operator!= (tchecker::por::rr::make_state_t<STATE> const & s1,
                       tchecker::por::rr::make_state_t<STATE> const & s2)
      {
        return ! (s1 == s2);
      }


      /*!
      \brief Hash
      \param s : state
      \return hash code for s
      */
      template <class STATE>
      std::size_t hash_value(tchecker::por::rr::make_state_t<STATE> const & s)
      {
        std::size_t h = tchecker::por::rr::hash_value
        (static_cast<tchecker::por::rr::state_t const &>(s));
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
      int lexical_cmp(tchecker::por::rr::make_state_t<STATE> const & s1,
                      tchecker::por::rr::make_state_t<STATE> const & s2)
      {
        int cmp = lexical_cmp(static_cast<STATE const &>(s1),
                              static_cast<STATE const &>(s2));
        if (cmp != 0)
          return cmp;
        return tchecker::por::rr::lexical_cmp
        (static_cast<tchecker::por::rr::state_t const &>(s1),
         static_cast<tchecker::por::rr::state_t const &>(s2));
      }


      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s2, false othewise
       */
      bool cover_leq(tchecker::por::rr::state_t const & s1,
                     tchecker::por::rr::state_t const & s2);

    } // end of namespace rr

  } // end of namespace por


  /*!
   \brief Specialization of class allocation_size_t for tchecker::por::rr::state_t
   */
  template <class STATE>
  class allocation_size_t<tchecker::por::rr::make_state_t<STATE>> {
  public:
    template <class ... ARGS>
    static constexpr std::size_t alloc_size(ARGS && ... args)
    {
      return sizeof(tchecker::por::rr::make_state_t<STATE>);
    }
  };

} // end of namespace tchecker

#endif // TCHECKER_POR_RR_STATE_HH
