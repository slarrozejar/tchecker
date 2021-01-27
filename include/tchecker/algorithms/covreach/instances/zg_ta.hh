/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ALGORITHMS_COVREACH_ZG_TA_HH
#define TCHECKER_ALGORITHMS_COVREACH_ZG_TA_HH

#include "tchecker/algorithms/covreach/builder.hh"
#include "tchecker/algorithms/covreach/options.hh"
#include "tchecker/algorithms/covreach/graph.hh"
#include "tchecker/variables/clocks.hh"
#include "tchecker/variables/intvars.hh"
#include "tchecker/zg/zg_ta.hh"

/*!
 \file zg_ta.hh
 \brief Instantiation of covreach algorithm for zone graph
 */

namespace tchecker {
  
  namespace covreach {
    
    namespace details {
      
      namespace zg {
        
        namespace ta {
          
          /*!
           \class algorithm_model_t
           \brief Model for covering reachability over zone graphs of timed automata
           */
          template <class ZONE_SEMANTICS>
          class algorithm_model_t {
          public:
            using zone_semantics_t = ZONE_SEMANTICS;
            using model_t = tchecker::zg::ta::model_t;
            
            using ts_t = typename zone_semantics_t::ts_t;
            using state_t = typename ts_t::state_t;
            using transition_t = typename ts_t::transition_t;
            
            using key_t = std::size_t;
            
            using node_t = typename tchecker::covreach::details::graph_types_t<ts_t>::node_t;
            using node_ptr_t = typename tchecker::covreach::details::graph_types_t<ts_t>::node_ptr_t;
            
            using node_allocator_t = typename zone_semantics_t::template state_pool_allocator_t<node_t>;
            using transition_allocator_t = typename zone_semantics_t::template transition_singleton_allocator_t<transition_t>;
            using ts_allocator_t = tchecker::ts::allocator_t<node_allocator_t, transition_allocator_t>;
            
            using builder_t = tchecker::covreach::full_states_builder_t<ts_t, ts_allocator_t>;
            
            using graph_t = tchecker::covreach::graph_t<key_t, ts_t, ts_allocator_t>;
            
            static inline bool valid_final_node(ts_t const & ts,
                                                node_ptr_t const & node)
            {
              return true;
            }

            static inline key_t node_to_key(node_ptr_t const & node)
            {
              return tchecker::ta::details::hash_value(*node);
            }
            
            class state_predicate_t {
            public:
              using node_ptr_t = typename tchecker::covreach::details::zg::ta::algorithm_model_t<ZONE_SEMANTICS>::node_ptr_t;
              
              bool operator() (node_ptr_t const & n1, node_ptr_t const & n2)
              {
                return (static_cast<tchecker::ta::state_t const &>(*n1) == static_cast<tchecker::ta::state_t const &>(*n2));
              }
            };
            
            class node_lt_t {
            public:
              bool operator() (node_ptr_t const & n1, node_ptr_t const & n2) const
              {
                return tchecker::lexical_cmp(*n1, *n2) < 0;
              }
            };
            
            static std::tuple<> state_predicate_args(model_t const & model)
            {
              return std::tuple<>();
            }
            
            static std::tuple<model_t const &> zone_predicate_args(model_t const & model)
            {
              return std::tuple<model_t const &>(model);
            }
            
            static std::tuple<model_t &> ts_args(model_t & model,
                                                 tchecker::covreach::options_t const & options)
            {
              return std::tuple<model_t &>(model);
            }
            
            using node_outputter_t = tchecker::zg::ta::state_outputter_t;
            
            static std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
            node_outputter_args(model_t const & model)
            {
              return std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
              (model.flattened_integer_variables().index(), model.flattened_clock_variables().index());
            }
          };
          
        } // end of namespace ta
        
      } // end of namespace zg

    } // end of namespace details

  } // enf of namespace covreach

} // end of namespace tchecker

#endif // TCHECKER_ALGORITHMS_COVREACH_ZG_TA_HH