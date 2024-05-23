/*
 *  minter.c++
 *
 *  Will Miller
 *  Jun 6, 2023
 *  
 *  CLI wrapper for ffmpeg motion interpolation
 *
 */

#include <cstdlib>

#include "enhance.h"

void sys(const std::string s) {
    std::cout << s << std::endl;
    int status = std::system(s.c_str());
}

int main(int argn, char** argv) {
    std::cout << res;
    std::string file;
    double search;
    double mb;
    double fps;
    bool skip;
    bool confirm;

   	po::options_description description("Usage");

	try {
		description.add_options()
			("input,i", po::value<std::string>(&file)->required(), 
			    "The file on which to perform the motion interpolation")
			("search,s", po::value<double>(&search)->default_value(32.0), 
			    "The search parameter for ffmpeg minterpolate")
			("mb,m", po::value<double>(&mb)->default_value(16.0), 
			    "The macroblock size parameter for ffmpeg minterpolate")
			("fps,f", po::value<double>(&fps)->default_value(60.0),
			    "The output framerate")
			("skip", po::bool_switch(&skip), 
			    "Skip the preview")
			("y", po::bool_switch(&confirm),
			    "Skip confirmation")
		;
	}
	catch (...) {
		std::cout << "Error in boost program options initialization" << std::endl;
		exit(0);
	}

  	po::variables_map vm;
    try {
    	po::store(po::command_line_parser(argn, argv).options(description).run(), vm);
    	po::notify(vm);
    }
    catch (...) {
        std::cout << description << std::endl;
        exit(1);
    }

    int status;

    // Do the motion interpolation
    sys("ffmpeg -i "+file+" -y -c:v libx265 -crf 18 -vf \"minterpolate='fps="+
           std::to_string(fps)+":mi_mode=mci:mc_mode=aobmc:vsbmc=1:me_mode=bidir"+
           ":search_param="+std::to_string(search)+":mb_size="+std::to_string(mb)+
           "'\" out.mp4");

    if (!skip) {
        // Create side-by-side preview
        sys("ffmpeg -i "+file+" -i out.mp4 -y -filter_complex \"[0:v] fps="+
             std::to_string(fps)+" [A]; [1:v] fps="+std::to_string(fps)+
             " [B]; [A][B]hstack=inputs=2\" -c:v libx265 comp.mp4");
        sys(std::string("ffplay -i comp.mp4 -loop 0"));   // Show the preview
        sys("rm comp.mp4");    
    }

    if (!confirm) {
        std::cout << "Apply the changes? [y/n]: " << std::flush;
        char c;
        std::cin >> c;
        confirm = (c == 'y');
    }
    
    if (confirm) sys("mv out.mp4 "+file);
    else sys(std::string("rm out.mp4"));
}
