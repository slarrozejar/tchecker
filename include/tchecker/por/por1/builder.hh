/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR1_BUILDER_HH
#define TCHECKER_POR_POR1_BUILDER_HH

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
        states_builder_t(MODEL & model, TS & ts, ALLOCATOR & allocator)
        : _ts(ts),
        _allocator(allocator),
        _pure_local_map(tchecker::pure_local_map(model.system()))
        {}

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

            /*
            if (! synchronizable(state))
              continue;
            */

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
            next_no_process_selected(s, v);
          else
            next_current_process(s, v);
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

          // 2. Calculer l'ensemble E' = {(next_state, pid),...} des transitions
          //    enabled (next state, PID du processus)
          //    Prendre PID du processus = int max si pas edge local
          std::vector<std::tuple<state_ptr_t, tchecker::process_id_t>> enabled;
          auto outgoing_vedges = _ts.outgoing_edges(*s);
          for (auto it = outgoing_vedges.begin(); ! it.at_end(); ++it) {
            auto const vedge = *it;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            /*
            if (! synchronizable(next_state))
              continue;
            */

            // TODO: mettre next_state->por_memory() à la bonne valeur

            v.push_back(next_state); // <- dans enabled
          }

          // 3. Déterminer le plus petit i td i est pure local et a un edge
          //    enabled
          
          // 4. Mettre dans v
          //    - les next_states du process i s'il y en a un
          //    - tous les next_states sinon
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

            if (vedge_pids.find(s->por_memory()) == vedge_pids.end())
              // current process not involved in vedge
              continue;

            state_ptr_t next_state = _allocator.construct_from_state(s);
            transition_ptr_t transition = _allocator.construct_transition();

            tchecker::state_status_t status
            = _ts.next(*next_state, *transition, vedge);

            if (status != tchecker::STATE_OK)
              continue;

            /*
            if (! synchronizable(next_state))
              continue;
            */

            v.push_back(next_state);
          }
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
        tchecker::pure_local_map_t _pure_local_map; /*!< Pure local map */
      };

    } // end of namespace por1

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR1_BUILDER_HH
