/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#ifndef TCHECKER_ALGORITHMS_COVREACH_ALGORITHM_HH
#define TCHECKER_ALGORITHMS_COVREACH_ALGORITHM_HH

#include <functional>
#include <iterator>
#include <tuple>
#include <vector>

#include "tchecker/basictypes.hh"
#include "tchecker/algorithms/covreach/builder.hh"
#include "tchecker/algorithms/covreach/graph.hh"
#include "tchecker/algorithms/covreach/stats.hh"

/*!
 \file algorithm.hh
 \brief Reachability algorithm with covering
 */

namespace tchecker {
  
  namespace covreach {
    
    /*!
     \brief Type of accepting condition
     */
    template <class NODE_PTR>
    using accepting_condition_t = std::function<bool(NODE_PTR const &)>;
    
    
    /*!
     \brief Type of verdict
     */
    enum outcome_t {
      REACHABLE,        /*!< Accepting state reachable */
      UNREACHABLE,      /*!< Accepting state unreachable */
    };
    
    
    /*!
     \class algorithm_t
     \brief Reachability algorithm with node covering
     \tparam BUILDER : type of states builder over a transition system, should
     implement tchecker::covreach::state_builder_t
     \tparam GRAPH : type of graph, should derive from tchecker::covreach::graph_t
     \tparam WAITING : type of waiting container, should derive from tchecker::covreach::active_waiting_t
     */
    template <class BUILDER, class GRAPH, template <class NPTR> class WAITING>
    class algorithm_t {
      using builder_t = BUILDER;
      using transition_t = typename GRAPH::ts_allocator_t::transition_t;
      using transition_ptr_t = typename GRAPH::ts_allocator_t::transition_ptr_t;
      using graph_t = GRAPH;
      using ts_allocator_t = typename GRAPH::ts_allocator_t;
      using node_ptr_t = typename GRAPH::node_ptr_t;
      using edge_ptr_t = typename GRAPH::edge_ptr_t;
      using waiting_t = WAITING<node_ptr_t>;
    public:
      /*!
       \brief Reachability algorithm with node covering
       \param builder : state builder over a transition system
       \param graph : a graph
       \param accepting : an accepting function over nodes
       \pre accepting is monotonous w.r.t. the ordering over nodes in graph: if a node is accepting,
       then any bigger node is accepting as well (partially checked by assertion)
       \post this algorithm visits a transition system using builder, and builds graph. Graph stores the maximal nodes in ts and edges
       between them. There are two kind of edges: actual edges which correspond to a transition in ts,
       and abstract edges. There is an abstract edge n1->n2 when the actual successor of n1 in ts is
       smaller than n2 (for some n2 in the graph). The order in which the states
       are visited depends on the policy implemented by WAITING, and the
       order in which the states are produced by builder
       The algorithms stops when an accepting node has been found, or when the
       transition system has been entirely visited.
       \return REACHABLE if the transition system has an accepting run, UNREACHABLE otherwise
       \note this algorithm may not terminate if the transition system is not finite
       */
      std::tuple<enum tchecker::covreach::outcome_t, tchecker::covreach::stats_t>
      run(BUILDER & builder, GRAPH & graph, tchecker::covreach::accepting_condition_t<node_ptr_t> accepting)
      {
        waiting_t waiting;
        node_ptr_t node{nullptr}, next_node{nullptr}, covering_node{nullptr};
        std::vector<node_ptr_t> nodes, covered_nodes;
        auto covered_nodes_inserter = std::back_inserter(covered_nodes);
        tchecker::covreach::stats_t stats;
        
        // initial nodes
        assert(nodes.empty());
        expand_initial_nodes(builder, graph, nodes);
        for (node_ptr_t const & n : nodes)
          waiting.insert(n);
        nodes.clear();
        
        // explore waiting nodes
        while (! waiting.empty()) {
          node = waiting.first();
          waiting.remove_first();
          
          stats.increment_visited_nodes();
          
          if (accepting(node))
            return std::make_tuple(tchecker::covreach::REACHABLE, stats);
          
          // expand node
          assert(nodes.empty());
          expand_node(node, builder, graph, nodes);
          
          // remove small nodes
          for (node_ptr_t & next_node : nodes) {
            if (! next_node->is_active())   // covered by another node in next_nodes
              continue;
            
            if (graph.is_covered(next_node, covering_node)) {
              cover_node(next_node, covering_node, graph);
              next_node->make_inactive();
              stats.increment_covered_leaf_nodes();
              continue;
            }
            
            waiting.insert(next_node);
            
            covered_nodes.clear();
            graph.covered_nodes(next_node, covered_nodes_inserter);
            for (node_ptr_t & covered_node : covered_nodes) {
              waiting.remove(covered_node);
              cover_node(covered_node, next_node, graph);
              covered_node->make_inactive();
              stats.increment_covered_nonleaf_nodes();
            }
          }
          nodes.clear();
        }
        
        return std::make_tuple(tchecker::covreach::UNREACHABLE, stats);
      }
      
      
      /*!
       \brief Expand initial nodes
       \param builder : a nodes builder
       \param graph : a graph
       \param nodes : a vector of nodes
       \post the initial nodes provided by builder have been added to graph and to nodes
       */
      void expand_initial_nodes(BUILDER & builder, GRAPH & graph, std::vector<node_ptr_t> & nodes)
      {
        builder.initial(nodes);
        for (node_ptr_t node : nodes)
          graph.add_node(node, GRAPH::ROOT_NODE);
      }
      
      
      /*!
       \brief Expand node
       \param node : a node
       \param builder : a nodes builder
       \param graph : a graph
       \param nodes : a vector of nodes
       \post the successor nodes of n provided by builder have been added to graph and to nodes
       */
      void expand_node(node_ptr_t & node, 
      BUILDER & builder,
      GRAPH & graph,
      std::vector<node_ptr_t> & nodes)
      {
        builder.next(node, nodes);
        for (node_ptr_t next_node : nodes) {
          graph.add_node(next_node);
          graph.add_edge(node, next_node, tchecker::covreach::ACTUAL_EDGE);
        }
      }
      
      
      /*!
       \brief Cover a node
       \param covered_node : covered node
       \param covering_node : covering node
       \param graph : a graph
       \post graph has been updated to let covering_node replace covered_node
       */
      void cover_node(node_ptr_t & covered_node, node_ptr_t & covering_node, GRAPH & graph)
      {
        graph.move_incoming_edges(covered_node, covering_node, tchecker::covreach::ABSTRACT_EDGE);
        graph.remove_edges(covered_node);
        graph.remove_node(covered_node);
      }
    };
    
  } // end of namespace covreach
  
} // end of namespace tchecker

#endif // TCHECKER_ALGORITHMS_COVREACH_ALGORITHM_HH
