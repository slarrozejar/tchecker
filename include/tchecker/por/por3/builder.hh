/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR3_BUILDER_HH
#define TCHECKER_POR_POR3_BUILDER_HH

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
#include "tchecker/por/por3/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing por3 POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace por3 {

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::por3::state_t
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
        _pure_local_map(tchecker::pure_local_map(model.system()))
        {
          if (! tchecker::client_server(model.system(), _server_pid))
				    throw std::invalid_argument("System is not client/server");
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::por3::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::por3::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por3::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por3::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::por3::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::por3::states_builder_t<TS, ALLOCATOR> &&)
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

            std::set<tchecker::process_id_t> vedge_pids
            = tchecker::vedge_pids(vedge);

            process_id_t active_pid = compute_active_pid(vedge_pids);
            state->por_memory(active_pid);

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

            if (! synchronizable(next_state))
              continue;

            // update por_memory
            process_id_t active_pid = compute_active_pid(vedge_pids);
            next_state->por_memory(active_pid);

            v.push_back(next_state);
          }
        }
      private:

        /*!
         \brief Checks if a state can reach a communication
         \param s : state
         \return true if a communication is reachable, false otherwise
         \note state with rank == tchecker::por::por3::communication can
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
         \brief Source set selection
         \param state : a state
         \param vedge_pids : process identifiers in a vedge
         \return true if a vedge involving processes vedge_pids is a local action 
         of process corresponding to the memory m or if it is a communication and 
         the location corresponding to process m is not pure local, false otherwise
         */
        bool in_source_set(state_ptr_t & state,
        std::set<tchecker::process_id_t> const & vedge_pids)
        {
          // Check if local action of por_memory process
          if (vedge_pids.size() == 1)
          {
            return *vedge_pids.begin() == state->por_memory();
          }
          // Check if location corresponding to por_memory() is pure local
          if(! _pure_local_map.is_pure_local(state->vloc()[state->por_memory()]->id()))
            return true;
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
      };

    } // end of namespace por3

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR3_BUILDER_HH
