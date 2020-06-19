/**
 *  @file    parallelVideoGenerator.cpp
 *  @author  Biniam Abrha Nigusse
 *  @date    13/02/2020
 *
 *  @brief Fastflow based parallel program to generate videos from image sequences
 *
 */

#include <fstream>
#include <sys/inotify.h>
#include <cstdlib>
#include <sys/stat.h>

#include "frameWindows.cpp"

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + NAME_MAX + 1) )

#if !defined(HAS_CXX11_VARIADIC_TEMPLATES)
#define HAS_CXX11_VARIADIC_TEMPLATES 1
#endif
#include <ff/ff.hpp>
#include <ff/map.hpp>
#include <ff/parallel_for.hpp>
using namespace ff;

typedef  vector<int> ff_task_t;
unsigned int numFrames;

#ifdef min
#undef min //MD workaround to avoid clashing with min macro in minwindef.h
#endif
struct Task {
    Task(const std::string &name): name(name) {}
    const std::string  name;
};

struct Reader: ff_node_t<ff_task_t> {

    Reader(
            const string &inputPath,
            int numWorkers,
            int tot_frames,
            int &emitter_time,
            int &firstWindow_time
    ):
            inputPath(inputPath),
            numWorkers(numWorkers),
            tot_frames(tot_frames),
            emitter_time(emitter_time),
            firstWindow_time(firstWindow_time)
    {};

    ff_task_t *svc(ff_task_t *) {
        // set window size equal to chuck size of workers
        const int winsize = int(tot_frames/numWorkers);
        int length;
        int fd;
        int wd;
        int count = 0;
        char buffer[BUF_LEN];
        bool timeSet = false;
        bool windTimeSet = false;
        auto start = std::chrono::high_resolution_clock::now();
        // const int tot_frames = 10;
        // stringVec v;
        vector<int> v;

        // init window
        FrameWindows window(winsize);

        fd = inotify_init();

        if ( fd < 0 ) {
            perror( "inotify_init" );
        }

        wd = inotify_add_watch( fd, inputPath.c_str(), IN_CLOSE_WRITE );

        printf(" --- Started watching folder ...\n");

        while(true) {
            length = read( fd, buffer, BUF_LEN );

            if ( length < 0 ) {
                perror( "read" );
            }
            int i = 0;
            while (i < length) {
                auto *event = (struct inotify_event *) &buffer[i];
                if (event->len) {
                    if (event->mask & IN_CLOSE_WRITE) {
                        if (event->mask & IN_ISDIR) {
                            // printf("The directory %s was created.\n", event->name);
                        } else {
                            //printf("The file %s was created.\n", event->name);
                            if( !timeSet ){
                                timeSet = true;
                                start  = std::chrono::high_resolution_clock::now();
                            }
                            if( regex_match (event->name, base_regex )) {
                                int wno = window.addframe(event->name);
                                bool complete = window.iscomplete(wno);
                                // std::cout << event->name << "\twno=" << wno << "\tcomplete=" << complete << '\n';
                                if (complete) {
                                    v = window.flush(wno);
                                   // printwin(wno, v);
                                    printf( " Window [%d]  completed.\n", wno );
                                    ff_task_t *t = new ff_task_t(v);
                                    ff_send_out(t); // sends the task t to workers

                                    if( !windTimeSet ) {
                                        windTimeSet = true;
                                        auto firstWindowElapsed = std::chrono::high_resolution_clock::now() - start;
                                        auto firstWindowElapsed_msec = std::chrono::duration_cast<std::chrono::milliseconds>(firstWindowElapsed).count();
                                        firstWindow_time = firstWindowElapsed_msec;
                                    }

                                }

                                count++;
                                // printf( "%d files created.\n", count );
                            }
                        }
                    }
                }

                i += EVENT_SIZE + event->len;

                if( count == tot_frames ) {
                    auto elapsed = std::chrono::high_resolution_clock::now() - start;
                    auto elapsed_msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                    emitter_time = elapsed_msec;

                    (void) inotify_rm_watch(fd, wd);
                    (void) close(fd);

                    printf(" --- Finished reading %d files ... \n", tot_frames);

                    return EOS;
                }
            }
        }
    }

    const string &inputPath;
    int tot_frames;
    int numWorkers;
    int &emitter_time;
    int &firstWindow_time;
};

struct Worker : ff_node_t<ff_task_t> {

    Worker(
            const string &inputFile,
            const string &outputFilename,
            int startIndex,
            int numWorker,
            int threads,
            stringVec &tmpOutputPathNames,
            map<pid_t, int> &pid2part,
            map<int, pid_t> &part2pid,
            bool re_encode,
            const string &tmpOutputDir,
            const string &finalOutputPath,
            int framerate
    ):
            inputFile(inputFile),
            outputFilename(outputFilename),
            startIndex(startIndex),
            numWorker(numWorker),
            threads(threads),
            tmpOutputPathNames(tmpOutputPathNames),
            pid2part(pid2part),
            part2pid(part2pid),
            re_encode(re_encode),
            tmpOutputDir(tmpOutputDir),
            finalOutputPath(finalOutputPath),
            framerate(framerate)


    {};

    ff_task_t *svc(ff_task_t *in) {
        ff_task_t &inImg = *in;
        int firstIndex = inImg[0];
        int lastIndex = inImg[inImg.size() - 1];
        int chunkSize = inImg.size();
        string inputParams = to_string(firstIndex);

        printf(" --- WORKER [%d] : started with frame index [%d] ...\n", startIndex, firstIndex);

        if(re_encode) {
           // printf(" --- WORKER started with frame index [%d] - enabling re-encoding ...\n", firstIndex);
            string tmpOutput= tmpOutputDir + "tmp_" + to_string(startIndex) + "_" + outputFilename + ".mov";
            tmpOutputPathNames.push_back(tmpOutput);

            imageConverterReduce(
                    inputFile,
                    tmpOutput,
                    "mp4",
                    numWorker,
                    inImg.size(),
                    false,
                    inputParams,
                    to_string(framerate),
                    chunkSize,
                    to_string(threads),
                    pid2part,
                    part2pid,
                    startIndex
            );

        }
        else
            {
            //printf(" --- WORKER started with frame index [%d] - disabling re-encoding ...\n", firstIndex);
            string tmpOutput= tmpOutputDir + to_string(startIndex) + "_" + outputFilename;
            tmpOutputPathNames.push_back(tmpOutput);

            imageConverter(
                    inputFile,
                    tmpOutput,
                    "mp4",
                    numWorker,
                    inImg.size(),
                    false,
                    inputParams,
                    to_string(framerate),
                    chunkSize,
                    to_string(threads)
            );
        }


        delete in;
        return GO_ON;


    };

    const string &inputFile;
    const string &outputFilename;
    int startIndex;
    int numWorker;
    int threads;
    stringVec &tmpOutputPathNames;
    map<pid_t, int> &pid2part;
    map<int, pid_t> &part2pid;
    bool re_encode;
    const string &tmpOutputDir;
    const string &finalOutputPath;
    int framerate;

};

/**
 *  @name parallelConverter
 *  @brief Function to manage the creation of workers and initialize the Emitter
 *  @return integer number on success
 *
 */
int parallelConverter(const string& inputPath,const string& filename,const string& outputFilename,
                        const string& output_format,int numWorker, int tot_frames, bool skip_save,
                        bool re_encode, int ffmpeg_thds, int framerate, bool hasAudio, const string& inputAudio){


    int numThreads = 1;
    int emitter_time = 0;
    int firstWindow_time = 0;

    string inputFile = inputPath + filename;
    stringVec tmpOutputPathNames;

    const string tmpOutputDir = "./tmp/";
    const string finalOutputPath = "./output/";

    // create output directory if it doesn't exist.
    mkdir(finalOutputPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    mkdir(tmpOutputDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    int FFthreads = ffmpeg_thds == 0 ? getFFThreads(numWorker): ffmpeg_thds;
    //cout<< "FFThreads " << FFthreads <<endl;

    std::map<pid_t, int> pid2part;
    std::map<int, pid_t> part2pid;

    // Init Emitter
    Reader read( inputPath, numWorker, tot_frames, emitter_time, firstWindow_time );

    ffTime(START_TIME);

    std::vector<std::unique_ptr<ff_node> > Workers;
    for(int startIndex=0;startIndex<numWorker;++startIndex) {
        Workers.push_back(make_unique<Worker>(
                inputFile,
                outputFilename,
                startIndex,
                numWorker,
                FFthreads,
                tmpOutputPathNames,
                pid2part,
                part2pid,
                re_encode,
                tmpOutputDir,
                finalOutputPath,
                framerate
            )
        );
    }

    ff_Farm<long> farm(std::move(Workers),read);
    farm.remove_collector();


    if (farm.run_and_wait_end()<0) {
        error("Running farm ");
        return -1;
    }

    // start Collector
    if(!re_encode) {

        // wait for all child processes to finish
        waitChildProcs(numWorker);
        //printf("now init concat\n");

        // Set tmp output path
        string outputPath = tmpOutputDir + outputFilename;

        // If no audio set as final output path
        if(!hasAudio) {
             outputPath = finalOutputPath + outputFilename;
        }

        // Concatenate videos
        mergeVideos(
                outputFilename,
                tmpOutputPathNames,
                outputPath
        );

        if(hasAudio) {
            // Mux audio file
            addAudio(
                    inputAudio,
                    outputPath,
                    finalOutputPath,
                    outputFilename,
                    numWorker
            );
        }
    }

    // IF re-encoding enabled
    if(re_encode) {
        string tmpOutPutPath = waitChildProcsReduce(
                                    pid2part,
                                    part2pid,
                                    numWorker,
                                    to_string(FFthreads),
                                    outputFilename,
                                    tmpOutputDir,
                                    finalOutputPath
                                );
       // cout << "Reduce output: " << tmpOutPutPath << endl;

            // Mux audio file
            addAudio(
                    inputAudio,
                    tmpOutPutPath,
                    finalOutputPath,
                    outputFilename,
                    numWorker
            );
    }

    ffTime(STOP_TIME);
    printf(" --- Converter completed!\n");
    cout << " ****** First window: " << (tot_frames/numWorker)  <<" frames waiting time (ms): " << firstWindow_time << "\n";
    cout << " ****** Total waiting time for " << tot_frames << " frames(ms): " << emitter_time << "\n";
    cout << " ****** Time spent by " << numWorker << " WORKERS (ms): " << (ffTime(GET_TIME) - emitter_time) << "\n";
    cout << " ****** Program COMPLETION TIME (ms): " << (ffTime(GET_TIME)) << "\n";

    // Clear tmp dir
    deleteDir(tmpOutputDir);


    return 0;
}