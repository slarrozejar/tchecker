/*
 * This file is a part of the TChecker project.
 *
 * See files AUTHORS and LICENSE for copyright details.
 *
 */

#include <fstream>
#include <iterator>

#include <boost/tokenizer.hpp>

#include "tchecker/algorithms/covreach/options.hh"

namespace tchecker {

  namespace covreach {

    options_t::options_t(tchecker::covreach::options_t && options)
    : _node_covering(options._node_covering),
    _output_format(std::move(options._output_format)),
    _accepting_labels(options._accepting_labels),
    _algorithm_model(std::move(options._algorithm_model)),
    _os(options._os),
    _search_order(std::move(options._search_order)),
    _block_size(std::move(options._block_size)),
    _nodes_table_size(std::move(options._nodes_table_size)),
    _source_set(std::move(options._source_set)),
    _stats(options._stats),
    _spread(options._spread)
    {
      options._os = nullptr;
    }


    options_t::~options_t()
    {
      _os->flush();
      if (_os != &std::cout)
        delete _os;
    }


    tchecker::covreach::options_t & options_t::operator= (tchecker::covreach::options_t && options)
    {
      if (this != &options) {
        _node_covering = std::move(options._node_covering);
        _output_format = std::move(options._output_format);
        _accepting_labels = std::move(options._accepting_labels);
        _algorithm_model = std::move(options._algorithm_model);
        _os = options._os;
        options._os = nullptr;
        _search_order = std::move(options._search_order);
        _block_size = options._block_size;
        _nodes_table_size = options._nodes_table_size;
        _source_set = options._source_set;
        _stats = options._stats;
        _spread = options._spread;
      }
      return *this;
    }


    enum tchecker::covreach::options_t::node_covering_t options_t::node_covering() const
    {
      return _node_covering;
    }


    enum tchecker::covreach::options_t::output_format_t options_t::output_format() const
    {
      return _output_format;
    }


    tchecker::covreach::options_t::accepting_labels_range_t options_t::accepting_labels() const
    {
      return tchecker::make_range(_accepting_labels);
    }


    enum tchecker::covreach::options_t::algorithm_model_t options_t::algorithm_model() const
    {
      return _algorithm_model;
    }


    std::ostream & options_t::output_stream() const
    {
      return *_os;
    }


    enum tchecker::covreach::options_t::search_order_t options_t::search_order() const
    {
      return _search_order;
    }


    std::size_t options_t::block_size() const
    {
      return _block_size;
    }


    std::size_t options_t::nodes_table_size() const
    {
      return _block_size;
    }


    enum tchecker::covreach::options_t::source_set_t options_t::source_set() const
    {
      return _source_set;
    }


    bool options_t::stats() const
    {
      return (_stats == 1);
    }

    tchecker::integer_t options_t::spread() const
    {
      return _spread;
    }


    std::string const & options_t::server_process() const
    {
      return _server_process;
    }


    void options_t::set_option(std::string const & key, std::string const & value, tchecker::log_t & log)
    {
      if (key == "c")
        set_node_covering(value, log);
      else if (key == "f")
        set_output_format(value, log);
      else if (key == "h") {
      }
      else if (key == "l")
        set_accepting_labels(value, log);
      else if (key == "m")
        set_algorithm_model(value, log);
      else if (key == "o")
        set_output_file(value, log);
      else if (key == "s")
        set_search_order(value, log);
      else if (key == "block-size")
        set_block_size(value, log);
      else if (key == "table-size")
        set_nodes_table_size(value, log);
      else if (key == "server")
        set_server_process(value, log);
      else if (key == "source-set")
        set_source_set(value, log);
      else if (key == "spread")
        set_spread(value, log);
      else if (key == "S")
        set_stats(value, log);
      else
        log.warning("Unknown command line option " + key);
    }


    void options_t::set_node_covering(std::string const & value, tchecker::log_t & log)
    {
      if (value == "inclusion")
        _node_covering = tchecker::covreach::options_t::INCLUSION;
      else if (value == "aLUg")
        _node_covering = tchecker::covreach::options_t::ALU_G;
      else if (value == "aLUl")
        _node_covering = tchecker::covreach::options_t::ALU_L;
      else if (value == "aMg")
        _node_covering = tchecker::covreach::options_t::AM_G;
      else if (value == "aMl")
        _node_covering = tchecker::covreach::options_t::AM_L;
      else
        log.error("Unkown node covering: " + value + " for commande line parameter -c");
    }


    void options_t::set_output_format(std::string const & value, tchecker::log_t & log)
    {
      if (value == "dot")
        _output_format = tchecker::covreach::options_t::DOT;
      else if (value == "raw")
        _output_format = tchecker::covreach::options_t::RAW;
      else
        log.error("Unkown output format: " + value + " for commande line parameter -f");
    }


    void options_t::set_accepting_labels(std::string const & value, tchecker::log_t & log)
    {
      boost::char_separator<char> sep(":");
      boost::tokenizer<boost::char_separator<char> > tokenizer(value, sep);
      auto it = tokenizer.begin(), end = tokenizer.end();
      for ( ; it != end; ++it)
        _accepting_labels.push_back(*it);
    }


    void options_t::set_algorithm_model(std::string const & value, tchecker::log_t & log)
    {
      boost::char_separator<char> sep(":");
      boost::tokenizer<boost::char_separator<char> > tokenizer(value, sep);
      auto begin = tokenizer.begin(), end = tokenizer.end();

      std::size_t count = std::distance(begin, end);

      if ((count < 2) || (count > 3)) {
        log.error("Unknown model: " + value + " for command line parameter -m");
        return;
      }

      std::string graph = *(begin++);
      std::string semantics = *(begin++);
      std::string extrapolation = (count == 2 ? "" : *(begin++));

      if (graph == "async_zg")
        set_algorithm_model_async_zg(semantics, extrapolation, log);
      else if (graph == "zg")
        set_algorithm_model_zg(semantics, extrapolation, log);
      else
        log.error("Unknown graph: " + graph + " for command line parameter -m");
    }


    void options_t::set_algorithm_model_async_zg(std::string const & semantics, std::string const & extrapolation,
                                                 tchecker::log_t & log)
    {
      if (semantics == "elapsed") {
        if (extrapolation == "")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED;
        else if (extrapolation == "NOextra")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_NOEXTRA;
        else if (extrapolation == "extraMg")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_G;
        else if (extrapolation == "extraMl")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_L;
        else if (extrapolation == "extraM+g")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_G;
        else if (extrapolation == "extraM+l")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRAM_PLUS_L;
        else if (extrapolation == "extraLUg")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_G;
        else if (extrapolation == "extraLUl")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_L;
        else if (extrapolation == "extraLU+g")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_G;
        else if (extrapolation == "extraLU+l")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_ELAPSED_EXTRALU_PLUS_L;
        else
          log.error("Unsupported extrapolation: " + extrapolation + " for command line parameter -m");
      }
      else if (semantics == "non-elapsed") {
        if (extrapolation == "")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED;
        else if (extrapolation == "NOextra")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_NOEXTRA;
        else if (extrapolation == "extraMg")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_G;
        else if (extrapolation == "extraMl")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_L;
        else if (extrapolation == "extraM+g")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_G;
        else if (extrapolation == "extraM+l")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRAM_PLUS_L;
        else if (extrapolation == "extraLUg")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_G;
        else if (extrapolation == "extraLUl")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_L;
        else if (extrapolation == "extraLU+g")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_G;
        else if (extrapolation == "extraLU+l")
          _algorithm_model = tchecker::covreach::options_t::ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L;
        else
          log.error("Unsupported extrapolation: " + extrapolation + " for command line parameter -m");
      }
      else
        log.error("Unknown semantics: " + semantics + " for command line parameter -m");
    }


    void options_t::set_algorithm_model_zg(std::string const & semantics, std::string const & extrapolation, tchecker::log_t & log)
    {
      if (semantics == "elapsed") {
        if (extrapolation == "NOextra")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_NOEXTRA;
        else if (extrapolation == "extraMg")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_G;
        else if (extrapolation == "extraMl")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_L;
        else if (extrapolation == "extraM+g")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_PLUS_G;
        else if (extrapolation == "extraM+l")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRAM_PLUS_L;
        else if (extrapolation == "extraLUg")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_G;
        else if (extrapolation == "extraLUl")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_L;
        else if (extrapolation == "extraLU+g")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_PLUS_G;
        else if (extrapolation == "extraLU+l")
          _algorithm_model = tchecker::covreach::options_t::ZG_ELAPSED_EXTRALU_PLUS_L;
        else
          log.error("Unknown extrapolation: " + extrapolation + " for command line parameter -m");
      }
      else if (semantics == "non-elapsed") {
        if (extrapolation == "NOextra")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_NOEXTRA;
        else if (extrapolation == "extraMg")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_G;
        else if (extrapolation == "extraMl")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_L;
        else if (extrapolation == "extraM+g")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_PLUS_G;
        else if (extrapolation == "extraM+l")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRAM_PLUS_L;
        else if (extrapolation == "extraLUg")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_G;
        else if (extrapolation == "extraLUl")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_L;
        else if (extrapolation == "extraLU+g")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_PLUS_G;
        else if (extrapolation == "extraLU+l")
          _algorithm_model = tchecker::covreach::options_t::ZG_NON_ELAPSED_EXTRALU_PLUS_L;
        else
          log.error("Unknown extrapolation: " + extrapolation + " for command line parameter -m");
      }
      else
        log.error("Unknown semantics: " + semantics + " for command line parameter -m");
    }


    void options_t::set_output_file(std::string const & filename, tchecker::log_t & log)
    {
      if (_os != &std::cout)
        delete _os;
      _os = new std::ofstream(filename, std::ios::out);
      if (_os->fail()) {
        log.error("Unable to open file: " + filename + " for command line parameter -o");
        return;
      }
    }


    void options_t::set_search_order(std::string const & value, tchecker::log_t & log)
    {
      if (value == "bfs")
        _search_order = tchecker::covreach::options_t::BFS;
      else if (value == "dfs")
        _search_order = tchecker::covreach::options_t::DFS;
      else
        log.error("Unknown search order: " + value + " for command line option -s");
    }


    void options_t::set_block_size(std::string const & value, tchecker::log_t & log)
    {
      for (auto c : value)
        if (! isdigit(c)) {
          log.error("Invalid value: " + value + " for command line option --block-size, expecting an unsigned integer");
          return;
        }

      _block_size = std::stoul(value);
    }


    void options_t::set_nodes_table_size(std::string const & value, tchecker::log_t & log)
    {
      for (auto c : value)
        if (! isdigit(c)) {
          log.error("Invalid value: " + value + " for command line option --table-size, expecting an unsigned integer");
          return;
        }

      _nodes_table_size = std::stoul(value);
    }


    void options_t::set_source_set(std::string const & value, tchecker::log_t & log)
    {
      if (value == "cs")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_CS;
      else if (value == "gl")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_GL;
      else if (value == "por1")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_POR1;
      else if (value == "por2")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_POR2;
      else if (value == "por3")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_POR3;
      else if (value == "por4")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_POR4;
      else if (value == "por5")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_POR5;
      else if (value == "mag")
        _source_set = tchecker::covreach::options_t::SOURCE_SET_MAG;
      else
        log.error("Unknown source set: " + value);
    }


    void options_t::set_stats(std::string const & value, tchecker::log_t & log)
    {
      _stats = 1;
    }


    void options_t::set_spread(std::string const & value, tchecker::log_t & log)
    {
      for (auto c : value)
      if (! isdigit(c)) {
        log.error("Invalid value: " + value + " for command line option --spread, expecting an unsigned integer");
        return;
      }

      unsigned long spread = std::stoul(value);
      if (spread >= tchecker::covreach::options_t::UNBOUNDED_SPREAD)
        log.error("Out-of-bound spread " + value);

      _spread = static_cast<tchecker::integer_t>(spread);
    }


    void options_t::set_server_process(std::string const & value, tchecker::log_t & log)
    {
      _server_process = value;
    }


    void options_t::check_mandatory_options(tchecker::log_t & log) const
    {
      if (_algorithm_model == UNKNOWN)
        log.error("model must be set, use -m command line option");
    }


    void options_t::check_source_set_model(tchecker::log_t & log) const
    {
      if ((_source_set != tchecker::covreach::options_t::SOURCE_SET_ALL) &&
          ((_algorithm_model < ASYNC_ZG_ELAPSED) ||
           (_algorithm_model > ASYNC_ZG_NON_ELAPSED_EXTRALU_PLUS_L)))
        log.error("source set can only be used with asynchronous zone graph models");
      else if (_source_set == options_t::SOURCE_SET_CS && _server_process == "")
        log.error("server process not set for client/server POR");
      else if (_source_set == options_t::SOURCE_SET_POR1 && _server_process == "")
        log.error("server process not set for client/server por1 POR");
      else if (_source_set == options_t::SOURCE_SET_POR2 && _server_process == "")
        log.error("server process not set for client/server por2 POR");
      else if (_source_set == options_t::SOURCE_SET_POR3 && _server_process == "")
        log.error("server process not set for client/server por3 POR");
      else if (_source_set == options_t::SOURCE_SET_POR4 && _server_process == "")
        log.error("server process not set for client/server por4 POR");
      else if (_source_set == options_t::SOURCE_SET_POR5 && _server_process == "")
        log.error("server process not set for client/server por5 POR");
      else if (_source_set == options_t::SOURCE_SET_MAG && _server_process == "")
        log.error("server process not set for client/server magnetic POR");
      else if (_server_process != "" && _source_set != options_t::SOURCE_SET_CS && _source_set != options_t::SOURCE_SET_POR1
              && _source_set != options_t::SOURCE_SET_POR2 && _source_set != options_t::SOURCE_SET_POR3
              && _source_set != options_t::SOURCE_SET_POR4 && _source_set != options_t::SOURCE_SET_POR5
              && _source_set != options_t::SOURCE_SET_MAG)
        log.warning("server process ignored if not used in combination with client/server POR");
    }


    void options_t::check_spread(tchecker::log_t & log) const
    {
      if (_spread < 0)
        log.error("spread should be >= 0");
      if ((_spread > 0) &&
          (_spread != tchecker::covreach::options_t::UNBOUNDED_SPREAD) &&
          (_algorithm_model != ASYNC_ZG_ELAPSED) &&
          (_algorithm_model != ASYNC_ZG_NON_ELAPSED))
        log.error("spread should only be specified for models async_zg:elapsed and async_zg::non_elapsed");
    }


    std::ostream & options_t::describe(std::ostream & os)
    {
      os << "-c cover         where cover is one of the following:" << std::endl;
      os << "                 inclusion     zone inclusion" << std::endl;
      os << "                 aLUg          aLU abstraction with global clock bounds" << std::endl;
      os << "                 aLUl          aLU abstraction with local clock bounds" << std::endl;
      os << "                 aMg           aM abstraction with global clock bounds" << std::endl;
      os << "                 aMl           aM abstraction with local clock bounds" << std::endl;
      os << "-f (dot|raw)     output format (graphviz DOT format or raw format)" << std::endl;
      os << "-h               this help screen" << std::endl;
      os << "-l labels        accepting labels, where labels is a column-separated list of identifiers" << std::endl;
      os << "-m model         where model is one of the following:" << std::endl;
      os << "                 graph:semantics:extrapolation     zone graph with:" << std::endl;
      os << "                   graph:          async_zg        asynchronous zone graph" << std::endl;
      os << "                                   zg              zone graph" << std::endl;
      os << "                   semantics:      elapsed         time-elapsed semantics" << std::endl;
      os << "                                   non-elapsed     non time-elapsed semantics" << std::endl;
      os << "                   extrapolation:  NOextra         no zone extrapolation" << std::endl;
      os << "                                   extraMg         ExtraM with global clock bounds" << std::endl;
      os << "                                   extraMl         ExtraM with local clock bounds" << std::endl;
      os << "                                   extraM+g        ExtraM+ with global clock bounds" << std::endl;
      os << "                                   extraM+l        ExtraM+ with local clock bounds" << std::endl;
      os << "                                   extraLUg        ExtraLU with global clock bounds" << std::endl;
      os << "                                   extraLUl        ExtraLU with local clock bounds" << std::endl;
      os << "                                   extraLU+g       ExtraLU+ with global clock bounds" << std::endl;
      os << "                                   extraLU+l       ExtraLU+ with local clock bounds" << std::endl;
      os << "-o filename      output graph to filename" << std::endl;
      os << "-s (bfs|dfs)     search order (breadth-first search or depth-first search)" << std::endl;
      os << "-S               output stats" << std::endl;
      os << "--server name    server process name (client/server POR)" << std::endl;
      os << "--spread n       bound on spread for asynchronous zone graph" << std::endl;
      os << "--source-set ss  where ss is one of:" << std::endl;
      os << "                 cs    partial-order reduction for client/server models" << std::endl;
      os << "                 gl    partial-order reduction for global/local models" << std::endl;
      os << "                 por1  partial-order reduction that implements por1 client/server POR" << std::endl;
      os << "                 por2  partial-order reduction that implements por2 client/server POR" << std::endl;
      os << "                 por3  partial-order reduction that implements por3 client/server POR" << std::endl;
      os << "                 por4  partial-order reduction that implements por4 client/server POR" << std::endl;
      os << "                 por5  partial-order reduction that implements por5 client/server POR" << std::endl;
      os << "                 mag  partial-order reduction that implements magnetic client/server POR" << std::endl;
      os << "--block-size n   size of an allocation block (number of allocated objects)" << std::endl;
      os << "--table-size n   size of the nodes table" << std::endl;
      os << std::endl;
      os << "Default parameters: -c inclusion -f raw -s dfs --block-size 10000 --table-size 65536, output to standard output";
      os << std::endl;
      os << "                    -m must be specified" << std::endl;
      return os;
    }

  } // end of namespace covreach

} // end of namespace tchecker
