/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_GL_TS_HH
#define TCHECKER_POR_GL_TS_HH

#include <algorithm>
#include <limits>
#include <type_traits>

#include "tchecker/basictypes.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/por/ts.hh"
#include "tchecker/system/static_analysis.hh"

/*!
 \file ts.hh
 \brief Transition system with POR for global/local systems
 */

#define PARTIAL_SYNCS_ALLOWED  // allows partial synchronizations in global/local systems

namespace tchecker {
  
  namespace por {
    
    namespace gl {
      
      /*! Rank value of global transitions */
      constexpr tchecker::process_id_t const global = std::numeric_limits<tchecker::process_id_t>::max();
      
      
      
      
      /*!
       \brief Compute  process involved in a vedge
       \param vedge : a vedge
       \pre system is global/local, i.e. vedge involves either one process (local) or all the processes (global)
       \return the identifier of active process in vedge if vedge is local, otherwise returns tchecker::por::gl:::global if vedge is global
       \throw std::runtime_error : if vedge is empty
       */
      template <class VEDGE>
      tchecker::process_id_t vedge_pid(VEDGE const & vedge)
      {
        auto it = vedge.begin(), end = vedge.end();
        
        if (it == end) // empty vedge
          throw std::runtime_error("Empty vedge");
        
        tchecker::process_id_t first_pid = (*it)->pid();
        
        ++it;
        if (it == end) // local vedge
          return first_pid;
        
        // global vedge
        return tchecker::por::gl::global;
      }
      
      
      
      /*!
       \brief Compute the size of a vedge
       \param vedge : a vedge
       \return number of transitions in vedge
       */
      template <class VEDGE>
      std::size_t vedge_size(VEDGE const & vedge)
      {
        std::size_t vedge_size = 0;
        auto end = vedge.end();
        for (auto it = vedge.begin(); it != end; ++it)
          ++vedge_size;
        return vedge_size;
      }
      
      
      
      /*!
       \class source_set_t
       \brief Source set for global/local systems
       \tparam STATE : type of state, should derive from tchecker::por::state_t
       */
      template <class STATE>
      class source_set_t {
        
        static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                      "STATE should derive from tchecker::por::state_t");
        
      public:
        /*!
         \brief Membership predicate
         \tparam VEDGE : type of vedge
         \param s : state
         \param vedge : a vedge
         \return true if vedge shoud be included in the source set of s, false otherwise
         \note vedge is in source_set(s) if either vedge is global or vedge is local to a process >= rank of s
         */
        template <class VEDGE>
        bool operator() (STATE const & s, VEDGE const & vedge)
        {
#ifdef PARTIAL_SYNCS_ALLOWED
          // Partial syncs are considered local and allowed if all processes above s.rank()
          if (s.rank() == tchecker::por::gl::global)
            return true;
          std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(vedge);
          return ((tchecker::por::gl::vedge_size(vedge) == s.vloc().size())                    // global
                  || (* std::min_element(vedge_pids.begin(), vedge_pids.end())) >= s.rank());
#else
          return (s.rank() == tchecker::por::gl::global) || (tchecker::por::gl::vedge_pid(vedge) >= s.rank());
#endif // PARTIAL_SYNCS_ALLOWED
        }
      };
      
      
      
      
      /*!
       \brief Transition system with partial-order reduction for global/local systems
       \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
       \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
       \tparam SOURCE_SET : source set, should provide membership operator
       bool operator() (STATE const &, tchecker::vedge_iterator_t const &)
       \note ts_t<TS> implements partial-order reduction on top of TS, using global/local source sets
       */
      template <class TS, class STATE>
      class ts_t final : public tchecker::por::ts_t<TS, STATE, tchecker::por::gl::source_set_t<STATE>> {
        using base_ts_t = tchecker::por::ts_t<TS, STATE, tchecker::por::gl::source_set_t<STATE>>;
      public:
        /*!
         \brief Constructor
         \param model : a model
         \throw std::invalid_argument : if model has partial synchronizations (only if compiled with LOCAL_ONE_PROCESS)
         \note TS should have a constructor TS(MODEL &)
         */
        template <class MODEL, class ... ARGS>
        ts_t(MODEL & model, ARGS && ... args)
        : base_ts_t(model, args...),
#ifdef PARTIAL_SYNCS_ALLOWED
        _location_next_syncs(tchecker::location_next_global_syncs(model.system())),
        _group_id(model.system().processes_count(), tchecker::por::gl::global)
#else
        _location_next_syncs(tchecker::location_next_syncs(model.system()))
#endif // PARTIAL_SYNCS_ALLOWED
        {
#ifdef PARTIAL_SYNCS_ALLOWED
          compute_groups_from_non_global_synchronizations(model.system(), _group_id);
#else
          if (! tchecker::global_local(model.system()))
            throw std::invalid_argument("System is not global/local");
#endif // PARTIAL_SYNCS_ALLOWED
        }
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::gl::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Copy constructor
         */
        ts_t(tchecker::por::gl::ts_t<TS, STATE> &&) = default;
        
        /*!
         \brief Destructor
         */
        ~ts_t() = default;
        
        /*!
         \brief Assignment operator
         */
        tchecker::por::gl::ts_t<TS, STATE> & operator= (tchecker::por::gl::ts_t<TS, STATE> const &) = default;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::gl::ts_t<TS, STATE> & operator= (tchecker::por::gl::ts_t<TS, STATE> &&) = default;
        

        /*!
         \brief Initialize state
         \param s : state
         \param t : transition
         \param v : initial state valuation
         \post state s and transtion t have been initialized from v
         \return status of state s after initialization
         \note t represents an initial transition to s
         */
        virtual enum tchecker::state_status_t initialize(STATE & s,
                                                         typename TS::transition_t & t,
                                                         typename TS::initial_iterator_value_t const & v)
        {
          enum tchecker::state_status_t status = base_ts_t::initialize(s, t, v);
          if (status != tchecker::STATE_OK)
            return status;
          
          s.rank(tchecker::por::gl::global);
          
          if (! synchronizable(s))
            return tchecker::STATE_POR_DISABLED;
          
          return tchecker::STATE_OK;
        }
        
        
        /*!
         \brief Next state computation
         \param s : state
         \param t : transition
         \param v : outgoing edge value
         \post s and t have been updated from v
         \return status of state s after update
         */
        virtual enum tchecker::state_status_t next(STATE & s,
                                                   typename TS::transition_t & t,
                                                   typename TS::outgoing_edges_iterator_value_t const & v)
        {
          enum tchecker::state_status_t status = base_ts_t::next(s, t, v);
          if (status != tchecker::STATE_OK)
            return status;
         
#ifdef PARTIAL_SYNCS_ALLOWED
          std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(v);
          if (vedge_pids.size() == s.vloc().size())
            s.rank(tchecker::por::gl::global);
          else
            s.rank(_group_id[* vedge_pids.begin()]);
#else
          s.rank(tchecker::por::gl::vedge_pid(v));
#endif // PARTIAL_SYNCS_ALLOWED
          
          if ((s.rank() != tchecker::por::gl::global) && (! synchronizable(s)))
            return tchecker::STATE_POR_DISABLED;
          
          return tchecker::STATE_OK;
        }
      private:
        /*!
         \brief Checks if a state can reach a global action
         \param s : state
         \return true if all processes can reach a common global action from s, false otherwise
         \note there is a reachable global action from s if all processes < s.rank() have a common global action that is also
         reachable from s for all processes >= s.rank()
         */
        bool synchronizable(STATE const & s) const
        {
          return tchecker::por::synchronizable_global(s.vloc(),
                                                      (s.rank() == tchecker::por::gl::global ? 0 : s.rank()),
                                                      _location_next_syncs);
        }
        
        /*!
         \brief Compute process groups from synchronization vectors
         \param system : system of processes
         \param group_id : map process IDs to group IDs
         \post group_id maps each process ID to the smallest process ID in its synchronization group. Two processes p and q are
         in the same group if there is a path of synchronizations from p to q
         */
        template <class SYSTEM>
        void compute_groups_from_non_global_synchronizations(SYSTEM const & system,
                                                             std::vector<tchecker::process_id_t> & group_id) const
        {
          // IMPLEMENTATION NOTE: non-global synchronizations define group of processes that
          // synchronize on local actions (i.e. actions that are local to the group of processes).
          // To each group, we associate a group ID (the smallest PID in the group). The group ID
          // is used as state rank
          tchecker::process_id_t processes_count = system.processes_count();
          
          // Initialization: each process belongs to its own group
          for (tchecker::process_id_t pid = 0; pid < processes_count; ++pid)
            group_id[pid] = pid;
          
          // From each synchronization vector: each process belongs to the group of the smallest process
          // it synchronizes with
          for (tchecker::synchronization_t const & sync : system.synchronizations()) {
            if (sync.size() == processes_count) // global
              continue;
            tchecker::process_id_t sync_group_id = smallest_pid(sync);
            for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
              group_id[constr.pid()] = std::min(group_id[constr.pid()], sync_group_id);
          }
          
          // Propagation: by transitivity of synchronizations
          bool modified;
          do {
            modified = false;
            for (tchecker::process_id_t pid = 0; pid < processes_count; ++pid)
              if (group_id[pid] != group_id[group_id[pid]]) {
                group_id[pid] = group_id[group_id[pid]];
                modified = true;
              }
          }
          while (modified);
        }
        
        /*!
         \brief Compute the smallest PID in a synchronization
         \param sync : synchronization
         \return smallest process identifier involved in sync
         */
        tchecker::process_id_t smallest_pid(tchecker::synchronization_t const & sync) const
        {
          tchecker::process_id_t pid = std::numeric_limits<tchecker::process_id_t>::max();
          for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
            pid = std::min(pid, constr.pid());
          return pid;
        }
        
        tchecker::location_next_syncs_t _location_next_syncs;  /*!< Next synchronisations */
#ifdef PARTIAL_SYNCS_ALLOWED
        std::vector<tchecker::process_id_t> _group_id;         /*!< Map PID -> group ID */
#endif // PARTIAL_SYNCS_ALLOWED
      };
    
 
 
      
      /*!
       \brief Covering check
       \param s1 : state
       \param s2 : state
       \return true if s1 can be covered by s1, false othewise
       */
      bool cover_leq(tchecker::por::state_t const & s1, tchecker::por::state_t const & s2);
      
      
    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_GL_TS_HH
