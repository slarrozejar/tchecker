/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR1_STATE_HH
#define TCHECKER_POR_POR1_STATE_HH

#include <limits>

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/basictypes.hh"
#include "tchecker/utils/allocation_size.hh"
#include "tchecker/system/static_analysis.hh"

/*!
 \file state.hh
 \brief States for por1 POR
 */

namespace tchecker {

  namespace por {

    namespace por1 {

      /*!< POR memory for stats with no selected process */
      extern tchecker::process_id_t NO_SELECTED_PROCESS;

      /*!
      \class state_t
      \brief State for por1 POR
      */
      class state_t {
      public:
        /*!
        \brief Constructor
        \param por_mem : POR memory
        */
        state_t(tchecker::process_id_t por_mem = NO_SELECTED_PROCESS);

        /*!
        \brief Equality predicate
        \param s : state
        \return true if this and s are same state
        */
        bool operator== (tchecker::por::por1::state_t const & s) const;

        /*!
        \brief Disequality predicate
        \param s : state
        \return true if this and s are different
        */
        bool operator!= (tchecker::por::por1::state_t const & s) const;

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
      private:
        tchecker::process_id_t _por_mem;   /*!< POR memory */
      };


      /*!
      \brief Hash
      \param s : state
      \return hash value for s
      */
      std::size_t hash_value(tchecker::por::por1::state_t const & s);


      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 has smaller state than s2, 0 if same state, >0 otherwise
      */
      int lexical_cmp(tchecker::por::por1::state_t const & s1,
                      tchecker::por::por1::state_t const & s2);


      /*!
      \brief Make state a client-server POR state
      \tparam STATE : type of state
      \note make_state_t<STATE> derives both from STATE and from tchecker::por::por1::state_t
      */
      template <class STATE>
      class make_state_t : public STATE, public tchecker::por::por1::state_t {
      public:
        using STATE::STATE;

        /*!
        \brief Constructor
        \param s : a state
        \param args : arguments to a constructor STATE::STATE(s, args...)
        */
        template <class ... ARGS>
        make_state_t(tchecker::por::por1::make_state_t<STATE> const & s,
                     ARGS && ... args)
        : STATE(s, args...), tchecker::por::por1::state_t(s)
        {}
      };


      /*!
      \brief Equality predicate
      \param s1 : state
      \param s2 : state
      \return true if s1 and s2 are equal, false otherwise
      */
      template <class STATE>
      bool operator== (tchecker::por::por1::make_state_t<STATE> const & s1,
                       tchecker::por::por1::make_state_t<STATE> const & s2)
      {
        return
        ((static_cast<tchecker::por::por1::state_t const &>(s1) ==
          static_cast<tchecker::por::por1::state_t const &>(s2))
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
      bool operator!= (tchecker::por::por1::make_state_t<STATE> const & s1,
                       tchecker::por::por1::make_state_t<STATE> const & s2)
      {
        return ! (s1 == s2);
      }


      /*!
      \brief Hash
      \param s : state
      \return hash code for s
      */
      template <class STATE>
      std::size_t hash_value(tchecker::por::por1::make_state_t<STATE> const & s)
      {
        std::size_t h = tchecker::por::por1::hash_value
        (static_cast<tchecker::por::por1::state_t const &>(s));
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
      int lexical_cmp(tchecker::por::por1::make_state_t<STATE> const & s1,
                      tchecker::por::por1::make_state_t<STATE> const & s2)
      {
        int cmp = lexical_cmp(static_cast<STATE const &>(s1),
                              static_cast<STATE const &>(s2));
        if (cmp != 0)
          return cmp;
        return tchecker::por::por1::lexical_cmp
        (static_cast<tchecker::por::por1::state_t const &>(s1),
         static_cast<tchecker::por::por1::state_t const &>(s2));
      }


      /*!
      \brief Covering check
      \param s1 : state
      \param s2 : state
      \return true if s1 can be covered by s2, false othewise
      */
      template <class STATE>
      bool cover_leq(tchecker::por::por1::make_state_t<STATE> const & s1,
                      tchecker::por::por1::make_state_t<STATE> const & s2,
                      tchecker::pure_local_map_t const & pure_local)
      {
        // same memory
        if (s1.por_memory() == s2.por_memory())
          return true;
        // state with selected process covered by a state with no selected process
        if (s1.por_memory() != NO_SELECTED_PROCESS && s2.por_memory() == NO_SELECTED_PROCESS) {
          for (auto const * loc : s2.vloc()){
            tchecker::loc_id_t id = loc->id();
            tchecker::process_id_t pid = loc->pid();
            tchecker::process_id_t m = s1.por_memory();
            if (pure_local.is_pure_local(id) && pid != m)
              return false;
          }
        }
        // state with no selected process covered by state with selected process
        if (s1.por_memory() == NO_SELECTED_PROCESS && s2.por_memory() != NO_SELECTED_PROCESS) {
          for (auto const * loc : s1.vloc()){
            tchecker::loc_id_t id = loc->id();
            tchecker::process_id_t pid = loc->pid();
            tchecker::process_id_t m = s2.por_memory();
            auto outgoing_edges = loc->outgoing_edges();
            if (pid != m && (pure_local.is_pure_local(id) || !outgoing_edges.empty()))
              return false;
          }
        }
        return true;
      }

    } // end of namespace por1

  } // end of namespace por


  /*!
   \brief Specialization of class allocation_size_t for tchecker::por::por1::state_t
   */
  template <class STATE>
  class allocation_size_t<tchecker::por::por1::make_state_t<STATE>> {
  public:
    template <class ... ARGS>
    static constexpr std::size_t alloc_size(ARGS && ... args)
    {
      return sizeof(tchecker::por::por1::make_state_t<STATE>);
    }
  };

} // end of namespace tchecker

#endif // TCHECKER_POR_POR1_STATE_HH
