/**
 *  @file    main.cpp
 *  @author  Biniam Abrha Nigusse
 *  @date    10/01/2020
 *
 *  @brief accept input parameters and call functions, based on the params
 *
 */

#include <iostream>
#include <cstdio>
#include <string>
#include <sys/types.h>
#include <chrono>
#include <cassert>
#include <unistd.h>
#include <fstream>

#include "./src/utils.cpp"
#include "./src/sequentialVideoGenerator.cpp"
#include "./src/paralleVideoGenerator.cpp"


using namespace std;
int main(int argc, char * argv[]) {

    if(cmdOptionExists(argv, argv+argc, "--help")){
        input_helper(argv[0]);
       // return -1;
    }
    if(argc < 7){
        input_helper(argv[0]);
        return -1;
    }

    // Default Options
    int tot_frames = 100;
    bool skip_save = false;
    bool re_encode = false;
    int ffmpeg_thds = 0;
    int par_degree = thread::hardware_concurrency();
    string output_format = "mp4";
    int framerate = 30;
    bool hasAudio = false;
    string inputAudio = "default-audio.mp3";


    if(cmdOptionExists(argv, argv+argc, "--tot_frames") )
        tot_frames =  atoi( getCmdOption(argv, argc + argv, "--tot_frames") );
    if(cmdOptionExists(argv, argv+argc, "--re_encode") )
        re_encode = true;
    if(cmdOptionExists(argv, argv+argc, "--skip_save") )
        skip_save = true;
    if(cmdOptionExists(argv, argv+argc, "--seq"))
        ffmpeg_thds = 1;
    if(cmdOptionExists(argv, argv+argc, "--par"))
        par_degree =  atoi( getCmdOption(argv, argc + argv, "--par") );
    if(cmdOptionExists(argv, argv+argc, "--ffmpeg_thds"))
        ffmpeg_thds =  atoi( getCmdOption(argv, argc + argv, "--ffmpeg_thds"));
    if(cmdOptionExists(argv, argv+argc, "--framerate"))
        framerate =  atoi( getCmdOption(argv, argc + argv, "--framerate"));
    if(cmdOptionExists(argv, argv+argc, "--audio")) {
        inputAudio = getCmdOption(argv, argc + argv, "--audio");
        hasAudio = true;
    }


    string input_path = sanitize_path(argv[1]);
    string filename = argv[2];
    string output_path = argv[3]; //sanitize_path(argv[2]);

    printf("\n ------------------------------------------------------------------ \n");
    if(cmdOptionExists(argv, argv+argc, "--seq")) {

        // start sequential program
        seqImgToVideoConverter(
                input_path,
                filename,
                output_path,
                output_format,
                ffmpeg_thds,
                tot_frames,
                skip_save,
                re_encode,
                framerate,
                hasAudio,
                inputAudio

        );

    }
    else if(cmdOptionExists(argv, argv+argc, "--par")) {

        // start parallel program
        parallelConverter(
                input_path,
                filename,
                output_path,
                output_format,
                par_degree,
                tot_frames,
                skip_save,
                re_encode,
                ffmpeg_thds,
                framerate,
                hasAudio,
                inputAudio

        );
    }

    printf(" -------------------------------------------------------------------- \n");
    return 0;
    
}