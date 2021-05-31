/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR1_BUILDER_HH
#define TCHECKER_POR_POR1_BUILDER_HH

#include <algorithm>
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
#include "tchecker/por/por1/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing por1 POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace por1 {

#define PARTIAL_SYNC_ALLOWED  // allow group of processes

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::por1::state_t
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
        states_builder_t(MODEL & model, std::string const & server, TS & ts, ALLOCATOR & allocator)
        : _ts(ts),
        _allocator(allocator),
        _server_pid(model.system().processes().key(server)),
        _location_next_syncs(tchecker::location_next_syncs(model.system())),
#ifdef PARTIAL_SYNC_ALLOWED
        _pure_local_map(tchecker::pure_local_map(model.system(), tchecker::client_server_groups(model.system(), _server_pid))), 
#else
        _pure_local_map(tchecker::pure_local_map(model.system())),
#endif
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
        (tchecker::por::por1::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::por1::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por1::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por1::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::por1::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por1::states_builder_t<TS, ALLOCATOR> &&)
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

            if (! synchronizable(state))
              continue;

            state->por_memory(tchecker::por::por1::NO_SELECTED_PROCESS);

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
          if (s->por_memory() == tchecker::por::por1::NO_SELECTED_PROCESS)
            next_no_selected_process(s, v);
          else
            next_current_process(s, v);
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
         \return true if s->por_mem() corresponds to a process id and the next 
         synchronization of this process is not feasible, false otherwise
         */
        bool cut(state_ptr_t & s) const
        {
          // mem is no selected process
          if(s->por_memory() == NO_SELECTED_PROCESS)
            return false;
          boost::dynamic_bitset<> const & next_server_sync = _location_next_syncs.next_syncs(s->vloc()[_server_pid]->id(),
                                                  location_next_syncs_t::next_type_t::NEXT_SYNC_REACHABLE);        
          boost::dynamic_bitset<> next_active_sync = _location_next_syncs.next_syncs(s->vloc()[s->por_memory()]->id(),
                                                  location_next_syncs_t::next_type_t::NEXT_SYNC_REACHABLE);      
          // active pid doesn't have a sync
          if(next_active_sync.none())
            return false;   
          next_active_sync &=  next_server_sync;
          return next_active_sync.none();                                  
        }

        /*!
         \brief Checks if a state can reach a communication
         \param s : state
         \return true if a communication is reachable, false otherwise
         \note state with rank == tchecker::por::por1::communication can
         trivially reach a communication. Other states, where only
         process s->por_active_pid() is allowed to do local actions, can reach a
         communication action if there is a communication that is feasible by
         the server process and reachable (through local actions)
         for process s.por_active_pid()
         */
        bool synchronizable(state_ptr_t & s) const
        {
          // all server synchronizations reachable
          boost::dynamic_bitset<> const & server_sync = _location_next_syncs.next_syncs(s->vloc()[_server_pid]->id(),
                                                  location_next_syncs_t::next_type_t::ALL_SYNC_REACHABLE);              
          for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
            auto const * location = *it;
            if (location->pid() != _server_pid) { 
              boost::dynamic_bitset<> synchronizable_process = 
                _location_next_syncs.next_syncs(location->id(),
                location_next_syncs_t::next_type_t::NEXT_SYNC_REACHABLE);
            if (synchronizable_process.none())
              continue;
            synchronizable_process &= server_sync;
            if (synchronizable_process.none())
                return false;
            }
          }
          return true;
        }

        /*!
        \brief Computes next states for state s with no selected process
        \param s : a state
        \param v : states container
        \post all successor states of s have been pushed back in v
        */
        void next_no_selected_process(state_ptr_t & s,
                                      std::vector<state_ptr_t> & v)
        {
          assert(s->por_memory() == tchecker::por::por1::NO_SELECTED_PROCESS);
          
          // 1. Calculer l'ensemble des processus pure locaux dans s->vloc()
#ifdef PARTIAL_SYNC_ALLOWED
          std::set<tchecker::process_id_t> pure_local_processes;
          for (auto it = s->vloc().begin(); it != s->vloc().end(); ++it)
            pure_local_processes.insert(_group_id[(*it)->pid()]);
          for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
            auto const * location = *it;
            if(! _pure_local_map.is_pure_local(location->id()))
              pure_local_processes.erase(_group_id[location->pid()]);  
          }
#else
          std::set<tchecker::process_id_t> pure_local_processes;
          for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
              auto const * location = *it;
              if(_pure_local_map.is_pure_local(location->id()))
                pure_local_processes.insert(location->pid());
          }
#endif
          // 2. Calculer l'ensemble E' = {(next_state, pid),...} des transitions
          //    enabled (next state, PID du processus)
          //    Prendre PID du processus = int max si pas edge local
          std::vector<std::tuple<state_ptr_t, std::set<tchecker::process_id_t>>> enabled;
          std::set<tchecker::process_id_t> enabled_processes;
          auto outgoing_vedges = _ts.outgoing_edges(*s);
          for (auto it = outgoing_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();
            
            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            if (! synchronizable(next_state))
              continue;

#ifdef PARTIAL_SYNC_ALLOWED
            std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(vedge);
            for (tchecker::process_id_t pid : vedge_pids) {
              enabled_processes.insert(_group_id[pid]);
            }
#else
            std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(vedge);
            enabled_processes.insert(vedge_pids.begin(), vedge_pids.end());
#endif //PARTIAL_SYNC_ALLOWED

            enabled.push_back(std::make_tuple(next_state, vedge_pids));
          }

          // 3. Déterminer le plus petit i tq i est pure local et a un edge
          //    enabled
          std::set<tchecker::process_id_t> pure_local_enabled;
          std::set_intersection(enabled_processes.begin(), 
                                enabled_processes.end(),
                                pure_local_processes.begin(),
                                pure_local_processes.end(),
                                std::inserter(pure_local_enabled,
                                pure_local_enabled.begin()));
          
          tchecker::process_id_t min_pure_local = tchecker::por::por1::NO_SELECTED_PROCESS;
          if (! pure_local_enabled.empty())
            min_pure_local = *std::min_element(pure_local_enabled.begin(),
            pure_local_enabled.end());
          bool pure_local_move = (min_pure_local != tchecker::por::por1::NO_SELECTED_PROCESS);

          // 4. Mettre dans v
          //    - les next_states du process i s'il y en a un
          //    - tous les next_states sinon
          for (auto && [next_state, vedge_pids] : enabled) {
            if (in_source(vedge_pids, min_pure_local)) {
              next_state->por_memory
              (update_memory(s->por_memory(),
                             pure_local_move,
                             vedge_pids));
                        // Check if s is a deadlock due to memory 
              if (cut(next_state)) {
                continue;
              }
              v.push_back(next_state);
            }
          }
        }

        /*!
        \brief Computes next states for state s with selected process
        \param s : a state
        \param v : states container
        \post all successor states of s have been pushed back in v
        */
        void next_current_process(state_ptr_t & s, std::vector<state_ptr_t> & v)
        {
          assert(s->por_memory() != tchecker::por::por1::NO_SELECTED_PROCESS);
          auto outgoing_vedges = _ts.outgoing_edges(*s);
          for (auto it = outgoing_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            std::set<tchecker::process_id_t> vedge_pids
            = tchecker::vedge_pids(vedge);

            if (! in_source(vedge_pids, s->por_memory()))
              // current process not involved in vedge
              continue;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            if (! synchronizable(next_state))
              continue;

            bool pure_local_move = _pure_local_map.is_pure_local(s->vloc()[s->por_memory()]->id());
            next_state->por_memory(update_memory(s->por_memory(),
                                   pure_local_move, 
                                   vedge_pids)); 
            if (cut(next_state)) {
                continue;
              }
            v.push_back(next_state);
          }
        }

        /*!
        \brief Checks if a vedge is enabled w.r.t. selected process
        \param vedge_pids : PIDs of processes involved in vedge
        \param selected_process : process that is active
        \return true if vedge_pids involves the selected process or if no
        process is selected, false otherwise
        */
        bool in_source(std::set<tchecker::process_id_t> const & vedge_pids,
                       tchecker::process_id_t selected_process)
        {
          if (selected_process == tchecker::por::por1::NO_SELECTED_PROCESS)
            return true;
#ifdef PARTIAL_SYNC_ALLOWED
          for (tchecker::process_id_t pid : vedge_pids)
            if (pid != _server_pid && _group_id[pid] == selected_process)
              return true;
          return false;
#else
          return vedge_pids.find(selected_process) != vedge_pids.end();
#endif // PARTIAL_SYNC_ALLOWED
        }

        /*!
        \brief Returns the updated memory depending on the edge taken
        \param current_memory : memory before taking edge
        \param pure_local_move : true if edge is from a pure local state,
        false otherwise
        \param vedge_pids : PIDS of processes involved in the edge
        \return tchecker::por::por1::NO_SELECTED_PROCESS if server move,
        current_memory if move from a pure local state,
        the client pid in vedge_pids otherwise (local move from non pure local state)
        */
        tchecker::process_id_t update_memory(tchecker::process_id_t current_memory,
                                          bool pure_local_move,
                                          std::set<tchecker::process_id_t> const & vedge_pids)
        {
            if (vedge_pids.find(_server_pid) != vedge_pids.end()) // server move
              return tchecker::por::por1::NO_SELECTED_PROCESS;
            if (pure_local_move)
              return current_memory;
#ifdef PARTIAL_SYNC_ALLOWED
            return _group_id[*vedge_pids.begin()];
#else
            assert(vedge_pids.size() == 1); // local edge
            return *vedge_pids.begin();
#endif
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::process_id_t _server_pid; /*!< PID of server process */        
        tchecker::location_next_syncs_t _location_next_syncs; /*!< Next synchronisations */
        tchecker::pure_local_map_t _pure_local_map; /*!< Pure local map */
#ifdef PARTIAL_SYNC_ALLOWED
        std::vector<tchecker::process_id_t> _group_id;  /*!< Map : process ID -> group ID */
        std::vector<std::set<tchecker::process_id_t>> _groups; /*!< Map : group ID -> process IDs */
        tchecker::clock_id_t _refcount;  /*!< Number of reference clocks */
        tchecker::clock_id_t _offset_dim; /*!< Dimension of offset DBMs */
#endif // PARTIAL_SYNC_ALLOWED
      };

    } // end of namespace por1

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR1_BUILDER_HH
