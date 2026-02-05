
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

#include <fastmatch-dataset/MetadataEntryReader.hpp>
#include <opencv2/opencv.hpp>
#include <chrono>
#include "runtime/WorkspaceRuntime.hpp"
#include "core/ParticleFilterConfig.hpp"
#include "io/ResultWriter.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

int main(int ac, char *av[]) {
    po::options_description desc("Allowed options");
    desc.add_options()
            ("map-image,m", po::value<std::string>(), "Path to map image")
            ("dataset,d", po::value<std::string>(), "Path to dataset directory")
            ("results,r", po::value<std::string>()->default_value("results"), "Result directory name directory")
            ("skip-rate,s", po::value<uint32_t>()->default_value(10), "Skip number of dataset entries each iteration")
            ("write-images,w", "Write preview images to disk")
            ("affine-matching,a", "Perform affine image matching when evaluating particles")
            ("preview,p", "Display preivew image using imshow")
            ("correlation-bound,c", po::value<float>()->default_value(0.2f), "Correlation activation bound")
            ("conversion-method,M", po::value<std::string>()->default_value("glf"), "Correlation to probability conversion "
                                                                                       "function: hprelu or glf")
            ("write-histograms,H", "Write correlation histograms to a separate CSV file")
            ("particle-radius", po::value<double>()->default_value(500.0), "Particle filter radius")
            ("epsilon", po::value<float>()->default_value(0.1f), "Particle filter epsilon")
            ("particle-count", po::value<int>()->default_value(200), "Particle filter particle count")
            ("quantile", po::value<float>()->default_value(0.99f), "Particle filter quantile")
            ("kld-error", po::value<float>()->default_value(0.5f), "Particle filter KLD error")
            ("bin-size", po::value<int>()->default_value(5), "Particle filter bin size")
            ("use-gaussian", po::value<bool>()->default_value(true), "Use gaussian sampling")
            ("help,h", "produce help message");

    po::variables_map vm;
    po::store(po::parse_command_line(ac, (const char *const *) av, desc), vm);
    po::notify(vm);

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

    ParticleFilterConfig config;
    config.radius = vm["particle-radius"].as<double>();
    config.epsilon = vm["epsilon"].as<float>();
    config.particleCount = vm["particle-count"].as<int>();
    config.quantile = vm["quantile"].as<float>();
    config.kld_error = vm["kld-error"].as<float>();
    config.binSize = vm["bin-size"].as<int>();
    config.use_gaussian = vm["use-gaussian"].as<bool>();

    std::stringstream output;

    bool displayPreview = vm.count("preview") > 0;
    bool pfInitialized = false;
    // Declare reader
    MetadataEntryReader reader;
    WorkspaceRuntime pf;
    pf.setDisplayImage(displayPreview);
    std::string mapName;
    if(vm.count("map-image")) {
        auto mapImage = vm["map-image"].as<std::string>();
        mapName = fs::path(mapImage).stem().string();
        if(fs::exists(mapImage)) {
            reader.setMap(mapImage);
        } else {
            std::cerr << "Map configuration files were not found\n";
        }
        if(!reader.getMap()->isValid()) {
            std::cerr << "Map configuration is corrupted!\n";
        }
    }
    reader.setSkipRate(vm["skip-rate"].as<uint32_t>());
    // Declare path and sanity check
    fs::path datasetPath(vm["dataset"].as<std::string>());
    std::ofstream outFile;
    bool writeHistograms = false;
    if(vm.count("write-histograms")) {
        writeHistograms = true;
    }
    if(fs::exists(datasetPath / "metadata.csv")) {
        bool writeImages = (vm.count("write-images") > 0);
        char mbstr[100];
        std::string time;
        std::time_t t = std::time(nullptr);
        if (std::strftime(mbstr, sizeof(mbstr), "%Y%m%d-%H%M%S", std::localtime(&t))) {
            time = std::string(mbstr);
        }
        auto resultsDir = vm["results"].as<std::string>();
        fs::path dir = datasetPath / resultsDir / (time + (mapName.empty() ? "" : "-" + mapName));
        fs::create_directories(dir);
        outFile.open((dir / "data.csv").string());
        if(writeImages) {
            pf.setWriteImageToDisk(writeImages);
            pf.setOutputDirectory(dir.string());
        }
        if(reader.openDirectory(datasetPath.string())) {
            std::ofstream hists;
            if(writeHistograms) {
                hists.open((dir / "histograms.csv").string());
            }
            output << "\"Iteration\",\"ImageName\",";
            ResultWriter::appendHeader(output);
            output << "\n";
            // Parse line by line into the structure
            MetadataEntry entry;
            int iteration = 0;
            while (reader.readNextEntry(entry)) {
                output << iteration++ << ",\"" << entry.imageFileName << "\",";
                if(!pfInitialized) {
                    pf.initialize(entry, config);
                    if(vm["conversion-method"].as<std::string>() == "glf") {
                        pf.setConversionMethod(ParticleFastMatch::GLF);
                    } else if (vm["conversion-method"].as<std::string>() == "softmax") {
                        pf.setConversionMethod(ParticleFastMatch::Softmax);
                    }
                    pf.setCorrelationLowBound(vm["correlation-bound"].as<float>());
                    pfInitialized = true;
                    pf.describe();
                } else {
                    pf.update(entry);
                    if(writeHistograms) {
                        bool firstParticle = true;
                        for(const auto& particle : pf.getParticles()) {
                            if(firstParticle) {
                                firstParticle = false;
                            } else {
                                hists << ",";
                            }
                            hists << particle.getCorrelation();
                        }
                        hists << std::endl;
                    }
                }
                cv::Mat image = entry.getImageColored();
                if(!pf.preview(entry, image, output)) {
                    break;
                }
                if(outFile.is_open()) {
                    outFile << output.str() << std::endl;
                } else {
                    std::cout << output.str() << "\n";
                }
                // Clear the output stream, it is already dumped
                output.str("");
                output.clear();
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
