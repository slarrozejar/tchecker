/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_RR_OUTPUT_HH
#define TCHECKER_POR_RR_OUTPUT_HH

#include <iostream>

#include "tchecker/por/rr/state.hh"

/*!
 \file output.hh
 \brief Outputters for rr POR states
 */

namespace tchecker {

  namespace por {

    namespace rr {

      /*!
      \brief State outputter
      \tparam STATE : type of state
      \tparam STATE_OUTPUTTER : state outputter for states of type STATE
      \note this outputs states of type tchecker::por::rr::make_state_t<STATE>
      */
      template <class STATE, class STATE_OUTPUTTER>
      class state_outputter_t : public STATE_OUTPUTTER {
      public:
        /*!
        \brief Constructor
        \param args : arguments to a constructor of STATE_OUPUTTER
        */
        template <class ... ARGS>
        state_outputter_t(ARGS && ... args) : STATE_OUTPUTTER(args...)
        {}

        /*!
        \brief Copy contructor
        */
        state_outputter_t(tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> const &) = default;

        /*!
        \brief Move contructor
        */
        state_outputter_t(tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> &&) = default;

        /*!
        \brief Destructor
        */
        ~state_outputter_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> &
        operator= (tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> const &) = default;

        /*!
        \brief Move-assignment operator
        */
        tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> &
        operator= (tchecker::por::rr::state_outputter_t<STATE, STATE_OUTPUTTER> &&) = default;

        /*!
        \brief Output state
        \param os : output stream
        \param s : state
        \post s has been output to os
        \return os after output
        */
        inline std::ostream & output (std::ostream & os, tchecker::por::rr::make_state_t<STATE> const & s)
        {
          std::ostream & os2 = STATE_OUTPUTTER::output(os, s);
          return os2;
        }
      };

    } // end of namespace rr

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_RR_OUTPUT_HH
