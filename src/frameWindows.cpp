/**
 *  @file    frameWindow.cpp
 *  @author  Biniam Abrha Nigusse
 *  @date    14/02/2020
 *
 *  @brief create a window based on the number of workers and
 * update data structures based on frame index values
 *
 */

#include <map>
#include <sstream>
#include <cassert>
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + NAME_MAX + 1) )


/**
 *  @name FrameWindows
 *  @brief a class to manage window based on arrival frames and
 *  add frames to a map
 *
 *
*/
class FrameWindows {
private:
    std::map<int, int> wincount;
    //std::map<int, std::string> fno2name;
    std::map<int, int> fno2name;
    int winsize;
public:
    FrameWindows(int n) : winsize(n) {
        std::cout << __func__ << " initialized with size " << n << '\n';
        assert(winsize > 0);
    }

    /**
     *  @name addframe
     *  @brief extract index from a frame and put to a window and
     *  updates window counter
     *  @return integer window number of a frame
     *
     */
    int addframe(const std::string& name) {
        int fno = name2no(name);
       // fno2name[fno] = name;
        fno2name[fno] = fno;
        int wno = fno / winsize;
        wincount[wno]++;
        return wno;
    }

    /**
     *  @name iscomplete
     *  @brief check if window is complete
     *
     *  @return boolean value
     *
     */
    bool iscomplete(int wno) const {
        return wincount.find(wno) != wincount.end()
               && winsize == wincount.find(wno)->second;
    }

    /**
     *  @name lastwin
     *  @brief determine if last window containes less frames than chuck size
     *
     *  @return integer
     *
     */
    int lastwin() const {
        return (wincount.size() == 1) ? wincount.begin()->first : -1;
    }

    /**
     *  @name flush
     *  @brief determine the frames in a window and add then in a vector
     *
     *  @return vector of int
     *
     */
    // std::vector<std::string> flush(int wno) {
    vector<int> flush(int wno) {
        assert(wincount.find(wno) != wincount.end());
        std::vector<int> v;
        for (int i = 0; i < winsize; i++) {
            int j = i + wno * winsize;
            if (fno2name.find(j) == fno2name.end())
                break; // last window might be shorter
            v.push_back(fno2name[j]);
            fno2name.erase(j);
        }
        wincount.erase(wno);
        return v;
    }

private:
    /**
    *  @name name2no
    *  @brief extract the integer value from frames filename
    *
    *  @return integer frame index
    *
    */
    int name2no(const std::string& name) {
        std::string::size_type p = name.rfind('.');
        assert(p != std::string::npos);
        p = name.rfind('_', p);
        assert(p != std::string::npos);
        int fno;
        std::istringstream(name.c_str() + p + 1) >> fno;
        assert(fno > 0);
        return fno - 1;
    }
};

// utility to print window data
void printwin(int wno, const std::vector<int>& v) {
    //std::cout << "window no " << wno << '\t';
    for (unsigned i = 0; i < v.size(); i++)
        std::cout << ' ' << v[i];
    std::cout << '\n';
}

