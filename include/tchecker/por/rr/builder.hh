/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_RR_BUILDER_HH
#define TCHECKER_POR_RR_BUILDER_HH

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
#include "tchecker/por/rr/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing rr POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace rr {

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::rr::state_t
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
        _pure_local_map(tchecker::pure_local_map(model.system())),
        _read_events(model.system().events().size()),
        _location_next_syncs(tchecker::location_next_syncs(model.system())),
        _mixed_map(tchecker::mixed_map(model.system()))
        {
          if (! tchecker::client_server(model.system(), _server_pid))
				    throw std::invalid_argument("System is not client/server");
          auto const & event_index = model.system().events();
          // Set all read events
          for (auto && [id, name] : event_index)
            if (name[0] == '!') 
              _read_events[id] = 1;
          for (auto && [id, name] : event_index){
            if(_read_events[id])
              std::cout << "read event: " << name << std::endl;
          }
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::rr::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::rr::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::rr::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::rr::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::rr::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::rr::states_builder_t<TS, ALLOCATOR> &&)
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

            tchecker::process_id_t next_mem = s->por_memory();

            if (! in_source_set(s, vedge_pids,next_mem,vedge))
              continue;
            
            tchecker::process_id_t active_pid = compute_active_pid(vedge_pids);

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            if (! synchronizable(next_state))
              continue;

            //Update memory
            next_state->mixed_local(NO_SELECTED_PROCESS);
            next_state->por_memory(next_mem);

            // Duplicate next state if location reached by vedge is mixed 
            if (_mixed_map.is_mixed(next_state->vloc()[active_pid]->id())){
              state_ptr_t next_state_bis = _allocator.construct_from_state(next_state);
              next_state_bis->mixed_local(active_pid);
              v.push_back(next_state_bis);
            }
            v.push_back(next_state);
          }
        }
      private:


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
        \brief Compute next active process identifier from a vedge
        \param vedge_pids : set of process identifiers in a vedge
        \return the identifier of the active process after taking a vedge
        involving processes vedge_pids
        */
        tchecker::process_id_t compute_active_pid
        (std::set<tchecker::process_id_t> const & vedge_pids) const
        {
          process_id_t active_pid = * vedge_pids.begin();
          if (vedge_pids.size() < 2) // not a communication
            return active_pid;
          for (tchecker::process_id_t pid : vedge_pids)
            if(pid != _server_pid)
               active_pid = pid;   
          return active_pid;
        }

        /*!
         \brief Source set selection
         \param state : a state
         \param vedge_pids : process identifiers in a vedge
         \param next_mem : memory of current state 
         \param vedge : a vedge 
         \return true if a vedge involving processes vedge_pids is in the source
         set of state, false otherwise. Update next_mem according to mem function.
         */
        template <class VEDGE>
        bool in_source_set(state_ptr_t & state,
        std::set<tchecker::process_id_t> const & vedge_pids,
        tchecker::process_id_t & next_mem,
        VEDGE const & vedge)
        {
          tchecker::process_id_t mixed_pid = state->mixed_local();
          tchecker::process_id_t active_pid = compute_active_pid(vedge_pids);
          // Local action of mixed state
          if (mixed_pid != NO_SELECTED_PROCESS){
            if (vedge_pids.size() == 1 && active_pid == mixed_pid)
              return true;
            return false;
          }
          // Local action of pure local process
          if (_pure_local_map.is_pure_local(state->vloc()[active_pid]->id()))
            return true;
          // Check if other pure local process
          for(auto it = state->vloc().begin(); it != state->vloc().end(); ++it) {
            auto const * location = *it;
            if (_pure_local_map.is_pure_local(location->id()))
              return false;
          }
          // TODO function 
          assert (vedge_pids.size() != 1);
          // TODO une fonction qui compte le nombre de pur local
          // et assert qui dit count <= 1
          // loop because of operator *
          for (auto const * edge : vedge) {
            // Read action
            if (_read_events[edge->event_id()]) {
              if (active_pid >= state->por_memory()){
                next_mem = active_pid;
                return true;
              }
              return false;
            }
            // Write action
            next_mem = 0;
            return true;
          }
          return false;
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */        
        tchecker::process_id_t _server_pid; /*!< PID of server process */
        tchecker::pure_local_map_t _pure_local_map; /*!< Pure local map */
        boost::dynamic_bitset<> _read_events; /*!< set of read events */        
        tchecker::location_next_syncs_t _location_next_syncs; /*!< Next synchronisations */
        tchecker::mixed_map_t _mixed_map; /*!< Pure local map */
      };

    } // end of namespace rr

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_RR_BUILDER_HH
