#include <yarp/os/all.h>
#include <opencv2/opencv.hpp>
#include <event-driven/core.h>
#include <event-driven/algs.h>

//using yarp::os::ResourceFinder;
namespace fs = std::filesystem;
using yarp::os::Value;



void helpfunction() 
{
    yInfo() << "USAGE:";
    yInfo() << "--file <string> logfile path";
    yInfo() << "--height <int> video height [720]";
    yInfo() << "--width <int> video width [1280]";
    yInfo() << "--block_size <int> EROS and HARRIS block size [7]";
}

class corner_detector
{
private:
    
    int harris_block_size{7};
    std::thread harris_thread;
    std::thread vis_thread;

    void updateLUT()
    {
        cv::Mat blurred;
        while(running) 
        {
            eros.getSurface().convertTo(blurred, CV_8U);
            cv::GaussianBlur(blurred, blurred, cv::Size(7, 7), 0, 0);
            cv::cornerHarris(blurred, LUT, harris_block_size, 3, 0.04);
            double temp;
            cv::minMaxLoc(LUT, nullptr, &temp);
            max_corner_score = std::max(temp, max_corner_score);
        }
    }

    void visualise()
    {
        cv::Mat img, img8U, imgC3, imgF32;
        cv::Mat imgVGA = cv::Mat(cv::Size(640, 480), CV_8UC3);
        cv::Mat imgCORN = cv::Mat(imgV.size(), CV_8U);
        while(running) 
        {
            //events image
            cv::cvtColor(imgV, imgC3, cv::COLOR_GRAY2BGR);
            cv::putText(imgC3, "Events", {10, 10}, cv::FONT_HERSHEY_PLAIN, 0.5, {100, 100, 100});
            cv::resize(imgC3, imgVGA({0, 0, 320, 240}), {320, 240});
            imgV = cv::Vec3b(255, 255, 255);

            //eros image
            img8U = 255 - eros.getSurface();
            cv::cvtColor(img8U, imgC3, cv::COLOR_GRAY2BGR);
            cv::putText(imgC3, "EROS", {10, 10}, cv::FONT_HERSHEY_PLAIN, 0.5, {100, 100, 100});
            cv::resize(imgC3, imgVGA({0, 240, 320, 240}), {320, 240});

            //LUT image
            cv::threshold(LUT, imgF32, 0, 0, cv::THRESH_TOZERO);
            imgF32.convertTo(img8U, CV_8U, 10*255.0/max_corner_score);
            cv::cvtColor(img8U, imgC3, cv::COLOR_GRAY2BGR);
            cv::putText(imgC3, "LUT", {10, 10}, cv::FONT_HERSHEY_PLAIN, 0.5, {100, 100, 100});
            cv::resize(imgC3, imgVGA({320, 240, 320, 240}), {320, 240});

            //corner location image
            cv::threshold(img8U, img8U, 127, 255, cv::THRESH_BINARY_INV);
            cv::GaussianBlur(img8U, img8U, {5, 5}, -1);
            imgCORN = 0.9*imgCORN + 0.1*img8U;
            cv::cvtColor(imgCORN, imgC3, cv::COLOR_GRAY2BGR);
            cv::putText(imgC3, "Corner Locations", {10, 10}, cv::FONT_HERSHEY_PLAIN, 0.5, {100, 100, 100});
            cv::resize(imgC3, imgVGA({320, 0, 320, 240}), {320, 240});


            cv::imshow("luvHarris", imgVGA);
            if(cv::waitKey(1) == '\e')
                running = false;
        }

    }

public:

    cv::Mat imgV;
    cv::Mat LUT;
    ev::EROS eros;
    double max_corner_score;
    
    bool running{false};

    void stop()
    {
        running = false;
        harris_thread.join();
        vis_thread.join();
    }

    void initialise(int height, int width, int harris_block_size)
    {
        if (harris_block_size % 2 == 0)
            harris_block_size += 1;
        this->harris_block_size = harris_block_size;
        eros.init(width, height, harris_block_size, 0.5);
        LUT = cv::Mat(height, width, CV_32F, 0.0);
        imgV = cv::Mat(height, width, CV_8U, 255);
        max_corner_score = 0.0;
        running = true;
        harris_thread = std::thread([this]{updateLUT();});
        vis_thread = std::thread([this]{visualise();});
    }

};


int main(int argc, char* argv[])
{

    yarp::os::ResourceFinder rf;
    rf.configure(argc, argv);
    if(rf.check("help") || rf.check("h")) {
        helpfunction();
        return 0;
    }

    if(!rf.check("file")) {
        yError() << "Please provide path to .log containing events";
        helpfunction();
        return -1;
    }
    std::string file_path = rf.find("file").asString();

    cv::Size res = {rf.check("width", Value(1280)).asInt32(), 
                    rf.check("height", Value(720)).asInt32()};
    int block_size = rf.check("block_size", Value(7)).asInt32();

    ev::offlineLoader<ev::AE> loader;
    yInfo() << "Loading log file ... ";
    if(!loader.load(file_path)) {
        yError() << "Could not open log file";
        return false;
    } else {
        yInfo() << loader.getinfo();
    }

    corner_detector cd;
    cd.initialise(res.height, res.width, block_size);

    loader.synchroniseRealtimeRead(yarp::os::Time::now());
    while(loader.incrementReadTill(yarp::os::Time::now()) && cd.running) {

        for(auto &v : loader) {
            cd.eros.update(v.x, v.y);
            cd.imgV.at<unsigned char>(v.y, v.x) = 0;
            //to do event-by-event corner detection
            // double event_corner_score = cd.LUT.at<float>(v.y, v.x);
            // if(event_corner_score > cd.max_corner_score*0.05) {
            //     //output as corner
            // }
        }

    }

    cd.stop();

    return 0;

}