/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_GL_BUILDER_HH
#define TCHECKER_POR_GL_BUILDER_HH

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
#include "tchecker/por/gl/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing global-local POR for covreach algorithm
 */

namespace tchecker {
  
  namespace por {
    
    namespace gl {

#define PARTIAL_SYNC_ALLOWED  // allow group of processes
        
      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      global-local partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::gl::state_t
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
        \param ts : transition built on top of model
        \param allocator : an allocator
        \note see tchecker::ts::builder_ok_t
        */
        template <class MODEL>
        states_builder_t(MODEL & model, TS & ts, ALLOCATOR & allocator)
        : _ts(ts),
        _allocator(allocator),
#ifdef PARTIAL_SYNCS_ALLOWED
        _location_next_syncs
        (tchecker::location_next_global_syncs(model.system())),
        _group_id(model.system().processes_count(), tchecker::por::gl::global)
#else
        _location_next_syncs(tchecker::location_next_syncs(model.system()))
#endif // PARTIAL_SYNCS_ALLOWED
        {
#ifdef PARTIAL_SYNCS_ALLOWED
          compute_groups_from_non_global_synchronizations(model.system(), 
                                        _group_id);
#else
          if (! tchecker::global_local(model.system()))
            throw std::invalid_argument("System is not global/local");
#endif // PARTIAL_SYNCS_ALLOWED
          _offset_dim 
          = model.flattened_offset_clock_variables().flattened_size();
          _offset_dbm = new tchecker::dbm::db_t[_offset_dim * _offset_dim];
          _refcount = model.flattened_offset_clock_variables().refcount();
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::gl::states_builder_t<TS, ALLOCATOR> const &) = delete;
        
        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::gl::states_builder_t<TS, ALLOCATOR> &&) = delete;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t()
        {
          delete[] _offset_dbm;
        }

        /*!
        \brief Assignment operator
        */
        tchecker::por::gl::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::gl::states_builder_t<TS, ALLOCATOR> const &) 
        = delete;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::gl::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::gl::states_builder_t<TS, ALLOCATOR> &&) 
        = delete;

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

            state->por_rank(tchecker::por::gl::global);
          
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

            if (vedge_pids.size() == next_state->vloc().size()) // global action
              next_state->por_rank(tchecker::por::gl::global);
            else
#ifdef PARTIAL_SYNCS_ALLOWED
              next_state->por_rank(_group_id[* vedge_pids.begin()]);
#else
              next_state->por_rank(* vedge_pids.begin());
#endif // PARTIAL_SYNCS_ALLOWED
          
            if (next_state->por_rank() != tchecker::por::gl::global && 
                ! synchronizable(next_state))
              continue;

            v.push_back(next_state);
          }
        }
      private:
        /*!
         \brief Source set selection
         \param s : a state
         \param vedge_pids : set of process identifiers in a vedge
         \return true if a vedge with processes vedge_pids is in the source set 
         of state s, false otherwise
         \note vedge_pids is in the source set of state s if:
         - s has por rank tchecker::por::gl::global
         - or vedge_pids is a global action
         - or vedge_pids is a local action of a process with identifier
         at least equal to por rank of state s (or a group of such processes 
         when PARTIAL_SYNC_ALLOWED)
        */
        virtual bool in_source_set
        (state_ptr_t & s, 
         std::set<tchecker::process_id_t> const & vedge_pids)
        {
          if (s->por_rank() == tchecker::por::gl::global)
            return true;
          if (vedge_pids.size() == s->vloc().size()) // global vedge
            return true;
          // local actions (or synchronizations within a group of processes when
          // PARTIAL_SYNC_ALLOWED) are allowed if all involved processes are
          // equal or above the por rank of state s
          tchecker::process_id_t min_pid
          = * std::min_element(vedge_pids.begin(), vedge_pids.end());
          return min_pid >= s->por_rank();
        }

        /*!
         \brief Checks if a state can reach a global action
         \param s : state
         \return true if all processes can reach a common global action from s, 
         false otherwise
         \note there is a reachable global action from s if all processes < s.
         por_rank() have a common global action that is also reachable from s 
         for all processes >= s.por_rank()
         */
        bool synchronizable(state_ptr_t & s)
        {
          return 
          tchecker::por::synchronizable_global
          (s->vloc(),
           (s->por_rank() == tchecker::por::gl::global ? 0 : s->por_rank()),
           _location_next_syncs
          ) 
          &&
          zone_synchronizable(s);
        }
        
        /*!
         \brief Check if the zone in a state can be partially synchronized
         \param s : state
         \return true if the zone in s can be synchronized w.r.t. processes 
         with an id lower than the rank of s, false otherwise
         */
        bool zone_synchronizable(state_ptr_t & s)
        {
          tchecker::clock_id_t refsync = std::min(s->por_rank(), _refcount);
          if (refsync == 0)
            return true;
          
          // Check that reference clocks below s.por_rank() can be synchronised
          std::memcpy(_offset_dbm, 
                      s->offset_zone().dbm(), 
                      _offset_dim * _offset_dim * sizeof(*_offset_dbm));
          enum tchecker::dbm::status_t status = 
          tchecker::offset_dbm::synchronize(_offset_dbm, _offset_dim, refsync);
          if (status == tchecker::dbm::EMPTY)
            return false;
          
          // Further check that, once all reference clocks below s.por_rank() 
          // have been synchronised, synchronization with other reference 
          // clocks is possible
          for (tchecker::clock_id_t r = s->por_rank(); r < _refcount; ++r)
            if (_offset_dbm[r * _offset_dim + 0] < tchecker::dbm::LE_ZERO)
              return false;
          
          return true;
        }

        /*!
         \brief Compute process groups from synchronization vectors
         \param system : system of processes
         \param group_id : map process IDs to group IDs
         \post group_id maps each process ID to the smallest process ID in its synchronization group. Two processes p and q are
         in the same group if there is a path of synchronizations from p to q
         */
        template <class SYSTEM>
        void compute_groups_from_non_global_synchronizations
        (SYSTEM const & system,
         std::vector<tchecker::process_id_t> & group_id) const
        {
          // IMPLEMENTATION NOTE: non-global synchronizations define group of 
          // processes that synchronize on local actions (i.e. actions that are 
          // local to the group of processes).
          // To each group, we associate a group ID (the smallest PID in the 
          // group). The group ID is used as state rank
          tchecker::process_id_t processes_count = system.processes_count();
          
          // Initialization: each process belongs to its own group
          for (tchecker::process_id_t pid = 0; pid < processes_count; ++pid)
            group_id[pid] = pid;
          
          // From each synchronization vector: each process belongs to the 
          // group of the smallest process it synchronizes with
          for (tchecker::synchronization_t const & sync : system.synchronizations()) {
            if (sync.size() == processes_count) // global
              continue;
            tchecker::process_id_t sync_group_id = smallest_pid(sync);
            for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
              group_id[constr.pid()] = std::min(group_id[constr.pid()], 
                                                sync_group_id);
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
        tchecker::process_id_t smallest_pid
        (tchecker::synchronization_t const & sync) const
        {
          tchecker::process_id_t pid = 
          std::numeric_limits<tchecker::process_id_t>::max();
          for (tchecker::sync_constraint_t const & constr : sync.synchronization_constraints())
            pid = std::min(pid, constr.pid());
          return pid;
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::location_next_syncs_t _location_next_syncs;  /*!< Next synchronisations */
        tchecker::dbm::db_t * _offset_dbm;                     /*!< Offset DBM to check synchronizability */
        tchecker::clock_id_t _offset_dim;                      /*!< Dimension of _offset_dbm */
        tchecker::clock_id_t _refcount;                        /*!< Number of reference clocks */
#ifdef PARTIAL_SYNCS_ALLOWED
        std::vector<tchecker::process_id_t> _group_id;         /*!< Map PID -> group ID */
#endif // PARTIAL_SYNCS_ALLOWED
      };

    } // end of namespace gl
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_GL_BUILDER_HH
