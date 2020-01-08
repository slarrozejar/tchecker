/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_TS_HH
#define TCHECKER_POR_TS_HH

#include <type_traits>

#include "tchecker/basictypes.hh"
#include "tchecker/por/state.hh"
#include "tchecker/ts/ts.hh"

/*!
 \file ts.hh
 \brief Transition system with partial-order reduction
 */

namespace tchecker {
  
  namespace por {
    
    namespace details {
      
      /*!
       \brief Short name for TS with states of type STATE
       */
      template <class TS, class STATE>
      using ts_instance_t
      = tchecker::ts::ts_t<STATE,
      typename TS::transition_t,
      typename TS::initial_iterator_t,
      typename TS::outgoing_edges_iterator_t,
      typename TS::initial_iterator_value_t,
      typename TS::outgoing_edges_iterator_value_t
      >;
      
    } // end of namespace details
    
    
    
    
    /*!
     \class ts_t
     \brief Transition system with partial-order reduction
     \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
     \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
     \tparam SOURCE_SET : source set, should provide membership operator
     bool operator() (STATE const &, tchecker::vedge_iterator_t const &)
     \note ts_t<TS> implements partial-order reduction on top of TS
     */
    template <class TS, class STATE, class SOURCE_SET>
    class ts_t : public tchecker::por::details::ts_instance_t<TS, STATE> {
    
      static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                    "STATE should derive from tchecker::por::state_t");
      
      static_assert(std::is_base_of<typename TS::state_t, STATE>::value,
                    "STATE should derive from TS::state_t");

    public:
      /*!
       \brief Constructor
       \param model : a model
       \note TS should have a constructor TS(MODEL &)
       */
      template <class MODEL>
      ts_t(MODEL & model)
      : _ts(model)
      {}
        
      /*!
       \brief Copy constructor
       */
      ts_t(tchecker::por::ts_t<TS, STATE, SOURCE_SET> const &) = default;
        
      /*!
       \brief Move constructor
       */
      ts_t(tchecker::por::ts_t<TS, STATE, SOURCE_SET> &&) = default;
        
      /*!
       \brief Destructor
       */
      virtual ~ts_t() = default;
        
      /*!
       \brief Assignment operator
       */
      tchecker::por::ts_t<TS, STATE, SOURCE_SET> &
      operator= (tchecker::por::ts_t<TS, STATE, SOURCE_SET> const &) = default;
        
      /*!
       \brief Move-assignment oeprator
       */
      tchecker::por::ts_t<TS, STATE, SOURCE_SET> &
      operator= (tchecker::por::ts_t<TS, STATE, SOURCE_SET> &&) = default;
       
      /*!
       \brief Accessor
       \return Initial state valuations
       */
      inline virtual tchecker::range_t<typename TS::initial_iterator_t> initial()
      {
        return _ts.initial();
      }
      
      /*!
       \brief Initialize state
       \param s : state
       \param t : transition
       \param v : initial state valuation
       \post state s and transtion t have been initialized from v
       \return status of state s after initialization
       \note t represents an initial transition to s
       */
      inline virtual enum tchecker::state_status_t initialize(STATE & s,
                                                              typename TS::transition_t & t,
                                                              typename TS::initial_iterator_value_t const & v)
      {
        return _ts.initialize(s, t, v);
      }
      
      /*!
       \brief Accessor
       \param s : state
       \return outgoing edges from state s
       */
      inline virtual tchecker::range_t<typename TS::outgoing_edges_iterator_t> outgoing_edges(STATE const & s)
      {
        return _ts.outgoing_edges(s);
      }
      
      /*!
       \brief Next state computation
       \param s : state
       \param t : transition
       \param v : outgoing edge value
       \post s and t have been updated from v
       \return status of state s after update
       */
      virtual enum tchecker::state_status_t next(STATE & s,
                                                 typename TS::transition_t & t,
                                                 typename TS::outgoing_edges_iterator_value_t const & v)
      {
        enum tchecker::state_status_t status = _ts.next(s, t, v);
        if (status != tchecker::STATE_OK)
          return status;
        
        if (! _source_set(s, v))
          return tchecker::STATE_POR_DISABLED;
        
        return tchecker::STATE_OK;
      }
    private:
      TS _ts;                   /*!< Underlying transition system */
      SOURCE_SET _source_set;   /*!< Source set */
    };
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_TS_HH

