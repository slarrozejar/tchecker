/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_MAG_BUILDER_HH
#define TCHECKER_POR_MAG_BUILDER_HH

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
#include "tchecker/por/mag/state.hh"
#include "tchecker/por/synchronizable.hh"
#include "tchecker/system/static_analysis.hh"
#include "tchecker/ts/builder.hh"

/*!
 \file builder.hh
 \brief Builder implementing mag POR for covreach algorithm
 */

namespace tchecker {

  namespace por {

    namespace mag {

      /*!
      \class states_builder_t
      \brief States builder for covering reachability algorithm with
      client-server partial order reduction
      \tparam TS : type of transition system (see tchecker::ts::builder_ok_t)
      \tparam ALLOCATOR : type of allocator (see tchecker::ts::builder_ok_t)
      \note states should derive from tchecker::por::mag::state_t
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
        _location_next_syncs(tchecker::location_next_syncs(model.system()))
        {
          if (! tchecker::client_server(model.system(), _server_pid))
				    throw std::invalid_argument("System is not client/server");
        }

        /*!
        \brief Copy constructor
        */
        states_builder_t
        (tchecker::por::mag::states_builder_t<TS, ALLOCATOR> const &) = default;

        /*!
        \brief Move constructor
        */
        states_builder_t
        (tchecker::por::mag::states_builder_t<TS, ALLOCATOR> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~states_builder_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::mag::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::mag::states_builder_t<TS, ALLOCATOR> const &)
        = default;

        /*!
        \brief Move assignment operator
        */
        tchecker::por::mag::states_builder_t<TS, ALLOCATOR> &
        operator= (tchecker::por::mag::states_builder_t<TS, ALLOCATOR> &&)
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

            // if (cut(state))
            //   continue;

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

            // if (cut(next_state))
            //   continue;

            v.push_back(next_state);
          }
        }
      private:

        /*!
        \brief Checks if a state leads to a deadlock
        \param s : state
        \return true if state leads to a deadlock, false otherwise
        */
        bool cut(state_ptr_t & s) const
        {
          // next server synchronizations reachable
          boost::dynamic_bitset<> const & server_next_sync = _location_next_syncs.next_syncs(s->vloc()[_server_pid]->id(),
                                                  location_next_syncs_t::next_type_t::NEXT_SYNC_REACHABLE);
          for(auto it = s->vloc().begin(); it != s->vloc().end(); ++it) {
            auto const * location = *it;
            if (location->pid() != _server_pid) { 
              if (!magnetic(location->name())){
                boost::dynamic_bitset<> next_sync_not_mag = 
                  _location_next_syncs.next_syncs(location->id(),
                  location_next_syncs_t::next_type_t::NEXT_SYNC_NOT_MAG_REACHABLE);
                if (next_sync_not_mag.none())
                  continue;
                next_sync_not_mag &= server_next_sync;
                if (next_sync_not_mag.none())
                    return true;
              }
            }
          }
          return false;
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
         \return true if a vedge involving processes vedge_pids is in the source
         set of state, false otherwise
         */
        bool in_source_set(state_ptr_t & state,
        std::set<tchecker::process_id_t> const & vedge_pids)
        {
          // Check if state contains a non magnetic state 
          for(auto it = state->vloc().begin(); it != state->vloc().end(); ++it) {
            auto const * location = *it;
            if (! magnetic(location->name())) {
              tchecker::process_id_t active_pid = compute_active_pid(vedge_pids);
              return active_pid == location->pid();
            }
          }
          return true;
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::process_id_t _server_pid; /*!< PID of server process */
        tchecker::location_next_syncs_t _location_next_syncs; /*!< Next synchronisations */
      };

    } // end of namespace mag

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_MAG_BUILDER_HH
