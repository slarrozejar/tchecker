/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_STATE_HH
#define TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_STATE_HH

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
    
    namespace sync_zones {
      
      namespace details {
        
        /*!
         \class state_t
         \brief State of asynchronous zone graph (details)
         \tparam VLOC : type of tuple of locations
         \tparam INTVARS_VAL : type of integer variables valuations
         \tparam OFFSET_ZONE : type of offset zone
         \tparam SYNC_ZONE : type of synchronized zone
         \tparam VLOC_PTR : type of pointer to tuple of locations
         \tparam INTVARS_VAL_PTR : type of pointer to integer variables valuation
         \tparam OFFSET_ZONE_PTR : type of pointer to offset zone
         \tparam SYNC_ZONE_PTR : type of pointer to synchronized zone
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE, class SYNC_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR, class SYNC_ZONE_PTR>
        class state_t
        : public tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
        INTVARS_VAL_PTR, OFFSET_ZONE_PTR>
        {
          using this_state_t
          = tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
          VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR>;
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
          
          /*!
           \brief Type of synchronized zone
           */
          using sync_zone_t = SYNC_ZONE;
          
          /*!
           \brief Type of pointer to synchronized zone
           */
          using sync_zone_ptr_t = SYNC_ZONE_PTR;
          
          /*!
           \brief Constructor
           \param sync_zone : a synchronized zone
           \param offset_zone : an offset zone
           \param vloc : tuple of locations
           \param intvars_val : integer variables valuation
           \pre vloc, intvars_val, offset_zone and sync_zone must be valid pointers (not checked)
           \note this keeps a pointer to vloc, intvars_val, offset_zone and sync_zone
           */
          state_t(SYNC_ZONE_PTR const & sync_zone,
                  OFFSET_ZONE_PTR const & offset_zone,
                  VLOC_PTR const & vloc,
                  INTVARS_VAL_PTR const & intvars_val)
          : tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR,
          INTVARS_VAL_PTR, OFFSET_ZONE_PTR>(offset_zone, vloc, intvars_val),
          _sync_zone(sync_zone)
          {}
          
          /*!
           \brief Constructor
           \param s : a state
           \param offset_zone : an offset zone
           \param sync_zone : a synchronized zone
           \param vloc : tuple of locations
           \param intvars_val : integer variables valuation
           \pre vloc, intvars_val, offset_zone and sync_zone must be valid pointers (not checked)
           \note this keeps a pointer to vloc, intvars_val, offset_zone and sync_zone
           */
          state_t(this_state_t const & s,
                  SYNC_ZONE_PTR const & sync_zone,
                  OFFSET_ZONE_PTR const & offset_zone,                  
                  VLOC_PTR const & vloc,
                  INTVARS_VAL_PTR const & intvars_val)
          : state_t(sync_zone, offset_zone, vloc, intvars_val)
          {}
          
          /*!
           \brief Copy constructor (deleted)
           */
          state_t(this_state_t const &) = delete;
          
          /*!
           \brief Move constructor (deleted)
           */
          state_t(this_state_t &&) = delete;
          
          /*!
           \brief Destructor
           \note vloc pointer, intvars valuation pointer and zone pointers are not deleted
           */
          ~state_t() = default;
          
          /*!
           \brief Assignment operator (deleted)
           */
          this_state_t & operator= (this_state_t const &) = delete;
          
          /*!
           \brief Move-assignment operator (deleted)
           */
          this_state_t & operator= (this_state_t &&) = delete;
          
          /*!
           \brief Accessor
           \return synchronized zone
           */
          inline SYNC_ZONE const & sync_zone() const
          {
            return *_sync_zone;
          }
          
          /*!
           \brief Accessor
           \return pointer to synchronized zone
           */
          inline SYNC_ZONE_PTR & sync_zone_ptr()
          {
            return _sync_zone;
          }
        protected:
          SYNC_ZONE_PTR _sync_zone;      /*!< Synchronized zone */
        };
        
        
        
        
        /*!
         \brief Equality check
         \param s1 : state
         \param s2 : state
         \return true if s1 and s2 have equal tuple of locations, equal integer variables
         valuation and equal zone, false otherwise
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE, class SYNC_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR, class SYNC_ZONE_PTR>
        inline bool
        operator== (tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
                    SYNC_ZONE, VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s1,
                    tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE,
                    SYNC_ZONE, VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s2)
        {
          return tchecker::async_zg::details::operator==(s1, s2) && (s1.sync_zone() == s2.sync_zone());
        }
        
        
        /*!
         \brief Disequality check
         \param s1 : state
         \param s2 : state
         \return false if s1 and s2 have equal tuple of locations, equal integer variables
         valuation, and equal zone, true otherwise
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE, class SYNC_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR, class SYNC_ZONE_PTR>
        inline bool operator!=
        (tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
         VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s1,
         tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
         VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s2)
        {
          return (! (s1 == s2));
        }
        
        
        /*!
         \brief Hash
         \param s : state
         \return Hash value for state s
         */
        template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE, class SYNC_ZONE,
        class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR, class SYNC_ZONE_PTR>
        inline std::size_t
        hash_value(tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
                   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s)
        {
          std::size_t h = tchecker::async_zg::details::hash_value(s);
          boost::hash_combine(h, s.sync_zone().hash());
          return h;
        }
        
      } // end of namespace details
      
    } // end of namespace sync_zones
    
  } // end of namespace async_zg
  
  
  /*!
   \brief Lexical ordering on asynchronous zone graph states
   \param s1 : first state
   \param s2 : second state
   \return 0 if s1 and s2 are equal, a negative value if s1 is smaller than s2 w.r.t. lexical ordering on tuple of locations, then intger valuation,
   then zone, a positive value otherwise
   */
  template <class VLOC, class INTVARS_VAL, class OFFSET_ZONE, class SYNC_ZONE,
  class VLOC_PTR, class INTVARS_VAL_PTR, class OFFSET_ZONE_PTR, class SYNC_ZONE_PTR>
  int lexical_cmp
  (tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s1,
   tchecker::async_zg::sync_zones::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, SYNC_ZONE,
   VLOC_PTR, INTVARS_VAL_PTR, OFFSET_ZONE_PTR, SYNC_ZONE_PTR> const & s2)
  {
    using async_zg_state_t
    = tchecker::async_zg::details::state_t<VLOC, INTVARS_VAL, OFFSET_ZONE, VLOC_PTR, INTVARS_VAL_PTR,
    OFFSET_ZONE_PTR>;
    
    return tchecker::lexical_cmp(static_cast<async_zg_state_t const &>(s1),
                                 static_cast<async_zg_state_t const &>(s2));
  }
  
} // end of namespace tchecker

#endif // TCHECKER_ASYNC_ZG_SYNC_ZONES_DETAILS_STATE_HH

