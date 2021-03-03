/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_CS_BUILDER_HH
#define TCHECKER_POR_CS_BUILDER_HH

#include <cassert>
#include <limits>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "tchecker/algorithms/covreach/builder.hh"
#include "tchecker/basictypes.hh"
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/cs/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing client-server POR for covreach algorithm
 */

namespace tchecker {
  
  namespace por {
    
    namespace cs {

#define PARTIAL_SYNC_ALLOWED  // allow group of processes
        
      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::cs::state_t
      */
      template <class TS, class ALLOCATOR>
      class states_builder_t final
      : public tchecker::covreach::states_builder_t<typename ALLOCATOR::state_ptr_t> {
      public:
        /*!
        \brief Type of pointers to state
        */
        using state_ptr_t = typename ALLOCATOR::state_ptr_t;
      
        /*!
        \brief Type of pointers to transition
        */
        using transition_ptr_t = typename ALLOCATOR::transition_ptr_t;
      
        /*!
        \brief Type of transition system
        */
        using ts_t = TS;

        /*!
        \brief Type of allocator
        */
        using allocator_t = ALLOCATOR;
      
        /*!
        \brief Constructor
        \tparam MODEL : type of model
        \param model : a model
        \param server : name of server process
        \param ts : transition built on top of model
        \param allocator : an allocator
        \note see tchecker::ts::builder_ok_t
        */
        template <class MODEL>
        states_builder_t(MODEL & model, std::string const & server, TS & ts, 
                         ALLOCATOR & allocator)
        : _ts(ts),
        _allocator(allocator),
        _server_pid(model.system().processes().key(server)),
#ifdef PARTIAL_SYNC_ALLOWED
        _location_next_syncs(tchecker::location_next_server_syncs(model.system(), _server_pid)),
#else
        _location_next_syncs(tchecker::location_next_syncs(model.system())),
#endif // PARTIAL_SYNC_ALLOWED
        _refcount(model.flattened_offset_clock_variables().refcount()),
        _offset_dim(model.flattened_offset_clock_variables().flattened_size())
        {
#ifdef PARTIAL_SYNC_ALLOWED
          _group_id = tchecker::client_server_groups(model.system(), _server_pid);
          compute_groups(model.system().processes_count());
          assert(_refcount == model.system().processes_count());
#else
          if (! tchecker::client_server(model.system(), _server_pid))
            throw std::invalid_argument("System is not client/server");
#endif // PARTIAL_SYNC_ALLOWED
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::cs::states_builder_t<TS, ALLOCATOR> const &) = default;
        
        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::cs::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::cs::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::cs::states_builder_t<TS, ALLOCATOR> const &) 
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::cs::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::cs::states_builder_t<TS, ALLOCATOR> &&) 
        = default;

        /*!
        \brief Computes initial states
        \param v : states container
        \post all initial states have been pushed back in v
        */
        virtual void initial(std::vector<state_ptr_t> & v)
        {
          auto initial_vedges = _ts.initial();
          for (auto it = initial_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            state_ptr_t state = _allocator.construct_state();
            transition_ptr_t transition = _allocator.construct_transition();
        
            tchecker::state_status_t status 
            = _ts.initialize(*state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

#ifdef PARTIAL_SYNC_ALLOWED
            // Synchronize reference clocks from the same group
            if (synchronize_groups(state) != tchecker::STATE_OK)
              continue;
#endif // PARTIAL_SYNC_ALLOWED

            state->por_active_pid(tchecker::por::cs::communication);

            if (! synchronizable(state))
              continue;

            v.push_back(state);
          }
        }

        /*!
        \brief Computes next states
        \param s : a state
        \param v : states container
        \post all successor states of s have been pushed back in v
        */
        virtual void next(state_ptr_t & s, std::vector<state_ptr_t> & v)
        {
          auto outgoing_vedges = _ts.outgoing_edges(*s);
          for (auto it = outgoing_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            std::set<tchecker::process_id_t> vedge_pids 
            = tchecker::vedge_pids(vedge);

            if (! in_source_set(s, vedge_pids))
              continue;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();
        
            tchecker::state_status_t status 
            = _ts.next(*next_state, *transition, vedge);
        
            if (status != tchecker::STATE_OK)
              continue;

#ifdef PARTIAL_SYNC_ALLOWED
            // Synchronize reference clocks from the same group
            if (synchronize_groups(next_state) != tchecker::STATE_OK)
              continue;
#endif // PARTIAL_SYNC_ALLOWED
            
            next_state->por_active_pid(compute_active_pid(vedge_pids));

            if (! synchronizable(next_state))
              continue;

            v.push_back(next_state);
          }
        }
      private:
        /*!
         \brief Compute map : group ID -> process IDs from map : process ID -> group ID
         \post _groups is the dual map of _group_id
         */
        void compute_groups(tchecker::process_id_t processes_count)
        {
          for (tchecker::process_id_t pid = 0; pid < processes_count; ++pid) {
            tchecker::process_id_t gid = _group_id[pid];
            if (gid >= _groups.size())
              _groups.resize(gid + 1);
            _groups[gid].insert(pid);
          }
        }

        /*!
         \brief Synchronise reference clocks that belong to the same group
         \param s : a state
         \post the reference clocks that belong to the same group have been synchronised in the zone of s
         \return tchecker::STATE_OK if the resulting zone is not empty,  tchecker::STATE_EMPTY_ZONE otherwise
         */
        enum tchecker::state_status_t synchronize_groups(state_ptr_t & s) const
        {
          tchecker::dbm::db_t * offset_dbm = s->offset_zone_ptr()->dbm();
          for (tchecker::clock_id_t r = 0; r < _refcount; ++r) {
            if (r == _group_id[r])
              continue;
            
            auto status = tchecker::offset_dbm::constrain(offset_dbm, 
            _offset_dim, r, _group_id[r], tchecker::dbm::LE, 0);
            if (status == tchecker::dbm::EMPTY)
              return tchecker::STATE_EMPTY_ZONE;
            
            status = tchecker::offset_dbm::constrain(offset_dbm, _offset_dim, 
            _group_id[r], r, tchecker::dbm::LE, 0);
            if (status == tchecker::dbm::EMPTY)
              return tchecker::STATE_EMPTY_ZONE;
          }
          
          return tchecker::STATE_OK;
        }
        
        /*!
         \brief Checks if a state can reach a communication
         \param s : state
         \return true if a communication is reachable, false otherwise
         \note state with rank == tchecker::por::cs::communication can 
         trivially reach a communication. Other states, where only
         process s->por_active_pid() is allowed to do local actions, can reach a
         communication action if there is a communication that is feasible by
         the server process and reachable (through local actions) 
         for process s.por_active_pid()
         */
        bool synchronizable(state_ptr_t & s) const
        {
          if (s->por_active_pid() == tchecker::por::cs::communication)
            return true;
#ifdef PARTIAL_SYNC_ALLOWED
          return tchecker::por::synchronizable_group_server
          (s->vloc(), _groups[s->por_active_pid()], _server_pid,
          _location_next_syncs);
#else
          return tchecker::por::synchronizable_server(s->vloc(), 
          s->por_active_pid(), _server_pid, _location_next_syncs);
#endif
        }

        /*!
        \brief Compute next active process identifier from a vedge
        \param vedge_pids : set of process identifiers in a vedge
        \return the identifier of the active process after taking a vedge
        involving processes vedge_pids
        */
        tchecker::process_id_t compute_active_pid
        (std::set<tchecker::process_id_t> const & vedge_pids) const
        {
#ifdef PARTIAL_SYNC_ALLOWED
          tchecker::process_id_t active_pid = tchecker::por::cs::communication;
          for (tchecker::process_id_t pid : vedge_pids) {
            if (pid == _server_pid) {
              active_pid = tchecker::por::cs::communication;
              break;
            }
            else
              active_pid = _group_id[pid];
          }
          return active_pid;
#else
          if (vedge_pids.size() < 2) // not a communication
            return * vedge_pids.begin();
          return tchecker::por::cs::communication;          
#endif // PARTIAL_SYNC_ALLOWED
        }

        /*!
         \brief Source set selection
         \param state : a state
         \param vedge_pids : process identifiers in a vedge
         \return true if a vedge involving processes vedge_pids is in the source
         set of state, false otherwise
         \note a vedge is in source_set(state) if either a communication has
         just happened, i.e.:
         (state->por_active_pid() == tchecker::por::cs::communication)
         or if vedge is a local or communication action of current active
         process, i.e. state->por_active_pid() belongs vedge_pids
         */
        bool in_source_set(state_ptr_t & state, 
        std::set<tchecker::process_id_t> const & vedge_pids)
        {
#ifdef PARTIAL_SYNC_ALLOWED
          if (state->por_active_pid() == tchecker::por::cs::communication)
            return true;
          for (tchecker::process_id_t pid : vedge_pids)
            if (pid != _server_pid && _group_id[pid] == state->por_active_pid())
              return true;
          return false;
#else
          return (s.por_active_pid() == tchecker::por::cs::communication || 
                  vedge_pids.count(s.por_active_pid()) >= 1);
#endif // PARTIAL_SYNC_ALLOWED
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::process_id_t _server_pid;  /*!< pid of server process */
        tchecker::location_next_syncs_t _location_next_syncs; /*!< Next synchronisations */
#ifdef PARTIAL_SYNC_ALLOWED
        std::vector<tchecker::process_id_t> _group_id;  /*!< Map : process ID -> group ID */
        std::vector<std::set<tchecker::process_id_t>> _groups; /*!< Map : group ID -> process IDs */
        tchecker::clock_id_t _refcount;  /*!< Number of reference clocks */
        tchecker::clock_id_t _offset_dim; /*!< Dimension of offset DBMs */
#endif // PARTIAL_SYNC_ALLOWED
      };

    } // end of namespace cs
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_CS_BUILDER_HH
