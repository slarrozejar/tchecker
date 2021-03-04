/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR1_TS_HH
#define TCHECKER_POR_POR1_TS_HH

#include "tchecker/basictypes.hh"
#include "tchecker/por/por1/state.hh"
#include "tchecker/ts/ts.hh"
#include "tchecker/utils/iterator.hh"

/*!
 \file ts.hh
 \brief Transition system for por1 partial-order reduction
 */

namespace tchecker {

  namespace por {

    namespace por1 {

      /*!
      \class ts_t
      \tparam TS : type of transition system
      \brief Transition system for por1 partial-order reduction,
      extends TS with states which embed the information required for
      por1 partial-order reduction
      */
      template <class TS>
      class ts_t final
      : public tchecker::ts::ts_t<
      tchecker::por::por1::make_state_t<typename TS::state_t>,
      typename TS::transition_t,
      typename TS::initial_iterator_t,
      typename TS::outgoing_edges_iterator_t,
      typename TS::initial_iterator_value_t,
      typename TS::outgoing_edges_iterator_value_t>
      {
      public:
        using state_t = tchecker::por::por1::make_state_t<typename TS::state_t>;
        using transition_t = typename TS::transition_t;
        using initial_iterator_t = typename TS::initial_iterator_t;
        using outgoing_edges_iterator_t
        = typename TS::outgoing_edges_iterator_t;
        using initial_iterator_value_t = typename TS::initial_iterator_value_t;
        using outgoing_edges_iterator_value_t
        = typename TS::outgoing_edges_iterator_value_t;

        /*!
        \brief Constructor
        \param args : arguments to a constructor of TS
        */
        template <class ... ARGS>
        ts_t(ARGS && ... args)
        : _ts(args...)
        {}

        /*!
        \brief Copy constructor
        */
        ts_t(tchecker::por::por1::ts_t<TS> const &) = default;

        /*!
        \brief Move constructor
        */
        ts_t(tchecker::por::por1::ts_t<TS> &&) = default;

        /*!
        \brief Destructor
        */
        virtual ~ts_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por1::ts_t<TS> &
        operator= (tchecker::por::por1::ts_t<TS> const &) = default;

        /*!
        \brief Move-assignment operator
        */
        tchecker::por::por1::ts_t<TS> &
        operator= (tchecker::por::por1::ts_t<TS> &&) = default;

        /*!
        \brief see tchecker::async_zg::ts_t::initial
        */
        virtual tchecker::range_t<initial_iterator_t> initial()
        {
          return _ts.initial();
        }

        /*!
        \brief see tchecker::async_zg::ts_t::initialize
        */
        virtual enum tchecker::state_status_t initialize
        (state_t & s, transition_t & t, initial_iterator_value_t const & v)
        {
          return _ts.initialize(s, t, v);
        }

        /*!
        \brief see tchecker::async_zg::ts_t::outgoing_edges
        */
        virtual tchecker::range_t<outgoing_edges_iterator_t>
        outgoing_edges(state_t const & s)
        {
          return _ts.outgoing_edges(s);
        }

        /*!
        \brief see tchecker::async_zg::ts_t::next
        */
        virtual enum tchecker::state_status_t next
        (state_t & s, transition_t & t,
        outgoing_edges_iterator_value_t const & v)
        {
          return _ts.next(s, t, v);
        }

        /*!
        \brief see tchecker::async_zg::ts_t::synchronizable_zone
        */
        virtual bool synchronizable_zone(state_t & s) const
        {
          return _ts.synchronizable_zone(s);
        }
      private:
        TS _ts;  /*!< Underlying transition system */
      };

    } // end of namespace por1

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_POR1_TS_HH
