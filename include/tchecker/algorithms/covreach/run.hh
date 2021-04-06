/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ALGORITHMS_COVREACH_RUN_HH
#define TCHECKER_ALGORITHMS_COVREACH_RUN_HH

#include <string>

#if BOOST_VERSION <= 106600
# include <boost/functional/hash.hpp>
#else
# include <boost/container_hash/hash.hpp>
#endif

#include "tchecker/algorithms/covreach/accepting.hh"
#include "tchecker/algorithms/covreach/algorithm.hh"
#include "tchecker/algorithms/covreach/cover.hh"
#include "tchecker/algorithms/covreach/graph.hh"
#include "tchecker/algorithms/covreach/instances/async_zg_ta.hh"
#include "tchecker/algorithms/covreach/instances/por-cs.hh"
#include "tchecker/algorithms/covreach/instances/por-gl.hh"
#include "tchecker/algorithms/covreach/instances/por-por1.hh"
#include "tchecker/algorithms/covreach/instances/por-por2.hh"
#include "tchecker/algorithms/covreach/instances/zg_ta.hh"
#include "tchecker/algorithms/covreach/options.hh"
#include "tchecker/algorithms/covreach/output.hh"
#include "tchecker/algorithms/covreach/stats.hh"
#include "tchecker/graph/allocators.hh"
#include "tchecker/graph/output.hh"
#include "tchecker/parsing/declaration.hh"
#include "tchecker/ts/allocators.hh"
#include "tchecker/utils/gc.hh"
#include "tchecker/utils/log.hh"

/*!
 \file run.hh
 \brief Running explore algorithm
 */

namespace tchecker {

  namespace covreach {

    namespace details {

      /*!
       \brief Run covering reachability algorithm
       \tparam ALGORITHM_MODEL : type of algorithm model
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param options : covering reachability algorithm options
       \param log : logging facility
       \post covering reachability algorithm has been run on a model of sysdecl as defined by
       ALGORITHM_MODEL and following options and the exploreation policy implented by WAITING.
       The graph has been output using GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template
      <template <class NODE_PTR, class STATE_PREDICATE> class COVER_NODE,
      class ALGORITHM_MODEL,
      template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER,
      template <class NPTR> class WAITING
      >
      void run(tchecker::parsing::system_declaration_t const & sysdecl,
               tchecker::covreach::options_t const & options,
               tchecker::log_t & log)
      {
        using model_t = typename ALGORITHM_MODEL::model_t;
        using ts_t = typename ALGORITHM_MODEL::ts_t;
        using builder_t = typename ALGORITHM_MODEL::builder_t;
        using graph_t = typename ALGORITHM_MODEL::graph_t;
        using node_ptr_t = typename ALGORITHM_MODEL::node_ptr_t;
        using state_predicate_t = typename ALGORITHM_MODEL::state_predicate_t;
        using cover_node_t = COVER_NODE<node_ptr_t, state_predicate_t>;

        model_t model(sysdecl, log);
        ts_t ts = std::make_from_tuple<ts_t>(ALGORITHM_MODEL::ts_args(model, options));
        cover_node_t cover_node(ALGORITHM_MODEL::state_predicate_args(model),
                                ALGORITHM_MODEL::zone_predicate_args(model));

        tchecker::label_index_t label_index(model.system().labels());
        for (std::string const & label : options.accepting_labels()) {
          if (label_index.find_value(label) == label_index.end_value_map())
            label_index.add(label);
        }

        // accepting node
        tchecker::covreach::accepting_labels_t<node_ptr_t> accepting_labels(label_index, options.accepting_labels());

        std::function<bool(node_ptr_t const &)> accepting_node =
        [&] (node_ptr_t const & n) -> bool {
          return accepting_labels(n) && ALGORITHM_MODEL::valid_final_node(ts, n);
        };

        tchecker::gc_t gc;

        graph_t graph(gc,
                      std::tuple<tchecker::gc_t &, std::tuple<model_t &, std::size_t>, std::tuple<>>
                      (gc, std::tuple<model_t &, std::size_t>(model, options.block_size()), std::make_tuple()),
                      options.block_size(),
                      options.nodes_table_size(),
                      ALGORITHM_MODEL::node_to_key,
                      cover_node);

        builder_t builder = std::make_from_tuple<builder_t>(ALGORITHM_MODEL::builder_args(model, options, ts, graph.ts_allocator()));

        gc.start();

        enum tchecker::covreach::outcome_t outcome;
        tchecker::covreach::stats_t stats;
        tchecker::covreach::algorithm_t<builder_t, graph_t, WAITING> algorithm;

        try {
          std::tie(outcome, stats) = algorithm.run(builder, graph, accepting_node);
        }
        catch (...) {
          gc.stop();
          graph.clear();
          graph.free_all();
          throw;
        }

        std::cout << "REACHABLE " << (outcome == tchecker::covreach::REACHABLE ? "true" : "false") << std::endl;

        if (options.stats()) {
          std::cout << "STORED_NODES " << graph.nodes_count() << std::endl;
          std::cout << stats << std::endl;
        }

        if (options.output_format() == tchecker::covreach::options_t::DOT) {
          tchecker::covreach::dot_outputter_t<typename ALGORITHM_MODEL::node_outputter_t>
          dot_outputter(false, ALGORITHM_MODEL::node_outputter_args(model));

          dot_outputter.template output<graph_t, typename ALGORITHM_MODEL::node_lt_t>
          (options.output_stream(), graph, model.system().name());
        }

        gc.stop();
        graph.clear();
        graph.free_all();
      }


      /*!
       \brief Run covering reachability algorithm for asynchronous zone graphs with sync zones
       \tparam ALGORITHM_MODEL : type of algorithm model
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param options : covering reachability algorithm options
       \param log : logging facility
       \post covering reachability algorithm has been run on a model of sysdecl as defined by ALGORITHM_MODEL
       and following options and the exploreation policy implented by WAITING. The graps has
       been output using GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template
      <class ALGORITHM_MODEL,
      template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER,
      template <class NPTR> class WAITING
      >
      void run_async_zg_sync_zones(tchecker::parsing::system_declaration_t const & sysdecl,
                                   tchecker::covreach::options_t const & options,
                                   tchecker::log_t & log)
      {
        if (options.node_covering() == tchecker::covreach::options_t::INCLUSION)
          tchecker::covreach::details::run<tchecker::covreach::cover_sync_inclusion_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else
          log.error("Unsupported node covering");
      }


      /*!
       \brief Run covering reachability algorithm for asynchronous zone graphs
       \tparam ALGORITHM_MODEL : type of algorithm model
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param options : covering reachability algorithm options
       \param log : logging facility
       \post covering reachability algorithm has been run on a model of sysdecl as defined by ALGORITHM_MODEL
       and following options and the exploreation policy implented by WAITING. The graps has
       been output using GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template
      <class ALGORITHM_MODEL,
      template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER,
      template <class NPTR> class WAITING
      >
      void run_async_zg(tchecker::parsing::system_declaration_t const & sysdecl,
                        tchecker::covreach::options_t const & options,
                        tchecker::log_t & log)
      {
        if (options.node_covering() == tchecker::covreach::options_t::INCLUSION)
          tchecker::covreach::details::run<tchecker::covreach::cover_async_inclusion_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else if (options.node_covering() == tchecker::covreach::options_t::ALU_G)
          tchecker::covreach::details::run<tchecker::covreach::cover_async_alu_global_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else if (options.node_covering() == tchecker::covreach::options_t::ALU_L)
          tchecker::covreach::details::run<tchecker::covreach::cover_async_alu_local_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else if (options.node_covering() == tchecker::covreach::options_t::AM_G)
          tchecker::covreach::details::run<tchecker::covreach::cover_async_am_global_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else if (options.node_covering() == tchecker::covreach::options_t::AM_L)
          tchecker::covreach::details::run<tchecker::covreach::cover_async_am_local_t, ALGORITHM_MODEL,
          GRAPH_OUTPUTTER, WAITING>
          (sysdecl, options, log);
        else
          log.error("Unsupported node covering");
      }


      /*!
       \brief Run covering reachability algorithm for zone graphs
       \tparam ALGORITHM_MODEL : type of algorithm model
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param options : covering reachability algorithm options
       \param log : logging facility
       \post covering reachability algorithm has been run on a model of sysdecl as defined by ALGORITHM_MODEL
       and following options and the exploreation policy implented by WAITING. The graps has
       been output using GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template
      <class ALGORITHM_MODEL,
      template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER,
      template <class NPTR> class WAITING
      >
      void run_zg(tchecker::parsing::system_declaration_t const & sysdecl,
                  tchecker::covreach::options_t const & options,
                  tchecker::log_t & log)
      {
        switch (options.node_covering()) {
          case tchecker::covreach::options_t::INCLUSION:
            tchecker::covreach::details::run<tchecker::covreach::cover_inclusion_t, ALGORITHM_MODEL, GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ALU_G:
            tchecker::covreach::details::run<tchecker::covreach::cover_alu_global_t, ALGORITHM_MODEL, GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ALU_L:
            tchecker::covreach::details::run<tchecker::covreach::cover_alu_local_t, ALGORITHM_MODEL, GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::AM_G:
            tchecker::covreach::details::run<tchecker::covreach::cover_am_global_t, ALGORITHM_MODEL, GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::AM_L:
            tchecker::covreach::details::run<tchecker::covreach::cover_am_local_t, ALGORITHM_MODEL, GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported node covering");
        }
      }




      /*!
       \brief Run covering reachabilty algorithm with POR for client/server systems
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param log : logging facility
       \param options : covering reachability algorithm options
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING. The graph has been output using
       GRAPH_OUTPUTTER
       Every error and warning has been reported to log.
       */
      template <template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER, template <class NPTR> class WAITING>
      void run_source_set_client_server(tchecker::parsing::system_declaration_t const & sysdecl,
                                        tchecker::covreach::options_t const & options,
                                        tchecker::log_t & log)
      {
        switch (options.algorithm_model()) {
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::cs::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::cs::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::cs::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::cs::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::cs::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported model with client/server POR");
        }
      }




      /*!
       \brief Run covering reachabilty algorithm with POR for global/local systems
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param log : logging facility
       \param options : covering reachability algorithm options
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING. The graph has been output using
       GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template <template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER, template <class NPTR> class WAITING>
      void run_source_set_global_local(tchecker::parsing::system_declaration_t const & sysdecl,
                                       tchecker::covreach::options_t const & options,
                                       tchecker::log_t & log)
      {
        switch (options.algorithm_model()) {
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::gl::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::gl::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::gl::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::gl::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::gl::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported model with global/local POR");
        }
      }


      /*!
       \brief Run covering reachabilty algorithm with POR for por1 reduction
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param log : logging facility
       \param options : covering reachability algorithm options
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING. The graph has been output using
       GRAPH_OUTPUTTER
       Every error and warning has been reported to log.
       */
      template <template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER, template <class NPTR> class WAITING>
      void run_source_set_por1(tchecker::parsing::system_declaration_t const & sysdecl,
                                        tchecker::covreach::options_t const & options,
                                        tchecker::log_t & log)
      {
        switch (options.algorithm_model()) {
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por1::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por1::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por1::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por1::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por1::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported model with por1 POR");
        }
      }

      /*!
       \brief Run covering reachabilty algorithm with POR for por2 reduction
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param log : logging facility
       \param options : covering reachability algorithm options
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING. The graph has been output using
       GRAPH_OUTPUTTER
       Every error and warning has been reported to log.
       */
      template <template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER, template <class NPTR> class WAITING>
      void run_source_set_por2(tchecker::parsing::system_declaration_t const & sysdecl,
                                        tchecker::covreach::options_t const & options,
                                        tchecker::log_t & log)
      {
        switch (options.algorithm_model()) {
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por2::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por2::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por2::async_zg::ta::
              algorithm_model_t<tchecker::async_zg::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::por::por2::async_zg::bounded_spread::ta::
              algorithm_model_t<tchecker::async_zg::bounded_spread::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::por::por2::async_zg::sync_zones::ta::
            algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported model with por2 POR");
        }
      }

      /*!
       \brief Run covering reachability algorithm
       \tparam GRAPH_OUTPUTTER : type of graph outputter
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param log : logging facility
       \param options : covering reachability algorithm options
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING. The graph has been output using
       GRAPH_OUPUTTER
       Every error and warning has been reported to log.
       */
      template <template <class N, class E, class NO, class EO> class GRAPH_OUTPUTTER, template <class NPTR> class WAITING>
      void run(tchecker::parsing::system_declaration_t const & sysdecl,
               tchecker::covreach::options_t const & options,
               tchecker::log_t & log)
      {
        if (options.source_set() == tchecker::covreach::options_t::SOURCE_SET_CS) {
          run_source_set_client_server<GRAPH_OUTPUTTER, WAITING>(sysdecl, options, log);
          return;
        }
        else if (options.source_set() == tchecker::covreach::options_t::SOURCE_SET_GL) {
          run_source_set_global_local<GRAPH_OUTPUTTER, WAITING>(sysdecl, options, log);
          return;
        }
        else if (options.source_set() == tchecker::covreach::options_t::SOURCE_SET_POR1) {
          run_source_set_por1<GRAPH_OUTPUTTER, WAITING>(sysdecl, options, log);
          return;
        }
        else if (options.source_set() == tchecker::covreach::options_t::SOURCE_SET_POR2) {
          run_source_set_por2<GRAPH_OUTPUTTER, WAITING>(sysdecl, options, log);
          return;
        }
        switch (options.algorithm_model()) {
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::async_zg::ta
              ::algorithm_model_t<tchecker::async_zg::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::async_zg::bounded_spread::ta
              ::algorithm_model_t<tchecker::async_zg::bounded_spread::ta::elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED:
            if (options.spread() == tchecker::covreach::options_t::UNBOUNDED_SPREAD)
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::async_zg::ta
              ::algorithm_model_t<tchecker::async_zg::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            else
              tchecker::covreach::details::run_async_zg
              <tchecker::covreach::details::async_zg::bounded_spread::ta
              ::algorithm_model_t<tchecker::async_zg::bounded_spread::ta::non_elapsed_no_extrapolation_t>,
              GRAPH_OUTPUTTER, WAITING>
              (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_async_zg_sync_zones
            <tchecker::covreach::details::async_zg::sync_zones::ta
            ::algorithm_model_t<tchecker::async_zg::sync_zones::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
            //
          case tchecker::covreach::options_t::ZG_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::elapsed_extraLUplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_NOEXTRA:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_no_extrapolation_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraM_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraM_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_PLUS_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraMplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_PLUS_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraMplus_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraLU_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_PLUS_G:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraLUplus_global_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_PLUS_L:
            tchecker::covreach::details::run_zg
            <tchecker::covreach::details::zg::ta::algorithm_model_t<tchecker::zg::ta::non_elapsed_extraLU_local_t>,
            GRAPH_OUTPUTTER, WAITING>
            (sysdecl, options, log);
            break;
          default:
            log.error("unsupported model");
        }
      }





      /*!
       \brief Run covering reachability algorithm
       \tparam WAITING : type of waiting container
       \param sysdecl : a system declaration
       \param options : covering reachability algorithm options
       \param log : logging facility
       \post covering reachability algorithm has been run on a model of sysdecl following options and
       the exploration policy implemented by WAITING
       Every error and warning has been reported to log.
       */
      template <template <class NPTR> class WAITING>
      void run(tchecker::parsing::system_declaration_t const & sysdecl,
               tchecker::covreach::options_t const & options,
               tchecker::log_t & log)
      {
        switch (options.output_format()) {
          case tchecker::covreach::options_t::DOT:
            tchecker::covreach::details::run<tchecker::graph::dot_outputter_t, WAITING>(sysdecl, options, log);
            break;
          case tchecker::covreach::options_t::RAW:
            tchecker::covreach::details::run<tchecker::graph::raw_outputter_t, WAITING>(sysdecl, options, log);
            break;
          default:
            log.error("unsupported output format");
        }
      }

    } // end of namespace details




    /*!
     \brief Run covering reachability algorithm
     \param sysdecl : a system declaration
     \param options : covering reachability algorithm options
     \param log : logging facility
     \post covering reachability algorithm has been run on a model of sysdecl following options.
     Every error and warning has been reported to log.
     */
    void run(tchecker::parsing::system_declaration_t const & sysdecl,
             tchecker::covreach::options_t const & options,
             tchecker::log_t & log);

  } // end of namespace covreach

} // end of namespace tchecker

#endif // TCHECKER_ALGORITHMS_COVREACH_RUN_HH
