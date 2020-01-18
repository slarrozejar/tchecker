/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_MODEL_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_MODEL_HH

#include "tchecker/async_zg/details/model.hh"

/*!
 \file model.hh
 \brief Model for asynchronous zone graph with bounded spread (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class model_t
         \brief Model for asynchronous zone graph with bounded spread
         \tparam SYSTEM : type of system, see tchecker::ta::details::model_t
         \tparam VARIABLES : type of model variables, should inherit from tchecker::async_zg::details::variables_t
         \note see tchecker::async_zg::details::model_t for why instances cannot be constructed.
         */
        template <class SYSTEM, class VARIABLES>
        class model_t : public tchecker::async_zg::details::model_t<SYSTEM, VARIABLES> {
        public:
          /*!
           \brief Copy constructor
           */
          model_t(tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> const &) = default;

          /*!
           \brief Move constructor
           */
          model_t(tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> &&) = default;
          
          /*!
           \brief Destructor
           */
          virtual ~model_t() = default;
          
          /*!
           \brief Assignement operator
           */
          tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> &
          operator= (tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> const &) = default;
          
          /*!
           \brief Move-assignment operator
           */
          tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> &
          operator= (tchecker::async_zg::bounded_spread::details::model_t<SYSTEM, VARIABLES> &&) = default;
        protected:
          /*!
           \brief Constructor
           \param system : a system
           \param log : logging facility
           \note see tchecker::async_zg::details::model_t for details
           */
          explicit model_t(SYSTEM * system, tchecker::log_t & log)
          : tchecker::async_zg::details::model_t<SYSTEM, VARIABLES>(system, log)
          {}
        };
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_MODEL_HH



