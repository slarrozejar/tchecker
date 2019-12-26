/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_STATE_HH
#define TCHECKER_POR_STATE_HH

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/basictypes.hh"

/*!
 \file state.hh
 \brief States for POR transition systems
 */

namespace tchecker {
  
  namespace por {
    
    extern tchecker::process_id_t const all_processes_active; /*!< Process identifier corresponding to all processes active */
    
    /*!
     \class state_t
     \brief State for POR transition systems
     */
    class state_t {
    public:
      /*!
       \brief Constructor
       \post all processes are active
       */
      state_t();
      
      /*!
       \brief Accessor
       \return true if all processes are active, false otherwise
       */
      constexpr bool all_active() const
      {
        return _active_pid == tchecker::por::all_processes_active;
      }
      
      /*!
       \brief Accessor
       \return identifier of active process, or tchecker::por::all_processes_active if all processes are active
       */
      constexpr inline tchecker::process_id_t active_pid() const
      {
        return _active_pid;
      }
      
      /*!
       \brief Set active process ID
       \param active_pid : active process ID
       \post active process identifier has been set to active_pid
       \note set to tchecker::por::all_processes_active to make all processes active
       */
      void active_pid(tchecker::process_id_t active_pid);
          
      /*!
       \brief Equality predicate
       \param s : state
       \return true if this and s have same active process, false otherwise
       */
      bool operator== (tchecker::por::state_t const & s) const;
      
      /*!
       \brief Disequality predicate
       \param s : state
       \return true if this and s do not have the same active process, false otherwise
       */
      bool operator!= (tchecker::por::state_t const & s) const;
    private:
      tchecker::process_id_t _active_pid;               /*!< Active process identifier */
    };
    
        
    /*!
     \brief Permissive state predicate
     \param s1 : state
     \param s2 : state
     \return true if all outgoing transitions from s1 exist from s2, false otherwise.
     */
    bool permissive_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
    
    
    /*!
     \brief Hash
     \param s : state
     \return hash value for s
     */
    std::size_t hash_value(tchecker::por::state_t const & s);
    
    
    /*!
     \brief Lexical ordering
     \param s1 : state
     \param s2 : state
     \return <0 if s1 has smaller active process than s2, 0 if same active processes, >0 otherwise
     */
    int lexical_cmp(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
    
    
    
    
    /*!
     \brief Make state a POR state
     \tparam STATE : type of state
     \note make_state_t<STATE> derives both from STATE and from tchecker::por::state_t
     */
    template <class STATE>
    class make_state_t : public STATE, public tchecker::por::state_t {
    public:
      using STATE::STATE;
      
      /*!
       \brief Constructor
       \param s : a state
       \param args : arguments to a constructor STATE::STATE(s, args...)
       */
      template <class ... ARGS>
      make_state_t(tchecker::por::make_state_t<STATE> const & s, ARGS && ... args)
      : STATE(s, args...), tchecker::por::state_t(s)
      {}
    };
    
    
    /*!
     \brief Equality predicate
     \param s1 : state
     \param s2 : state
     \return true if s1 and s2 are equal w.r.t. both STATE and tchecker::por::state_t, false otherwise
     */
    template <class STATE>
    bool operator== (tchecker::por::make_state_t<STATE> const & s1,
                     tchecker::por::make_state_t<STATE> const & s2)
    {
      return
      ((static_cast<tchecker::por::state_t const &>(s1) == static_cast<tchecker::por::state_t const &>(s2)) &&
       (static_cast<STATE const &>(s1) == static_cast<STATE const &>(s2)));
    }
    
    
    /*!
     \brief Disequality predicate
     \param s1 : state
     \param s2 : state
     \return false if s1 and s2 are equal w.r.t. both STATE and tchecker::por::state_t, true otherwise
     */
    template <class STATE>
    bool operator!= (tchecker::por::make_state_t<STATE> const & s1,
                     tchecker::por::make_state_t<STATE> const & s2)
    {
      return ! (s1 == s2);
    }
    
    
    /*!
     \brief Hash
     \param s : state
     \return hash code for s
     */
    template <class STATE>
    std::size_t hash_value(tchecker::por::make_state_t<STATE> const & s)
    {
      std::size_t h = tchecker::por::hash_value(static_cast<tchecker::por::state_t const &>(s));
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
    int lexical_cmp(tchecker::por::make_state_t<STATE> const & s1, tchecker::por::make_state_t<STATE> const & s2)
    {
      int cmp = lexical_cmp(static_cast<STATE const &>(s1), static_cast<STATE const &>(s2));
      if (cmp != 0)
        return cmp;
      return tchecker::por::lexical_cmp(static_cast<tchecker::por::state_t const &>(s1),
                                        static_cast<tchecker::por::state_t const &>(s2));
    }
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_STATE_HH
