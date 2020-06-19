/**
 *  @file    utils.cpp
 *  @author  Biniam Abrha Nigusse
 *  @date    14/01/2020
 *
 *  @brief different helper functions
 *
 */

#include <vector>
#include <cstddef>
#include <math.h>
#include <string>
#include <cstring>
#include <regex>
#include <dirent.h>
#include <filesystem>
#include <cstdint>
#include <thread>
#include <iterator>


using namespace std;
namespace fs = std::filesystem;

/*
Definition of a new type stringVec that is a vector of string, used to read the directory
passed in parameter.
*/
typedef vector<string> stringVec;

regex base_regex("^(.(.*\\.png$|.*\\.jpg$|.*\\.jpeg$|.*\\.JPEG$|.*\\.JPG$|.*\\.gif$|.*\\.svg))*$");
smatch base_match;


/**
 *  @name read_directory
 *  @brief The function iterates over the input directory and checks whether the file is an image or
 *  not using a properly written regex. Then it inserts all the correct filenames into a stringVec.
 *
 *
 */
void read_directory(const string& name, stringVec& v, int tot_frames){

    int i=0;
    DIR* dirp = opendir(name.c_str());
    struct dirent * dp;

    while ((dp = readdir(dirp)) != NULL) {
        if (regex_match (dp->d_name, base_regex )) {
            i++;
            string entry(dp->d_name);
            v.push_back(dp->d_name);
           // string full_file_name = name + entry;
            //printf("Reading file %s \n",dp->d_name);
            //v.push_back(full_file_name);

        }
        if(i == tot_frames) break;
    }
    closedir(dirp);
}


/**
*  @name deleteDir
*  @brief Delete directory and its files
*
*
*/
void deleteDir(string dirName){
    fs::path dir = "./";
    std::uintmax_t n = fs::remove_all(dir / dirName);
   // std::cout << "Deleted " << n << " tmp files or directories\n";
}

/**
*  @name cmdOptionExists
*  @brief Check if input option exists
* @return boolean
*
*/
bool cmdOptionExists(char** begin, char** end, const string& option) {
    return find(begin, end, option) != end;
}

/**
*  @name getCmdOption
*  @brief Retrieve input option value
* @return array of chars
*
*/
char* getCmdOption(char ** begin, char ** end, const string & option) {
    char ** itr = find(begin, end, option);
    if (itr != end && ++itr != end) return *itr;
    return 0;
}

/**
*  @name getCmdOption
*  @brief Add auto end slash to dir names
* @return string
*
*/
string sanitize_path(const char* in) {
    string out(in);

    if( in[strlen(in)-1] != '/')
        return string(out + "/");

    return out;
}

/**
*  @name input_helper
*  @brief help menu
*
*/
void input_helper(const char* arg) {
    cerr << "Usage: " << arg << " input_path inputFileNamePattern outputFilename [--tot_frames n] [--seq | --par n] [--ffmpeg_thds n] --re_encode" << endl;
    cerr << "---------------------------------------------------------" << endl;
    cerr << "input_path:\t path to input directory where the images are located." << endl;
    cerr << "inputFileNamePattern:\t input file name with pattern and format." << endl;
    cerr << "outputfilename:\t path and name of the final output file and it's format." << endl;
    cerr << "--tot_frames:\t total number of input images to process." << endl;
    cerr << "--seq:\t\t sequential version" << endl;
    cerr << "--par:\t\t parallel version. Specify n workers." << endl;
    cerr << "--ffmpeg_thds:\t [Optional] number of threads for ffmpeg to use, defautls[seq =1 , par= auto calculate]." << endl;
    cerr << "--re_encode:\t [Optional] add this option to enable re-encoding. Better compression but much processing time." << endl;
    cerr << "--audio:\t [Optional] path and filename with it's format of the audio file." << endl;
    cerr << "--framerate:\t [Optional] output file encoding framerate." << endl;
    cerr << "--help:\t to show this help file." << endl;
    cerr << "---------------------------------------------------------" << endl;

    return;

}

/**
*  @name getFFThreads
*  @brief determin number of available threads to enable FFmpeg internal parallelism
* @return integer
*/

int getFFThreads(int nw){
    int FFthreads = 1;

    unsigned int TotThreads = thread::hardware_concurrency();
    // FFmpeg performs well until 16 threads and using more is not recommended
    FFthreads = min( int( floor(TotThreads / nw )) , 16);
    if(FFthreads <= 0) FFthreads = 1;

    return FFthreads;
}
