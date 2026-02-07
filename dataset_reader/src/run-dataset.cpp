
#include <boost/program_options.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <fstream>
#include <boost/token_functions.hpp>
#include <boost/tokenizer.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgcodecs.hpp>
#include "fastmatch-dataset/MetadataEntryReader.hpp"

namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int ac, char *av[]) {
    // Console application argument configuration
    po::options_description desc("Allowed options");
    desc.add_options()
            ("map-image,m", po::value<std::string>(), "Path to map image")
            ("map-description,D", po::value<std::string>(), "Path to map description text file")
            ("dataset,d", po::value<std::string>(), "Path to dataset directory")
            ("sharpen,s", "Sharpen the read images")
            ("blur,b", "Blur images, available only if sharpen is enabled")
            ("help,h", "produce help message");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, (const char *const *) av, desc), vm);
    po::notify(vm);

    //Show help
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    // If dataset was not passed as an argument, throw error and show help
    if(!vm.count("dataset")) {
        std::cerr << "Please set dataset\n";
        std::cout << desc << "\n";
        return 1;
    }

    // Declare reader
    MetadataEntryReader reader;
    if(vm.count("map-image") && vm.count("map-description")) {
        auto mapImage = vm["map-image"].as<std::string>();
        auto mapDescription = vm["map-description"].as<std::string>();
        if(fs::exists(mapImage) && fs::exists(mapDescription)) {
            reader.setMap(mapImage, mapDescription);
        } else {
            std::cerr << "Map configuration files were not found\n";
        }
        if(!reader.getMap()->isValid()) {
            std::cerr << "Map configuration is corrupted!\n";
        }
    }

    // Declare path and sanity check
    fs::path datasetPath(vm["dataset"].as<std::string>());
    if(fs::exists(datasetPath / "metadata.csv")) {
        if(reader.openDirectory(datasetPath.string())) {

            // Parse line by line into the structure
            MetadataEntry entry;
            while (reader.readNextEntry(entry)) {
                if(vm.count("sharpen")) {
                    entry.getImageSharpened(static_cast<bool>(vm.count("blur")));
                } else {
                    entry.getImage();
                }
            }
        } else {
            std::cerr << "Failed to open metadata file in the dataset\n";
        }
    } else {
        std::cerr << "Dataset directory does not contain metadata.csv file!\n";
        std::cout << desc << "\n";
        return 2;
    }
    return 0;
}
