#include <boost/program_options.hpp>
#include <colby/optional.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <opencv2/imgcodecs.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

class program_options {
public:
	std::string in;
	std::string out;
};

}

static colby::optional<program_options> get_program_options (int argc, const char ** argv) {
	boost::program_options::options_description desc("Command line parameters");
	desc.add_options()
		("in",boost::program_options::value<std::string>(),"Input file")
		("out",boost::program_options::value<std::string>(),"Output file")
		("help,?","Display usage information");
	boost::program_options::variables_map vm;
	boost::program_options::store(boost::program_options::parse_command_line(argc,argv,desc),vm);
	boost::program_options::notify(vm);
	if (vm.count("help") || !(vm.count("in") && vm.count("out"))) {
		std::cout << desc << std::endl;
		return colby::nullopt;
	} 
	program_options retr;
	retr.in = vm["in"].as<std::string>();
	retr.out = vm["out"].as<std::string>();
	return retr;
}

static void main_impl (int argc, const char ** argv) {
	auto opts = get_program_options(argc,argv);
	if (!opts) return;
	std::cout << "Reading " << opts->in << "..." << std::endl;
	auto mat = cv::imread(opts->in,cv::IMREAD_COLOR|cv::IMREAD_ANYDEPTH);
	if (!mat.data) {
		std::ostringstream ss;
		ss << "Failed to read file " << opts->in;
		throw std::runtime_error(ss.str());
	}
	std::cout << "Read " << opts->in << ".\n"
		<< "Converting to color by numbers..." << std::endl;
	colby::sp3000_color_by_numbers impl(10.0f/255.0f,10);
	auto result = impl.convert(mat);
	std::cout << "Converted to color by numbers.\n"
		<< "Saving to " << opts->out << "..." << std::endl;
	if (!cv::imwrite(opts->out,result.image())) {
		std::ostringstream ss;
		ss << "Failed to write file " << opts->out;
		throw std::runtime_error(ss.str());
	}
	std::cout << "Saved to " << opts->out << '.' << std::endl;
}

int main (int argc, const char ** argv) {
	try {
		try {
			main_impl(argc,argv);
		} catch (const std::exception & ex) {
			std::cerr << "ERROR: " << ex.what() << std::endl;
			throw;
		} catch (...) {
			std::cerr << "ERROR" << std::endl;
			throw;
		}
	} catch (...) {
		return EXIT_FAILURE;
	}
}
