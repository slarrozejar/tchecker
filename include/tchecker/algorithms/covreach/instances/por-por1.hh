/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ALGORITHMS_COVREACH_POR_POR1_HH
#define TCHECKER_ALGORITHMS_COVREACH_POR_POR1_HH

#include "tchecker/algorithms/covreach/graph.hh"
#include "tchecker/algorithms/covreach/options.hh"
#include "tchecker/async_zg/async_zg_ta.hh"
#include "tchecker/async_zg/bounded_spread/async_zg_ta.hh"
#include "tchecker/async_zg/sync_zones/async_zg_ta.hh"
#include "tchecker/por/por1/builder.hh"
#include "tchecker/por/por1/output.hh"
#include "tchecker/por/por1/state.hh"
#include "tchecker/por/por1/ts.hh"

/*!
 \file por-por1.hh
 \brief Instantiation of covreach algorithm for asynchronous zone graph with
 por1 partial-order reduction
 */

namespace tchecker {

  namespace covreach {

    namespace details {

      namespace por {

        namespace por1 {

          namespace async_zg {

            namespace ta {

              /*!
               \class algorithm_model_t
               \brief Model for covering reachability asynchronous zone graphs of por1 timed automata with POR
               */
              template <class ZONE_SEMANTICS>
              class algorithm_model_t {
              public:
                using zone_semantics_t = ZONE_SEMANTICS;
                using model_t = tchecker::async_zg::ta::model_t;
                using ts_t = tchecker::por::por1::ts_t<typename zone_semantics_t::ts_t>;
                using state_t = typename ts_t::state_t;
                using transition_t = typename ts_t::transition_t;

                using key_t = std::size_t;

                using graph_types_t = typename tchecker::covreach::details::graph_types_t<ts_t>;
                using node_t = typename graph_types_t::node_t;
                using node_ptr_t = typename graph_types_t::node_ptr_t;

                using node_allocator_t = typename zone_semantics_t::template state_pool_allocator_t<node_t>;
                using transition_allocator_t
                = typename zone_semantics_t::template transition_singleton_allocator_t<transition_t>;
                using ts_allocator_t = tchecker::ts::allocator_t<node_allocator_t, transition_allocator_t>;

                using builder_t = tchecker::por::por1::states_builder_t<ts_t, ts_allocator_t>;

                using graph_t = tchecker::covreach::graph_t<key_t, ts_t, ts_allocator_t>;

                static inline bool valid_final_node(ts_t const & ts,
                                                    node_ptr_t const & node)
                {
                  return ts.synchronizable_zone(*node);
                }

                static inline key_t node_to_key(node_ptr_t const & node)
                {
                  return tchecker::ta::details::hash_value(*node);
                  // NB: we don't hash node->rank() since we want to compare nodes with same ta state,
                  // but distinct ranks
                }


                class state_predicate_t {
                public:
                  using node_ptr_t
                  = typename tchecker::covreach::details::por::por1::async_zg::ta::
                  algorithm_model_t<ZONE_SEMANTICS>::node_ptr_t;

                  bool operator() (node_ptr_t const & n1, node_ptr_t const & n2)
                  {
                    return ((static_cast<tchecker::ta::state_t const &>(*n1)
                             == static_cast<tchecker::ta::state_t const &>(*n2))
                            &&
                            tchecker::por::por1::cover_leq(*n1, *n2)
                            );
                  }
                };

                class node_lt_t {
                public:
                  bool operator() (node_ptr_t const & n1, node_ptr_t const & n2) const
                  {
                    int cmp = tchecker::lexical_cmp(*n1, *n2);
                    if (cmp < 0)
                      return true;
                    if (cmp > 0)
                      return false;
                    return tchecker::por::por1::lexical_cmp(*n1, *n2) < 0;
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

                static std::tuple<model_t &>
                ts_args(model_t & model, tchecker::covreach::options_t const & options)
                {
                  return std::tuple<model_t &>(model);
                }

                static std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>
                builder_args(model_t & model,
                             tchecker::covreach::options_t const & options,
                             ts_t & ts,
                             ts_allocator_t & allocator)
                {
                  return std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>(model, options.server_process(), ts, allocator);
                }

                using node_outputter_t
                = tchecker::por::por1::state_outputter_t
                <typename zone_semantipor1_t::ts_t::state_t,
                tchecker::async_zg::ta::state_outputter_t>;

                static std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
                node_outputter_args(model_t const & model)
                {
                  return
                  std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
                  (model.flattened_integer_variables().index(),
                   model.flattened_offset_clock_variables().index());
                }
              };

            } // end of namespace ta



            namespace sync_zones {

              namespace ta {

                /*!
                 \class algorithm_model_t
                 \brief Model for covering reachability over sync-zones asynchronous zone graphs of client/server timed automata with POR
                 */
                template <class ZONE_SEMANTICS>
                class algorithm_model_t {
                public:
                  using zone_semantics_t = ZONE_SEMANTICS;
                  using model_t = tchecker::async_zg::sync_zones::ta::model_t;
                  using ts_t = tchecker::por::por1::ts_t<typename zone_semantics_t::ts_t>;
                  using state_t = typename ts_t::state_t;
                  using transition_t = typename ts_t::transition_t;

                  using key_t = std::size_t;

                  using graph_types_t = typename tchecker::covreach::details::graph_types_t<ts_t>;
                  using node_t = typename graph_types_t::node_t;
                  using node_ptr_t = typename graph_types_t::node_ptr_t;

                  using node_allocator_t = typename zone_semantics_t::template state_pool_allocator_t<node_t>;
                  using transition_allocator_t
                  = typename zone_semantics_t::template transition_singleton_allocator_t<transition_t>;
                  using ts_allocator_t = tchecker::ts::allocator_t<node_allocator_t, transition_allocator_t>;

                  using builder_t = tchecker::por::por1::states_builder_t<ts_t, ts_allocator_t>;

                  using graph_t = tchecker::covreach::graph_t<key_t, ts_t, ts_allocator_t>;

                  static inline bool valid_final_node(ts_t const & ts,
                                                      node_ptr_t const & node)
                  {
                    return ts.synchronizable_zone(*node);
                  }

                  static inline key_t node_to_key(node_ptr_t const & node)
                  {
                    return tchecker::ta::details::hash_value(*node);
                    // NB: we don't hash node->rank() since we want to compare nodes with same ta state,
                    // but distinct ranks
                  }


                  class state_predicate_t {
                  public:
                    using node_ptr_t
                    = typename tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
                    algorithm_model_t<ZONE_SEMANTICS>::node_ptr_t;

                    bool operator() (node_ptr_t const & n1, node_ptr_t const & n2)
                    {
                      return ((static_cast<tchecker::ta::state_t const &>(*n1)
                               == static_cast<tchecker::ta::state_t const &>(*n2))
                              &&
                              tchecker::por::por1::cover_leq(*n1, *n2)
                              );
                    }
                  };

                  class node_lt_t {
                  public:
                    bool operator() (node_ptr_t const & n1, node_ptr_t const & n2) const
                    {
                      int cmp = tchecker::lexical_cmp(*n1, *n2);
                      if (cmp < 0)
                        return true;
                      if (cmp > 0)
                        return false;
                      return tchecker::por::por1::lexical_cmp(*n1, *n2) < 0;
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

                  static std::tuple<model_t &>
                  ts_args(model_t & model, tchecker::covreach::options_t const & options)
                  {
                    return std::tuple<model_t &>(model);
                  }

                  static std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>
                  builder_args(model_t & model,
                               tchecker::covreach::options_t const & options,
                               ts_t & ts,
                               ts_allocator_t & allocator)
                  {
                    return std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>(model, options.server_process(), ts, allocator);
                  }

                  using node_outputter_t
                  = tchecker::por::por1::state_outputter_t
                  <typename zone_semantics_t::ts_t::state_t,
                  tchecker::async_zg::sync_zones::ta::state_outputter_t>;

                  static std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &,
                  tchecker::clock_index_t const &>
                  node_outputter_args(model_t const & model)
                  {
                    return
                    std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &,
                    tchecker::clock_index_t const &>
                    (model.flattened_integer_variables().index(),
                     model.flattened_offset_clock_variables().index(),
                     model.flattened_clock_variables().index());
                  }
                };

              } // end of namespace ta

            } // end of namespace sync_zones



            namespace bounded_spread {

              namespace ta {

                /*!
                 \class algorithm_model_t
                 \brief Model for covering reachability over bounded spread asynchronous zone graphs
                 of por1 timed automata with POR
                 */
                template <class ZONE_SEMANTICS>
                class algorithm_model_t {
                public:
                  using zone_semantics_t = ZONE_SEMANTICS;
                  using model_t = tchecker::async_zg::bounded_spread::ta::model_t;
                  using ts_t = tchecker::por::por1::ts_t<typename zone_semantics_t::ts_t>;
                  using state_t = typename ts_t::state_t;
                  using transition_t = typename ts_t::transition_t;

                  using key_t = std::size_t;

                  using graph_types_t = typename tchecker::covreach::details::graph_types_t<ts_t>;
                  using node_t = typename graph_types_t::node_t;
                  using node_ptr_t = typename graph_types_t::node_ptr_t;

                  using node_allocator_t = typename zone_semantics_t::template state_pool_allocator_t<node_t>;
                  using transition_allocator_t
                  = typename zone_semantics_t::template transition_singleton_allocator_t<transition_t>;
                  using ts_allocator_t = tchecker::ts::allocator_t<node_allocator_t, transition_allocator_t>;

                  using builder_t = tchecker::por::por1::states_builder_t<ts_t, ts_allocator_t>;

                  using graph_t = tchecker::covreach::graph_t<key_t, ts_t, ts_allocator_t>;

                  static inline bool valid_final_node(ts_t const & ts,
                                                      node_ptr_t const & node)
                  {
                    return ts.synchronizable_zone(*node);
                  }

                  static inline key_t node_to_key(node_ptr_t const & node)
                  {
                    return tchecker::ta::details::hash_value(*node);
                    // NB: we don't hash node->rank() since we want to compare nodes with same ta state,
                    // but distinct ranks
                  }


                  class state_predicate_t {
                  public:
                    using node_ptr_t
                    = typename tchecker::covreach::details::por::por1::async_zg::bounded_spread::ta::
                    algorithm_model_t<ZONE_SEMANTICS>::node_ptr_t;

                    bool operator() (node_ptr_t const & n1, node_ptr_t const & n2)
                    {
                      return ((static_cast<tchecker::ta::state_t const &>(*n1)
                               == static_cast<tchecker::ta::state_t const &>(*n2))
                              &&
                              tchecker::por::por1::cover_leq(*n1, *n2)
                              );
                    }
                  };

                  class node_lt_t {
                  public:
                    bool operator() (node_ptr_t const & n1, node_ptr_t const & n2) const
                    {
                      int cmp = tchecker::lexical_cmp(*n1, *n2);
                      if (cmp < 0)
                        return true;
                      if (cmp > 0)
                        return false;
                      return tchecker::por::por1::lexical_cmp(*n1, *n2) < 0;
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

                  static std::tuple<model_t &, tchecker::integer_t>
                  ts_args(model_t & model, tchecker::covreach::options_t const & options)
                  {
                    return std::tuple<model_t &, tchecker::integer_t>
                    (model, options.spread());
                  }

                  static std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>
                  builder_args(model_t & model,
                               tchecker::covreach::options_t const & options,
                               ts_t & ts,
                               ts_allocator_t & allocator)
                  {
                    return std::tuple<model_t &, std::string const &, ts_t &, ts_allocator_t &>(model, options.server_process(), ts, allocator);
                  }

                  using node_outputter_t
                  = tchecker::por::por1::state_outputter_t
                  <typename zone_semantics_t::ts_t::state_t,
                  tchecker::async_zg::bounded_spread::ta::state_outputter_t>;

                  static std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
                  node_outputter_args(model_t const & model)
                  {
                    return
                    std::tuple<tchecker::intvar_index_t const &, tchecker::clock_index_t const &>
                    (model.flattened_integer_variables().index(),
                     model.flattened_offset_clock_variables().index());
                  }
                };

              } // end of namespace ta

            } // end of namespace bounded_spread

          } // end of namespace async_zg

        } // end of namespace por1

      } // edd of namespace por

    } // end of namespace details

  } // end of namespace covreach

} // end of namespace tchecker

#endif // TCHECKER_ALGORITHMS_COVREACH_POR_POR1_HH
