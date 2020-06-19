//
// Created by d7270 on 15/01/2020.
//
#include <thread>
#include <sys/types.h>
#include <sys/stat.h>

#include "spawnProcess.cpp"

int seqImgToVideoConverter( const string& input_path, const string& filename, const string& outputFilename,
        const string& output_format, int ffmpeg_thds, int tot_frames, bool skip_save, bool re_encode,
         int framerate, bool hasAudio, const string& inputAudio) {

    string inputFile = input_path + filename;
    const string tmpOutputDir = "./tmp/";
    const string finalOutputPath = "./output/";
    //string inputAudio = "audio_720.oga";
    string input_params = " -loglevel error -stats -framerate " + to_string(framerate) + " -start_number 0 ";
    string tmpOutputVideo = "tmp" + outputFilename;
    string tmpOutputPath = tmpOutputDir + tmpOutputVideo;

    // If no further processing is needed set path as final output path
    if(!hasAudio && !re_encode) {
        tmpOutputPath = finalOutputPath + outputFilename;
    }

    // Change the intermediate output video format to mov
    if(re_encode){
        tmpOutputVideo = tmpOutputVideo + ".mov";
        tmpOutputPath = tmpOutputDir + tmpOutputVideo;
    }
    // create output directory if it doesn't exist.
    mkdir(finalOutputPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(tmpOutputDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    auto start   = std::chrono::high_resolution_clock::now();
    // Loading input file paths in a stringVec
     stringVec files;
     read_directory(input_path, files, tot_frames);
     if(files.size() == 0){
         printf("No files found!\n");
         exit(0);
     }
    cout << "Processing " << files.size() << " images in SEQUENTIAL mode." << endl;

    string output_params = " -threads " + to_string(ffmpeg_thds) + " -frames:v " + to_string(files.size())  +
            " -vcodec libx264 -preset veryslow " + tmpOutputPath ;

    string command = "ffmpeg " + input_params + " -i " + inputFile + output_params; //+ ffCommand;

    cout<<"COMMAND: " << command << endl;
    cout << " --- Starting converter ..." << endl;

    // convert images to video
    int resp = system(command.c_str());

    // Mux audio file
    if(re_encode) {

        if(hasAudio) {
            tmpOutputVideo = "audio_" + tmpOutputVideo;
            addAudio(inputAudio, tmpOutputPath, tmpOutputDir, tmpOutputVideo, ffmpeg_thds);
        }
       // string cmd = "ffmpeg  -loglevel error -i " + tmpOutputDir + tmpOutputVideo + " -c:v libx264 -preset veryslow -crf 22 -c:a copy " +
        string cmd = "ffmpeg -loglevel error -i " + tmpOutputDir + tmpOutputVideo + " -c:v libx264 -preset veryslow -c:a copy " +
                " -threads " + to_string(ffmpeg_thds) + " " + finalOutputPath + outputFilename;

        cout << " --- Re-encoding video ..." << endl;
        // convert images to video
        int resp = system(cmd.c_str());

        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        std::cout << " ***** Sequential COMPLETION TIME with ReEncoding (ms): " << msec << std::endl;

    } else {
        if(hasAudio) {
            addAudio(inputAudio, tmpOutputPath, finalOutputPath, outputFilename, ffmpeg_thds);
        }
        auto elapsed = std::chrono::high_resolution_clock::now() - start;
        auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        std::cout << " ****** Sequential COMPLETION TIME (ms): " << msec << std::endl;
    }

    // Clear tmp dir
    deleteDir(tmpOutputDir);

    return 0;
}
