/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_STATE_HH
#define TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_STATE_HH

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/async_zg/details/state.hh"

/*!
 \file state.hh
 \brief State of asynchronous zone graphs with sync zones (details)
 */

namespace tchecker {
  
  namespace async_zg {
    
    namespace bounded_spread {
      
      namespace details {
        
        /*!
         \class state_t
         \brief State of asynchronous zone graph with bounded spread (details)
         \tparam VLOC : type of tuple of locations
         \tparam INTVARS_VAL : type of integer variables valuations
         \tparam OFFSET_ZONE : type of offset zone
         \tparam VLOC_PTR : type of pointer to tuple of locations
         \tparam INTVARS_VAL_PTR : type of pointer to integer variables valuation
         \tparam OFFSET_ZONE_PTR : type of pointer to offset zone
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR>
        class state_t
        : public tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
        INTVARS_VAL_PTR, OFFSET_ZONE_PTR>
        {
        public:
          /*!
           \brief Type of offset zone
           */
          using offset_zone_t
          = typename tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
          INTVARS_VAL_PTR, OFFSET_ZONE_PTR>::offset_zone_t;
          
          /*!
           \brief Type of pointer to offset zone
           */
          using offset_zone_ptr_t
          = typename tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
          INTVARS_VAL_PTR, OFFSET_ZONE_PTR>::offset_zone_ptr_t;
          
          
          using tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
          INTVARS_VAL_PTR, OFFSET_ZONE_PTR>::state_t;
        };
        
        
        
        
        /*!
         \brief Equality check
         \param s1 : state
         \param s2 : state
         \return true if s1 and s2 have equal tuple of locations, equal integer variables
         valuation and equal zone, false otherwise
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR>
        inline bool
        operator== (tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
                    VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s1,
                    tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
                    VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s2)
        {
          return tchecker::async_zg::details::operator==(s1, s2);
        }
        
        
        /*!
         \brief Disequality check
         \param s1 : state
         \param s2 : state
         \return false if s1 and s2 have equal tuple of locations, equal integer variables
         valuation, and equal zone, true otherwise
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR>
        inline bool operator!=
        (tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
         VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s1,
         tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
         VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s2)
        {
          return (! (s1 == s2));
        }
        
        
        /*!
         \brief Hash
         \param s : state
         \return Hash value for state s
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR>
        inline std::size_t
        hash_value(tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
                   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s)
        {
          return tchecker::async_zg::details::hash_value(s);
        }
        
      } // end of namespace details
      
    } // end of namespace bounded_spread
    
  } // end of namespace async_zg
  
  
  /*!
   \brief Lexical ordering on asynchronous zone graph states with bounded spread
   \param s1 : first state
   \param s2 : second state
   \return 0 if s1 and s2 are equal, a negative value if s1 is smaller than s2 w.r.t. lexical ordering on tuple of locations, then intger valuation,
   then zone, a positive value otherwise
   */
  template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE,
  class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR>
  int lexical_cmp
  (tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s1,
   tchecker::async_zg::bounded_spread::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR> const & s2)
  {
    using async_zg_state_t
    = tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR, INTVARS_VAL_PTR,
    OFFSET_ZONE_PTR>;
    
    return tchecker::lexical_cmp(static_cast<async_zg_state_t const &>(s1),
                                 static_cast<async_zg_state_t const &>(s2));
  }
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_BOUNDED_SPREAD_DETAILS_STATE_HH

