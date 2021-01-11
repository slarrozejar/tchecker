/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_PURE_LOCAL_STATE_HH
#define TCHECKER_POR_PURE_LOCAL_STATE_HH

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/basictypes.hh"
#include "tchecker/por/state.hh"

/*!
 \file state.hh
 \brief States for POR transition systems with priority to pure local processes
 */

namespace tchecker {
  
  namespace por {

    namespace pure_local {
    
      /*!
      \class state_t
      \brief State for POR transition systems with pid of pure local process and
      rank
      */
      class state_t : public tchecker::por::state_t {
      public:
        /*!
        \brief Constructor
        \param rank : rank of the state
        \param pid : pid of pure local process
        */
        state_t(tchecker::process_id_t rank = 0, tchecker::process_id_t pid = 0);
      
        /*!
        \brief Accessor
        \return pid of pure local process
       */
        tchecker::process_id_t pl_pid() const;
      
        /*!
        \brief Set pid of pure local process
        \param pid : a pid
        \post pid of pure local process has been set to pid
        */
        void pl_pid(tchecker::process_id_t pid);

        /*!
        \brief Equality predicate
        \param s : state
        \return true if this and s have same active process, false otherwise
        */
        bool operator== (tchecker::por::pure_local::state_t const & s) const;
      
        /*!
        \brief Disequality predicate
        \param s : state
        \return true if this and s do not have the same active process, false otherwise
        */
        bool operator!= (tchecker::por::pure_local::state_t const & s) const;
      private:
        tchecker::process_id_t _pl_pid;   /*!< pid of active process */
      };
    
    
      /*!
      \brief Hash
      \param s : state
      \return hash value for s
      */
      std::size_t hash_value(tchecker::por::pure_local::state_t const & s);
    
    
      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 has smaller active process than s2, 0 if same active processes, >0 otherwise
      */
      int lexical_cmp(tchecker::por::pure_local::state_t const & s1, tchecker::por::pure_local::state_t const & s2);
    
    
    
    
      /*!
      \brief Make state a POR pure local state
      \tparam STATE : type of state
      \note make_state_t<STATE> derives both from STATE and from tchecker::por::pure_local::state_t
      */
      template <class STATE>
      class make_state_t : public STATE, public tchecker::por::pure_local::state_t {
      public:
        using STATE::STATE;
        
        /*!
        \brief Constructor
        \param s : a state
        \param args : arguments to a constructor STATE::STATE(s, args...)
        */
        template <class ... ARGS>
        make_state_t(tchecker::por::pure_local::make_state_t<STATE> const & s, ARGS && ... args)
        : STATE(s, args...), tchecker::por::pure_local::state_t(s)
        {}
      };
    
    
      /*!
      \brief Equality predicate
      \param s1 : state
      \param s2 : state
      \return true if s1 and s2 are equal w.r.t. both STATE and tchecker::por::pure_local::state_t, false otherwise
      */
      template <class STATE>
      bool operator== (tchecker::por::pure_local::make_state_t<STATE> const & s1,
                      tchecker::por::pure_local::make_state_t<STATE> const & s2)
      {
        return
        ((static_cast<tchecker::por::pure_local::state_t const &>(s1) == static_cast<tchecker::por::pure_local::state_t const &>(s2)) &&
        (static_cast<STATE const &>(s1) == static_cast<STATE const &>(s2)));
      }
    
    
      /*!
      \brief Disequality predicate
      \param s1 : state
      \param s2 : state
      \return false if s1 and s2 are equal w.r.t. both STATE and tchecker::por::pure_local::state_t, true otherwise
      */
      template <class STATE>
      bool operator!= (tchecker::por::pure_local::make_state_t<STATE> const & s1,
                      tchecker::por::pure_local::make_state_t<STATE> const & s2)
      {
        return ! (s1 == s2);
      }
    
    
      /*!
      \brief Hash
      \param s : state
      \return hash code for s
      */
      template <class STATE>
      std::size_t hash_value(tchecker::por::pure_local::make_state_t<STATE> const & s)
      {
        std::size_t h = tchecker::por::pure_local::hash_value(static_cast<tchecker::por::pure_local::state_t const &>(s));
        boost::hash_combine(h, hash_value(static_cast<STATE const &>(s)));
        return h;
      }
    
    
      /*!
      \brief Lexical ordering
      \param s1 : state
      \param s2 : state
      \return <0 if s1 is lexically smaller than s2, 0 if s1 and s2 are equal, >0 otherwise
      */
      template <class STATE>
      int lexical_cmp(tchecker::por::pure_local::make_state_t<STATE> const & s1, tchecker::por::pure_local::make_state_t<STATE> const & s2)
      {
        int cmp = lexical_cmp(static_cast<STATE const &>(s1), static_cast<STATE const &>(s2));
        if (cmp != 0)
          return cmp;
        return tchecker::por::pure_local::lexical_cmp(static_cast<tchecker::por::pure_local::state_t const &>(s1),
        static_cast<tchecker::por::pure_local::state_t const &>(s2));
      }
    
    } // end of namespace pure_local

  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_PURE_LOCAL_STATE_HH
