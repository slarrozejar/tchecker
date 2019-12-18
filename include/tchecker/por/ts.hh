/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_POR_TS_HH
#define TCHECKER_POR_TS_HH

#include <cassert>
#include <tuple>
#include <type_traits>

#include "tchecker/basictypes.hh"
#include "tchecker/flat_system/vedge.hh"
#include "tchecker/por/state.hh"
#include "tchecker/ts/ts.hh"

/*!
 \file ts.hh
 \brief Transition system with POR
 */

namespace tchecker {
  
  namespace por {
    
    namespace details {
      
      /*!
       \class gl_outgoing_iterator_t
       \brief Outgoing iterator for global-local transition system with POR
       \tparam OUTGOING_ITERATOR : type of outgoing iterator for underlying transition system
       */
      template <class OUTGOING_ITERATOR>
      class gl_outgoing_iterator_t {
      public:
        /*!
         \brief Constructor
         \param outgoing_it : outgoing iterator in underlying transition system
         \param rank : rank of active processes
         \param processes_count : number of processes
         \pre rank <= processes_count
         \throw std::invalid_argument : if rank > processes_count
         \note this iterator filters outgoing_it by only allowing global transitions and local transitions of
         processes with a PID greater than or equal to rank. If rank == processes_count, then only global
         transitions are allowed
         */
        gl_outgoing_iterator_t(OUTGOING_ITERATOR const & outgoing_it,
                               tchecker::process_id_t rank,
                               tchecker::process_id_t processes_count)
        : _outgoing_it(outgoing_it),
        _rank(rank),
        _processes_count(processes_count),
        _edge_rank(0)
        {
          if (_rank > _processes_count)
            throw std::invalid_argument("Invalid rank");
          skip();
        }
        
        /*!
         \brief Copy constructor
         */
        gl_outgoing_iterator_t(tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const &)
        = default;
        
        /*!
         \brief Move constructor
         */
        gl_outgoing_iterator_t(tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
        
        /*!
         \brief Destructor
         */
        ~gl_outgoing_iterator_t() = default;
        
        /*!
         \brief Assignment operator
         */
        tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &
        operator= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const &) = default;
        
        /*!
         \brief Move-assignment operator
         */
        tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &
        operator= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> &&) = default;
        
        /*!
         \brief Equality predicate
         \param it : iterator
         \return true iff this and it are equal
         */
        inline bool
        operator== (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
        {
          return ((_outgoing_it == it._outgoing_it) &&
                  (_rank == it._rank) &&
                  (_processes_count == it._process_count) &&
                  (_edge_rank == it._edge_rank));
        }
        
        /*!
         \brief Disequality predicate
         \param it : iterator
         \return true iff this and it are different
         */
        inline bool
        operator!= (tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> const & it) const
        {
          return ! (*this == it);
        }
        
        /*!
         \brief Accessor
         \return true if this is past-the-end, false otherwise
         */
        inline bool at_end() const
        {
          return _outgoing_it.at_end();
        }
        
        /*!
         \brief Increment operator
         \pre this is not past-the-end
         \post this points to the next outgoing transition if any, or past-the-end otherwise
         \return this after increment
         */
        inline tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR> & operator++ ()
        {
          assert(! at_end());
          ++ _outgoing_it;
          skip();
          return *this;
        }
        
        /*!
         \brief Value type (edge rank, edge)
         */
        using value_type_t
        = std::tuple<tchecker::process_id_t, typename std::iterator_traits<OUTGOING_ITERATOR>::value_type>;
        
        /*!
         \brief Dereference operator
         \pre this is not past-the-end
         \return data pointed by this
         */
        inline value_type_t operator* ()
        {
          assert(! at_end());
          return std::make_tuple(_edge_rank, *_outgoing_it);
        }
      private:
        /*!
         \brief Skip transitions that involve processes with a PID less than _rank
         \post _outgoing_it points to the next transition that is either global, or that is local and that involves a process with
         a PID greater than or equal to _rank, if any, and past-the-end otherwise.. If _rank == _processes_count, only global
         transitions are allowed.
         \throw std::runtime_error : if the model is not local/gobal
         */
        void skip()
        {
          while (! _outgoing_it.at_end()) {
            // Compute vedge size and edge rank
            _edge_rank = _processes_count;  // not a PID
            std::size_t vedge_size = 0;
            for (auto it = (*_outgoing_it).begin(); it != (*_outgoing_it).end(); ++it) {
              if (_edge_rank == _processes_count)
                _edge_rank = (*it)->pid();
              ++vedge_size;
            }

            // Check global/local system
            if ((vedge_size != 1) && (vedge_size != _processes_count))
              throw std::runtime_error("System is not local/global");
            
            // Check if expected vedge found
            if ((vedge_size == 1) && (_edge_rank >= _rank))
              break;
            else if (vedge_size == _processes_count) {
              _edge_rank = _processes_count;
              break;
            }
              
            ++ _outgoing_it;
          }
        }
                
        OUTGOING_ITERATOR _outgoing_it;           /*!< Underlying iterator */
        tchecker::process_id_t _rank;             /*!< Rank of active processes  */
        tchecker::process_id_t _processes_count;  /*!< Number of processes */
        tchecker::process_id_t _edge_rank;        /*!< Rank of current edge */
      };
      
    } // end of namespace details
    
  } // end of namespace por
  
} // end of namespace tchecker




/*!
 \brief Iterator traits for tchecker::por::details::gl_outgoing_iterator_t
 */
template <class OUTGOING_ITERATOR>
struct std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR>> {
  using difference_type = nullptr_t;
  
  using value_type
  = typename tchecker::por::details::gl_outgoing_iterator_t<OUTGOING_ITERATOR>::value_type_t;
  
  using pointer = value_type *;
  
  using reference = value_type &;
  
  using iterator_category = std::forward_iterator_tag;
};
    

    
  
namespace tchecker {
    
  namespace por {
  
    /*!
     \class gl_ts_t
     \brief Transition system with global-local POR
     \tparam TS : type of underlying transition system, should implement tchecker::ts::ts_t
     \tparam STATE : type of states, should inherit from TS::state_t, and from tchecker::por::state_t
     \note gl_ts_t<TS> implements a POR on top of TS. TS is supposed to be global/local: every transition involves either
     one process (local) or all the processes (global)
     */
    template <class TS, class STATE>
    class gl_ts_t
    : public tchecker::ts::ts_t
    <
    STATE,
    typename TS::transition_t,
    typename TS::initial_iterator_t,
    tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
    typename TS::initial_iterator_value_t,
    typename std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
    >
    {
      
      static_assert(std::is_base_of<tchecker::por::state_t, STATE>::value,
                    "STATE should derive from tchecker::por::state_t");
      
      static_assert(std::is_base_of<typename TS::state_t, STATE>::value,
                    "STATE should derive from TS::state_t");
      
      using por_ts_t
      = tchecker::ts::ts_t
      <
      STATE,
      typename TS::transition_t,
      typename TS::initial_iterator_t,
      tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>,
      typename TS::initial_iterator_value_t,
      typename std::iterator_traits<tchecker::por::details::gl_outgoing_iterator_t<typename TS::outgoing_edges_iterator_t>>::value_type
      >;
      
    public:
      /*!
       \brief Constructor
       \param args : arguments to a constructor of TS
       */
      template <class ... ARGS>
      gl_ts_t(ARGS && ... args)
      : _ts(args...), _processes_count(_ts.model().system().processes_count())
      {}
      
      /*!
       \brief Copy constructor
       */
      gl_ts_t(tchecker::por::gl_ts_t<TS, STATE> const &) = default;
      
      /*!
       \brief Move constructor
       */
      gl_ts_t(tchecker::por::gl_ts_t<TS, STATE> &&) = default;
      
      /*!
       \brief Destructor
       */
      virtual ~gl_ts_t() = default;
      
      /*!
       \brief Assignment operator
       */
      tchecker::por::gl_ts_t<TS, STATE> & operator= (tchecker::por::gl_ts_t<TS, STATE> const &) = default;
      
      /*!
       \brief Move-assignment oeprator
       */
      tchecker::por::gl_ts_t<TS, STATE> & operator= (tchecker::por::gl_ts_t<TS, STATE> &&) = default;
      
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
       \brief Type of outgoing edges iterator
       */
      using outgoing_edges_iterator_t = typename por_ts_t::outgoing_edges_iterator_t;
      
      /*!
       \brief Type of outgoing edges itreator value
       */
      using outgoing_edges_iterator_value_t = typename por_ts_t::outgoing_edges_iterator_value_t;
      
      /*!
       \brief Accessor
       \param s : state
       \return outgoing edges from state s
       */
      virtual tchecker::range_t<outgoing_edges_iterator_t> outgoing_edges(STATE const & s)
      {
        tchecker::range_t<typename TS::outgoing_edges_iterator_t> ts_outgoing_edges = _ts.outgoing_edges(s);
        return tchecker::make_range
        (outgoing_edges_iterator_t(ts_outgoing_edges.begin(), s.rank(), _processes_count),
         outgoing_edges_iterator_t(ts_outgoing_edges.end(), s.rank(), _processes_count));
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
                                                 outgoing_edges_iterator_value_t const & v)
      {
        tchecker::process_id_t edge_rank = std::get<0>(v);
        typename TS::outgoing_edges_iterator_value_t const & vedge = std::get<1>(v);
        
        assert(s.rank() <= edge_rank);
        
        enum tchecker::state_status_t state_status = _ts.next(s, t, vedge);
        s.rank(edge_rank == _processes_count ? 0 : edge_rank);
        
        return state_status;
      }
    private:
      TS _ts;                                   /*!< Underlying transition system */
      tchecker::process_id_t _processes_count;  /*!< Number of processes */
    };
    
  } // end of namespace por
  
} // end of namespace tchecker

#endif // TCHECKER_POR_TS_HH
