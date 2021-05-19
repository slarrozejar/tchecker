/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR4_BUILDER_HH
#define TCHECKER_POR_POR4_BUILDER_HH

#include <cassert>
#include <limits>
#include <set>
#include <string>
#include <type_traits>
#include <vector>
#include <boost/dynamic_bitset>
#include <vector>

#include "tchecker/algorithms/covreach/builder.hh"
#include "tchecker/basictypes.hh"
#include "tchecker/dbm/offset_dbm.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/por4/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing por4 POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace por4 {

      tchecker::process_id_t NO_SELECTED_PROCESS = std::numeric_limits<tchecker::process_id_t>::max();

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::por4::state_t
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
        states_builder_t(MODEL & model, TS & ts, ALLOCATOR & allocator)
        : _ts(ts),
        _allocator(allocator),
        _pure_local_map(tchecker::pure_local_map(model.system()))
        {
          if (! tchecker::client_server(model.system(), _server_pid))
				    throw std::invalid_argument("System is not client/server")
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::por4::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::por4::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por4::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por4::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::por4::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por4::states_builder_t<TS, ALLOCATOR> &&)
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
          // 2. Calculer l'ensemble E' = {(next_state, pid),...} des transitions
          //    enabled (next state, PID du processus) and update bitvector of 
          // pure local processes and int vector of outgoing edges for each process
          std::vector<std::tuple<state_ptr_t, std::set<tchecker::process_id_t>>> enabled;
          std::set<tchecker::process_id_t> enabled_processes;

          // Initialize all clients to be pure local
          process_id_t client_processes = s->vloc().size()-1;
          boost::dynamic_bitset<> pure_local(client_processes).set();

          // Vector counting the number of outgoing transition for each process
          std::vector<int> outgoing_degree(client_processes,0);         

          auto outgoing_vedges = _ts.outgoing_edges(*s);
          for (auto it = outgoing_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            std::set<tchecker::process_id_t> vedge_pids
            = tchecker::vedge_pids(vedge);

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            if (! synchronizable(next_state))
              continue;

            std::set<tchecker::process_id_t> vedge_pids = tchecker::vedge_pids(vedge);
            tchecker::process_id_t active_pid = compute_active_pid(vedge_pids);

            // Check wether synchronization and update pure local processes accordingly
            if (vedge_pids.size() == 2)
              pure_local[active_pid] = 0;

            // Update outgoing_degree
            outgoing_degree[active_pid] += 1;

            // Store next state and process that led to it
            enabled_processes.insert(active_pid);
            enabled.push_back(std::make_tuple(next_state, vedge_pids));


            // Déterminer le processus pur local qui a le moins de transitions sortantes 
            // ou NO_SELECTED_PROCESS si il n'y a pas de processus pur local 
            min_pid = tchecker::por::por4::NO_SELECTED_PROCESS;
            if (!pure_local.none()) {
              for(process_id_t it = pure_local.find_first(); it < client_processes;
                  it = pure_local.find_next(it)) {
                if (outgoing_degree[it] < outgoing_degree[min_pid])
                  min_pid = it;
              }
            }

            // 4. Mettre dans v
            //    - les next_states du process i s'il y en a un
            //    - tous les next_states sinon
            for (auto && [next_state, vedge_pids] : enabled) {
              if (in_source(vedge_pids, min_pid))
                v.push_back(next_state);
            }
          }
        }
      private:

        /*!
         \brief Checks if a state can reach a communication
         \param s : state
         \return true if a communication is reachable, false otherwise
         \note state with rank == tchecker::por::por4::communication can
         trivially reach a communication. Other states, where only
         process s->por_active_pid() is allowed to do local actions, can reach a
         communication action if there is a communication that is feasible by
         the server process and reachable (through local actions)
         for process s.por_active_pid()
         */
        bool synchronizable(state_ptr_t & s) const
        {
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
        \brief Checks if a vedge is enabled w.r.t. selected process
        \param vedge_pids : PIDs of processes involved in vedge
        \param selected_process : process that is active
        \return true if vedge_pids involves the selected process or if no
        process is selected, false otherwise
        */
        bool in_source(std::set<tchecker::process_id_t> const & vedge_pids,
                       tchecker::process_id_t selected_process)
        {
          if (selected_process == tchecker::por::por4::NO_SELECTED_PROCESS)
            return true;
          return vedge_pids.find(selected_process) != vedge_pids.end();
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::process_id_t _server_pid; /*!< PID of server process */
        _server_pid(model.system().processes().key(server));
      };

    } // end of namespace por4

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR4_BUILDER_HH
