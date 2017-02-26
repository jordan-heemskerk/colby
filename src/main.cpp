#include <boost/program_options.hpp>
#include <colby/optional.hpp>
#include <colby/sp3000_color_by_numbers.hpp>
#include <colby/sp3000_color_by_numbers_observer.hpp>
#include <colby/timer.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

class observer : public colby::sp3000_color_by_numbers_observer {
private:
	bool show_;
	colby::timer t_;
	std::size_t n_merge_;
	std::size_t flood_fill_;
	std::size_t small_cells_;
	std::vector<std::thread> ts_;
	void show (const std::string & name, const base_event & e, std::size_t num = 0) {
		if (!show_) return;
		ts_.emplace_back([name,mat = e.image(),num] () {
			std::ostringstream ss;
			ss << "Sp3000 Color by Numbers - " << name;
			if (num != 0) ss << " #" << num;
			cv::imshow(ss.str(),mat);
			cv::waitKey(0);
		});
	}
	void print (const char * str) {
		std::cout << str << " (" << elapsed_string() << ')' << std::endl;
		t_.restart();
	}
public:
	explicit observer (bool show = false)
		:	show_(show),
			n_merge_(0),
			flood_fill_(0),
			small_cells_(0)
	{	}
	~observer () noexcept {
		for (auto && t : ts_) t.join();
	}
	colby::timer::duration elapsed () const {
		return t_.elapsed();
	}
	std::string elapsed_string () const {
		std::ostringstream ss;
		ss << std::chrono::duration_cast<
			std::chrono::milliseconds
		>(elapsed()).count() << " ms";
		return ss.str();
	}
	virtual void flood_fill (flood_fill_event e) override {
		show("Flood Fill",e,++flood_fill_);
		print("Flood fill complete!");
	}
	virtual void merge_small_cells (merge_small_cells_event e) override {
		show("Merge Small Cells",e,++small_cells_);
		print("Merge small cells complete!");
	}
	virtual void merge_similar_cells (merge_similar_cells_event e) override {
		show("Merge Similar Cells",e);
		print("Merge similar cells complete!");
	}
	virtual void n_merge (n_merge_event e) override {
		show("N-Merge",e,++n_merge_);
		print("N-merge complete!");
	}
	virtual void p_merge (p_merge_event e) override {
		show("P-Merge",e);
		print("P-merge complete!");
	}
	virtual void gaussian_smooth (gaussian_smooth_event e) override {
		show("Gaussian Smooth",e);
		print("Gaussian smooth complete!");
	}
	virtual void laplacian (laplacian_event e) override {
		show("Laplacian",e);
		print("Laplacian complete!");
	}
};

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
	observer o(true);
	colby::sp3000_color_by_numbers impl(o,250,10);
	colby::timer timer;
	auto result = impl.convert(mat);
	auto elapsed = timer.elapsed();
	std::cout << "Converted to color by numbers (took "
		<< std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << " ms).\n"
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
