
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdexcept>

#include <fastmatch-dataset/MetadataEntryReader.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv_modules.hpp>
#include <chrono>
#if defined(HAVE_OPENCV_HIGHGUI)
#include "runtime/WorkspaceRuntime.hpp"
#endif
#include "core/ParticleFilterConfig.hpp"
#include "core/ParticleFilterCore.hpp"
#include "io/ResultWriter.hpp"
#include "models/MotionModelSvo.hpp"
#include "models/ScaleModel.hpp"

namespace fs = boost::filesystem;
namespace po = boost::program_options;

namespace {
class HeadlessRuntime {
public:
    void initialize(const MetadataEntry &metadata, const ParticleFilterConfig &config) {
        std::cout << "Initializing...";
        std::cout.flush();
        direction = metadata.imuOrientation.toRPY().getZ();
        svoCurPosition = metadata.mapLocation;
        svoCoordinates = std::make_shared<GeographicLib::LocalCartesian>(
                metadata.latitude,
                metadata.longitude,
                metadata.altitude
        );
        core.initialize(metadata, config);
        core.setDirection(direction);
        cv::Mat templ = metadata.getImageColored();
        currentScale = scaleModel.updateScale(
                1.0f,
                static_cast<float>(metadata.altitude),
                templ.cols,
                [this](float minScale, float maxScale) {
                    core.setScale(minScale, maxScale);
                }
        );
        startLocation = core.getFilter()->getPredictedLocation();
        std::cout << " done!" << std::endl;
    }

    void update(const MetadataEntry &metadata) {
        cv::Point movement = motionModel.getMovementFromSvo(metadata, svoCoordinates, direction, svoCurPosition);
        currentScale = scaleModel.updateScale(
                1.0f,
                static_cast<float>(metadata.altitude),
                640,
                [this](float minScale, float maxScale) {
                    core.setScale(minScale, maxScale);
                }
        );
        direction = metadata.imuOrientation.toRPY().getZ();
        core.setDirection(direction);
        cv::Mat templ = metadata.getImageColored();
        core.setTemplate(templ);
        if(!affineMatching) {
            core.filterParticles(movement, bestTransform);
        } else {
#ifdef USE_CV_GPU
            core.filterParticlesAffine(movement, bestTransform);
#else
            throw std::runtime_error("Affine particle matching is available with GPU support only");
#endif
        }
    }

    bool preview(const MetadataEntry &metadata, const cv::Mat & /*image*/, std::stringstream &stringOutput) {
        if(writeImageToDisk && !warnedWriteImages) {
            warnedWriteImages = true;
            std::cerr << "Headless mode ignores --write-images because GUI preview rendering is disabled.\n";
        }
        cv::Point2i prediction = core.getFilter()->getPredictedLocation();
        cv::Point2i relativeLocation = prediction - startLocation;
        double distance = std::sqrt(std::pow(metadata.mapLocation.x - prediction.x, 2) +
                                    std::pow(metadata.mapLocation.y - prediction.y, 2));
        double svoDistance = std::sqrt(std::pow(metadata.mapLocation.x - svoCurPosition.x, 2) +
                                       std::pow(metadata.mapLocation.y - svoCurPosition.y, 2));
        ResultWriter::appendRow(
                stringOutput,
                core.getFilter()->particleCount(),
                relativeLocation,
                distance,
                svoDistance
        );
        return true;
    }

    bool isAffineMatching() const {
        return affineMatching;
    }

    void setAffineMatching(bool affineMatching) {
        HeadlessRuntime::affineMatching = affineMatching;
    }

    bool isDisplayImage() const {
        return false;
    }

    void setDisplayImage(bool /*displayImage*/) {}

    void setWriteImageToDisk(bool writeImageToDisk) {
        HeadlessRuntime::writeImageToDisk = writeImageToDisk;
    }

    void setOutputDirectory(const std::string &outputDirectory) {
        HeadlessRuntime::outputDirectory = outputDirectory;
    }

    void setCorrelationLowBound(float bound) {
        core.setLowBound(bound);
    }

    void setConversionMethod(ParticleFastMatch::ConversionMode method) {
        core.setConversionMethod(method);
    }

    void describe() const {
        core.describe();
    }

    const Particles &getParticles() const {
        return core.getParticles();
    }

private:
    bool affineMatching = false;
    ParticleFilterCore core;
    cv::Point svoCurPosition;
    double direction = 0.0;
    cv::Point startLocation;
    cv::Mat bestTransform;
    float currentScale = 0.0f;
    std::shared_ptr<GeographicLib::LocalCartesian> svoCoordinates;
    MotionModelSvo motionModel;
    ScaleModel scaleModel;
    bool writeImageToDisk = false;
    bool warnedWriteImages = false;
    std::string outputDirectory;
};

template <typename Runtime>
int runDataset(Runtime &pf,
               MetadataEntryReader &reader,
               const po::variables_map &vm,
               const ParticleFilterConfig &config,
               const std::string &mapName,
               bool displayPreview,
               bool writeImages,
               bool writeHistograms) {
    std::stringstream output;
    bool pfInitialized = false;
    pf.setDisplayImage(displayPreview);
    fs::path datasetPath(vm["dataset"].as<std::string>());
    std::ofstream outFile;
    if(fs::exists(datasetPath / "metadata.csv")) {
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
                output.str("");
                output.clear();
            }
        } else {
            std::cerr << "Failed to open metadata file in the dataset\n";
        }
    } else {
        std::cerr << "Dataset directory does not contain metadata.csv file!\n";
        return 2;
    }

    return 0;
}
} // namespace

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
            ("no-gui", "Run without GUI preview rendering (no OpenCV HighGUI dependency)")
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

    bool displayPreview = vm.count("preview") > 0;
    bool noGui = vm.count("no-gui") > 0;
#if !defined(HAVE_OPENCV_HIGHGUI)
    if(displayPreview) {
        std::cerr << "Preview disabled: OpenCV HighGUI is not available in this build.\n";
    }
    displayPreview = false;
    noGui = true;
#endif
    if(noGui) {
        displayPreview = false;
    }
    // Declare reader
    MetadataEntryReader reader;
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
    bool writeHistograms = vm.count("write-histograms") > 0;
    bool writeImages = vm.count("write-images") > 0;
    if(noGui) {
        HeadlessRuntime pf;
        return runDataset(pf, reader, vm, config, mapName, displayPreview, writeImages, writeHistograms);
    }
#if defined(HAVE_OPENCV_HIGHGUI)
    WorkspaceRuntime pf;
    return runDataset(pf, reader, vm, config, mapName, displayPreview, writeImages, writeHistograms);
#else
    std::cerr << "GUI preview is not available in this build. Use --no-gui to run headless.\n";
    return 1;
#endif
}
