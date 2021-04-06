/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_POR2_OUTPUT_HH
#define TCHECKER_POR_POR2_OUTPUT_HH

#include <iostream>

#include "tchecker/por/por2/state.hh"

/*!
 \file output.hh
 \brief Outputters for por2 POR states
 */

namespace tchecker {

  namespace por {

    namespace por2 {

      /*!
      \brief State outputter
      \tparam STATE : type of state
      \tparam STATE_OUTPUTTER : state outputter for states of type STATE
      \note this outputs states of type tchecker::por::por2::make_state_t<STATE>
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
        state_outputter_t(tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> const &) = default;

        /*!
        \brief Move contructor
        */
        state_outputter_t(tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> &&) = default;

        /*!
        \brief Destructor
        */
        ~state_outputter_t() = default;

        /*!
        \brief Assignment operator
        */
        tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> &
        operator= (tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> const &) = default;

        /*!
        \brief Move-assignment operator
        */
        tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> &
        operator= (tchecker::por::por2::state_outputter_t<STATE, STATE_OUTPUTTER> &&) = default;

        std::ostream & print_bitset(std::ostream & os, boost::dynamic_bitset<> const & bs)
        {
          os << "{";
          bool first = true; // check if first element to add
          for (unsigned i = 0; i < bs.size(); ++i){
            if(first && bs[i])
              os << i;
            else if (bs[i])
              os << "," << i ;
          }
          os << "}";
          return os;
        }

        /*!
        \brief Output state
        \param os : output stream
        \param s : state
        \post s has been output to os
        \return os after output
        */
        inline std::ostream & output (std::ostream & os, tchecker::por::por2::make_state_t<STATE> const & s)
        {
          STATE_OUTPUTTER::output(os, s);
          os << " /";
          print_bitset(os, s.por_L());
          os << " ";
          print_bitset(os, s.por_S());
          os << " /";
          return os;
        }
      };

    } // end of namespace por2

  } // end of namespace por

} // end of namespace tchecker

#endif // TCHECKER_POR_por2_OUTPUT_HH
