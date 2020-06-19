/**
 *  @file    spawnProcess.cpp
 *  @author  Biniam Abrha Nigusse
 *  @date    10/02/2020
 *
 *  @brief different functions to spawn process and execute commands
 *
 */

#include <sys/types.h>
#include <cstdio>
#include <wait.h>

/**
 *  @name imageConverter
 *  @brief Function to spawn a process which generates video from images sequences
 *  later on to be concatenated
 *  @return integer number on success
 *
 */
int imageConverter( const string& input_filename, const string& output_filename, const string& output_format,
        int num_worker, int tot_frames, bool skip_save, const string& input_params, const string& framerate,
        int chunkSize, const string &threads ) {

    // TODO: cross-platform command
    string chunk = to_string(chunkSize);
   // char* argv[MAX_ARGS];
    pid_t child_pid;
   // int child_status;

   // parsecmd(cmd,argv);
    child_pid = fork();
    if(child_pid == 0) {
        /* This is done by the child process. */

        //execve(path, args, newenviron)
        char *newargv[21];
        for(int i=0; i < 20; i++) newargv[i] = (char *)malloc(sizeof(char) * 60); //allocate the array in memory
        strcpy(newargv[0], "ffmpeg");
        strcpy(newargv[1], "-framerate");
        strcpy(newargv[2], framerate.c_str());
        strcpy(newargv[3], "-start_number");
        strcpy(newargv[4], input_params.c_str());
        strcpy(newargv[5], "-i");
        strcpy(newargv[6], input_filename.c_str());
        strcpy(newargv[7], "-threads");
        strcpy(newargv[8], threads.c_str());// pass as param
        strcpy(newargv[9], "-frames:v");
        strcpy(newargv[10], chunk.c_str()); // pass as param
        strcpy(newargv[11], "-vcodec");
        strcpy(newargv[12], "libx264");
        strcpy(newargv[13], "-preset");
        strcpy(newargv[14], "medium");
        strcpy(newargv[15],  output_filename.c_str());
        strcpy(newargv[16], "-loglevel");
        strcpy(newargv[17], "error");
        strcpy(newargv[18], "-stats");
        strcpy(newargv[19], "-nostdin");
        newargv[20]= (char *)nullptr;

        // execute command
        execvp("ffmpeg", newargv);

        /* If execvp returns, it must have failed. */
        printf("Unknown command\n");
        exit(0);
    }

    return 0;
}


/**
 *  @name imageConverterReduce
 *  @brief Function to spawn a process which generates video from images sequences and
 *  pass data for reduce workers
 *  @return integer number on success
 *
 */
int imageConverterReduce( const string& input_filename, const string& output_filename, const string& output_format,
                    int num_worker, int tot_frames, bool skip_save, const string& input_params, const string& framerate,
                    int chunkSize, const string &threads,  map<pid_t, int> &pid2part, map<int, pid_t> &part2pid, int index ) {

    // TODO: cross-platform command
    string chunk = to_string(chunkSize);
    // char* argv[MAX_ARGS];
    pid_t child_pid;
    // int child_status;

    // parsecmd(cmd,argv);
    child_pid = fork();
    if(child_pid == 0) {
        /* This is done by the child process. */

        //execve(path, args, newenviron)
        char *newargv[21];
        for(int i=0; i < 20; i++) newargv[i] = (char *)malloc(sizeof(char) * 60); //allocate the array in memory

        strcpy(newargv[0], "ffmpeg");
        strcpy(newargv[1], "-framerate");
        strcpy(newargv[2], framerate.c_str());
        strcpy(newargv[3], "-start_number");
        strcpy(newargv[4], input_params.c_str());
        strcpy(newargv[5], "-i");
        strcpy(newargv[6], input_filename.c_str());
        strcpy(newargv[7], "-threads");
        strcpy(newargv[8], threads.c_str());// pass as param
        strcpy(newargv[9], "-frames:v");
        strcpy(newargv[10], chunk.c_str()); // pass as param
        strcpy(newargv[11], "-vcodec");
        strcpy(newargv[12], "libx264");
        strcpy(newargv[13], "-preset");
        strcpy(newargv[14], "veryslow");
        strcpy(newargv[15],  output_filename.c_str());
        strcpy(newargv[16], "-loglevel");
        strcpy(newargv[17], "error");
        strcpy(newargv[18], "-stats");
        strcpy(newargv[19], "-nostdin");
        newargv[20]= (char *)nullptr;

        // execute command
        execvp("ffmpeg", newargv);

        /* If execvp returns, it must have failed. */
        printf("Unknown command\n");
        exit(0);
    } else {
            pid2part[child_pid] = index;
            part2pid[index] = child_pid;
    }


    return 0;
}

/**
 *  @name waitChildProcs
 *  @brief Function to wait termination of child process for concatenation of results
 *  @return integer number on success
 *
 */
int  waitChildProcs(int nw){
    for ( int i = 0; i < nw; i++ ) {
        int child_status;
        pid_t pid = wait(&child_status);
        cout << "parent: finished child with pid " << pid << endl;
    }
    return 0;
}

/**
 *  @name Reduce
 *  @brief Class to determine index values of reduce workers
 *  @return  an integer index value
 *
*/
class Reduce {
    public:

        /**
        *  @name companion
        *  @brief A method to determine a campanion index to be paired with a partial workers output video index value
        *  @return an integer index value
        *
        */
        int companion(int i) const {
            if (i >= last)
                return -1;
            return (i ^ 1);
        }

        /**
        *  @name resulting
        *  @brief A method to determine a result index to store the final reduce output
        *  @return an integer index value
        *
        */
        int resulting(int i) const {
            if (i >= last)
                return -1;
            return last - (last - i - 1) / 2;
        }

        /**
        *  @name resulting
        *  @brief A constructor to determine the last index of the reduce function
        *  @return an integer index value
        *
        */
        Reduce(int n) {
            assert(n > 0);
           // assert((n & (n - 1)) == 0 && "n must be a power of 2");
            assert((n & (n - 1)) == 0 && "n must be a power of 2");
            last = (n | (n - 1)) - 1;
        }
    private:
        int last;
};



/**
 *  @name waitChildProcsReduce
 *  @brief Class to determine the index of reduce workers
 *  @return string filename of the final reduce output
 *
*/
string  waitChildProcsReduce( map<pid_t, int> &pid2part, map<int, pid_t> &part2pid, int numWorker,
        string FFthreads, const string &outputFilename, const string &tmpOutputDir, const string &finalOutputPath){
    // reduce
    pid_t pid;
    int status;
    int i,j,k;

    string tmpOutput, tmpInput_i, tmpInput_j, output;

    // init reduce
    Reduce reduce(numWorker);

    while (pid2part.size() > 0) {

         pid = waitpid((pid_t) -1, &status, 0);
        // Verify status if exited with success
        assert(pid2part.find(pid) != pid2part.end());
        i = pid2part[pid];
        cout << " +++ Reduce Worker " << i << " STARTED" <<endl;
        if(numWorker <= 1){
            output = tmpOutputDir + "tmp_" + to_string(i) + "_" + outputFilename + ".mov";
           return output;
        }
        //rename(tmp.<i>.mp4, <i>.mp4);
        part2pid[i] = (pid_t) -1;		// marked as complete
        j = reduce.companion(i);
        if (j < 0) break;	// was last concatenation
        if (part2pid[j] == (pid_t) -1) { // also companion is completed
            pid2part.erase(pid);
            pid2part.erase(part2pid[j]);
            part2pid.erase(i);
            part2pid.erase(j);
            if (i > j)
                std::swap(i, j);
            k = reduce.resulting(j);
            pid = fork();
            if (pid == 0) {

                char *args[17];
                tmpInput_i = tmpOutputDir + "tmp_" + to_string(i) + "_" + outputFilename + ".mov";
                tmpInput_j = tmpOutputDir + "tmp_" + to_string(j) + "_" + outputFilename + ".mov";
                tmpOutput = tmpOutputDir + "tmp_" + to_string(k) + "_" + outputFilename + ".mov";

                for(int i=0; i < 16; i++) {
                    args[i] = (char *)malloc(sizeof(char) * 60); //allocate the array in memory
                }

                strcpy(args[0], "ffmpeg");
                strcpy(args[1], "-i");
                strcpy(args[2], tmpInput_i.c_str()); // set input1
                strcpy(args[3], "-i");
                strcpy(args[4], tmpInput_j.c_str());  // set input2
                strcpy(args[5], "-filter_complex");
                strcpy(args[6], " [0:v] [1:v] concat=n=2:v=1 ");
                strcpy(args[7], "-c:v");
                strcpy(args[8], "libx264");
                strcpy(args[9], "-threads");
                strcpy(args[10], FFthreads.c_str()); // TODO get threads
                strcpy(args[11], tmpOutput.c_str());
                strcpy(args[12], "-loglevel");
                strcpy(args[13], "error");
                strcpy(args[14], "-stats");
                strcpy(args[15], "-nostdin");
                args[16]= (char *)nullptr;

                // execute command
                execvp("ffmpeg", args);

                /* If execvp returns, it must have failed. */
                printf("Unknown command\n");
                exit(0);

            } else {
                pid2part[pid] = k;
                part2pid[k] = pid;
                output = tmpOutputDir + "tmp_" + to_string(k) + "_" + outputFilename + ".mov";
            }

        }

    }

    return output;
}


/**
 *  @name mergeVideos
 *  @brief a function to spawn a process to concatenate the partial output of workers
 *  @return integer number on success
 *
*/
int mergeVideos( const string &filename, stringVec &tmpInputPaths, string &tmpOutputPath ) {
    auto start = std::chrono::high_resolution_clock::now();
    if(tmpInputPaths.size() == 1) {
        tmpOutputPath = tmpInputPaths[0];
        return 0;
    }
    string tmpFile = filename + ".txt";
    ofstream file (tmpFile);
   // string tmpOutputPath = outputDir + filename;
    sort(tmpInputPaths.begin(), tmpInputPaths.end());

    if( file.is_open() ) {

        for(auto & path : tmpInputPaths) {
            //cout<< "----output paths: " << path <<endl;
            file <<"file \'"<<path.c_str()<<"\'"<<"\n";
        }
        file.close();
    }

    printf(" --- concatinating parts ...\n");

    string cmd = "ffmpeg -loglevel error -f concat -safe 0 -i " + filename +  ".txt -c copy "+ tmpOutputPath;
    //cout<< cmd << endl;
    int ret = system( cmd.c_str() );

    remove(tmpFile.c_str());
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto elapsed_msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    cout << " ****** MERGE TIME (ms): " << elapsed_msec << "\n";

    return 0;
}

/**
 *  @name addAudio
 *  @brief a function to spawn a process to add audio to a video
 *  @return integer number on success
 *
*/
int  addAudio( const string &inputAudioPath, const string &inputVideoPath,
        const string &outputVideoPath ,const string &outputFilename, int par_degree) {
    auto start = std::chrono::high_resolution_clock::now();
    printf(" --- Adding audio file ...\n");

    string cmd = "ffmpeg -loglevel error -i " + inputVideoPath + " -i " + inputAudioPath +
            " -shortest -c copy -map 0:v:0 -map 1:a:0 " + " -threads " + to_string(par_degree) + " " +
            outputVideoPath + outputFilename;

    int ret = system( cmd.c_str() );
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    auto elapsed_msec    = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
    cout << " ****** Audio Time (ms): " << elapsed_msec << "\n";

    return 0;
}
