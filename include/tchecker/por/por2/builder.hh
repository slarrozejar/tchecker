/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR2_BUILDER_HH
#define TCHECKER_POR_POR2_BUILDER_HH

#include <cassert>
#include <limits>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include <iostream>

#include "tchecker/algorithms/covreach/builder.hh"
#include "tchecker/basictypes.hh"
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/por2/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing por2 POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace por2 {

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::por2::state_t
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
        _processes_count(model.system().processes_count()-1)
        {}

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::por2::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::por2::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por2::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por2::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::por2::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por2::states_builder_t<TS, ALLOCATOR> &&)
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

            state->por_L().resize(_processes_count);
            state->por_S().resize(_processes_count);
            state->por_S().set();

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

            bool synchro_phase = false;

            std::set<tchecker::process_id_t> vedge_pids
            = tchecker::vedge_pids(vedge);

            if (s->por_L().none())
              if (! in_source_synchro_phase(s, vedge_pids))
                continue;
              else
                synchro_phase = true;
            else
              if (! in_source_local_phase(s, vedge_pids))
                continue;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;
            
            // update memory sets por_L and por_S
            process_id_t active_pid = compute_active_pid(vedge_pids);
            bool synchro = (vedge_pids.size() == 2);
            if(synchro_phase)
				update_mem_synchro(s, next_state, active_pid, synchro);
            else
				update_mem_local(s, next_state, active_pid, synchro);


			// Check whether next_state leads to a deadlock
			if (next_state->por_L().none()){
              if (cut_synchro(next_state))
                continue;
			}
            else{
              if (cut_local(next_state))
                continue;
			}

            v.push_back(next_state);
          }
        }
      private:
        /*!
         \brief Checks if a vedge is enabled w.r.t. selected process
         \param s : a state
         \param vedge_pids : process identifiers in a vedge
         \return true if a vedge involving processes vedge_pids is a communication 
         with the server or  local action of a process in s->por_S()
         */
        bool in_source_synchro_phase(state_ptr_t & s,
        std::set<tchecker::process_id_t> const & vedge_pids)
        {
          if (vedge_pids.size() == 2) // communication
            return true;
          // Check if local action of a process in s->por_S()
          process_id_t active_pid = *vedge_pids.begin();
          return s->por_S()[active_pid];
        }

         /*!
         \brief Checks if a vedge is enabled w.r.t. selected process
         \param s : a state
         \param vedge_pids : process identifiers in a vedge
         \return true if a vedge involving processes vedge_pids is a communication 
         involving a process in s->por_L() or a local action of a process in s->por_S()
         whose pid is greater than the max_pid of s->por_L()
         */
        bool in_source_local_phase(state_ptr_t & s,
        std::set<tchecker::process_id_t> const & vedge_pids)
        {
          
          if (vedge_pids.size() == 2) // communication
          {
            process_id_t active_pid = compute_active_pid(vedge_pids);
            return s->por_L()[active_pid]; // first pid is a client 
          }
          // Check if local action of a process in s->por_S greater than max in s->_por_L()
          process_id_t max_pid = max(s->por_L());
          process_id_t active_pid = compute_active_pid(vedge_pids);
          return (active_pid >= max_pid) && s->por_S()[active_pid];
        }

        /*!
         \brief Updates the memory of next_state according to synchro phase
         \param s : a state
         \param next_state : a successor of s
         \param active_pid : pid of process active on transition from s to next_state
         \param synchro : true if action from s to next_state is a synchronization
         */
        void update_mem_synchro(state_ptr_t & s, state_ptr_t & next_state, 
                                process_id_t active_pid, bool synchro)
        {
          if(synchro){
            next_state->por_S() = s->por_S();
            next_state->por_S()[active_pid] = true;
          }
          else{
            next_state->por_L().reset();
            next_state->por_L()[active_pid] = true;
            next_state->por_S() = s->por_S();
          }
        }

         /*!
         \brief Updates the memory of next_state according to local phase
         \param s : a state
         \param next_state : a successor of s
         \param active_pid : pid of process active on transition from s to next_state
         \param synchro : true if action from s to next_state is a synchronization
         */
        void update_mem_local(state_ptr_t & s, state_ptr_t & next_state, 
                                process_id_t active_pid, bool synchro)
        {
          if(synchro){
            next_state->por_L().reset();
            next_state->por_S().reset();
            next_state->por_S()[active_pid] = true;
          }
          else{
            next_state->por_L() = s->por_L();
            next_state->por_L()[active_pid] = true;
            next_state->por_S() = s->por_S();
          }
        }

		/*!
         \brief Check if a sync state leads to a deadlock
         \param s : a state
		 \return true if there exists a process_id_t pid in s such that 
         pid is not in s->por(S) and pid pure local
		 */
        bool cut_synchro(state_ptr_t & s)
        {
			for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
				auto const * location = *it;
				if(_pure_local_map.is_pure_local(location->id())  && location->pid() != _server_pid)
					if (!s->por_S()[location->pid()])
						return true;
			}
			return false;	
        }

		/*!
         \brief Check if a local state leads to a deadlock
         \param s : a state
		 \return true if there exists a process_id_t pid in s such that 
         pid is not in s->por(S) and pid pure local or such that pid < max(s->por_L()),
		 pid is in s->por(S) and pid pure local  
		 */
        bool cut_local(state_ptr_t & s)
        {
			for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
				auto const * location = *it;
				if (_pure_local_map.is_pure_local(location->id()) && location->pid() != _server_pid){
					if (!s->por_S()[location->pid()])
						return true;
					else if (location->pid() < max(s->por_L()))
						return true;
				}
			}
			return false;	
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

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::process_id_t _server_pid; /*!< PID of server process */
        tchecker::pure_local_map_t _pure_local_map; /*!< Pure local map */
        tchecker::process_id_t _processes_count; /*!< Number of client processes */
      };

    } // end of namespace por2

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR2_BUILDER_HH
