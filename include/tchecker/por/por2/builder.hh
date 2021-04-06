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
        _allocator(allocator)
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

            v.push_back(next_state);
          }
        }
      private:

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
          return true;
        }

        TS & _ts; /*!< Transition system */
        ALLOCATOR & _allocator; /*!< Allocator */
      };

    } // end of namespace por2

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR2_BUILDER_HH
