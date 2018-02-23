// Options class
// https://gist.github.com/ksimek/4a2814ba7d74f778bbee
// boost::log
// boost::log tutorial

#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <string>
#include <cstdint>

namespace logging = boost::log;
namespace po = boost::program_options;
class Options
{
public:
    bool parse(int argc, char** argv)
    {
        std::vector<std::string> config_fnames;
        po::options_description desc("General Options");
        desc.add_options()
            ("help", "Display help message")
            ("config", po::value(&config_fnames), "Config file where options may be specified (can be specified more than once)")
            ("threshold,t", po::value<double>(&threshold)->default_value(0.25), "Threshold value")
            ("optional,o", po::bool_switch(&optional_flag), "Optional flag")
            ("log-level,l", po::value(&log_level), "trace, debug, info, warning, error (default) or fatel")
        ;

        po::options_description hidden;
        hidden.add_options()
            ("fname", po::value<std::string>(&fname)->required(), "filename")
            ;

        po::options_description all_options;
        all_options.add(desc);
        all_options.add(hidden);

        po::positional_options_description p;
        p.add("fname", -1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(all_options).
                positional(p).
                run(),
                vm);

        if(vm.count("help"))
        {
            std::cout << make_usage_string_(basename_(argv[0]), desc, p) << '\n';
            return false;
        }

        if(vm.count("config") > 0)
        {
            config_fnames = vm["config"].as<std::vector<std::string> >();

            for(size_t i = 0; i < config_fnames.size(); ++i)
            {
                std::ifstream ifs(config_fnames[i].c_str());

                if(ifs.fail())
                {
                    std::cerr << "Error opening config file: " << config_fnames[i] << std::endl;
                    return false;
                }

                po::store(po::parse_config_file(ifs, all_options), vm);
            }
        }

        if(vm.count("log-level"))
            set_log_level(vm["log-level"].as<std::string>());
        else
            set_log_level("error");



        po::notify(vm);

        return true;
    }

private:
    // https://stackoverflow.com/a/19123540/1704566
    static inline constexpr unsigned const_hash(char const *input, unsigned hash = 5381) {
        return *input ?
            const_hash(input + 1, hash * 33 + static_cast<unsigned>(*input)): 
            hash;
    }

    void set_log_level(const std::string &level)
    {
        switch (const_hash(level.c_str())) {
            case const_hash("trace"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace); break;
            case const_hash("debug"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::debug); break;
            case const_hash("info"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info); break;
            case const_hash("warning"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::warning); break;
            case const_hash("error"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error); break;
            case const_hash("fatal"):
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::fatal); break;
            default:
                logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::error); break;
        }
    }

    std::string basename_(const std::string& p)
    {
#ifdef HAVE_BOOST_FILESYSTEM
        return boost::filesystem::path(p).stem().string();
#else
        size_t start = p.find_last_of("/");
        if(start == std::string::npos)
            start = 0;
        else
            ++start;
        return p.substr(start);
#endif
    }

    // Boost doesn't offer any obvious way to construct a usage string
    // from an infinite list of positional parameters.  This hack
    // should work in most reasonable cases.
    std::vector<std::string> get_unlimited_positional_args_(const po::positional_options_description& p)
    {
        assert(p.max_total_count() == std::numeric_limits<unsigned>::max());

        std::vector<std::string> parts;

        // reasonable upper limit for number of positional options:
        const int MAX = 1000; 
        std::string last = p.name_for_position(MAX);

        for(size_t i = 0; true; ++i)
        {
            std::string cur = p.name_for_position(i);
            if(cur == last)
            {
                parts.push_back(cur);
                parts.push_back('[' + cur + ']');
                parts.push_back("...");
                return parts;
            }
            parts.push_back(cur);
        }
        return parts; // never get here
    }

    std::string make_usage_string_(const std::string& program_name, const po::options_description& desc, po::positional_options_description& p)
    {
        std::vector<std::string> parts;
        parts.push_back("Usage: ");
        parts.push_back(program_name);
        size_t N = p.max_total_count();
        if(N == std::numeric_limits<unsigned>::max())
        {
            std::vector<std::string> args = get_unlimited_positional_args_(p);
            parts.insert(parts.end(), args.begin(), args.end());
        }
        else
        {
            for(size_t i = 0; i < N; ++i)
            {
                parts.push_back(p.name_for_position(i));
            }
        }
        if(desc.options().size() > 0)
        {
            parts.push_back("[options]");
        }
        std::ostringstream oss;
        std::copy(
                parts.begin(),
                parts.end(),
                std::ostream_iterator<std::string>(oss, " "));
        oss << '\n' << desc;
        return oss.str();
    }

public:
    std::string fname;
    double threshold;
    bool optional_flag;
    std::string log_level;
};
Options options;

int main(int argc, char** argv)
{
    if(!options.parse(argc, argv))
        return 1;

    std::cout << options.fname << std::endl;
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

    return 0;
}
