#include <map>
#include <sstream>
#include <cassert>

class FrameWindows {
private:
    std::map<int, int> wincount;
    //std::map<int, std::string> fno2name;
    std::map<int, int> fno2name;
    int winsize;
public:
    FrameWindows(int n) : winsize(n) {
        // std::cout << __func__ << " initialized with size " << n << '\n';
        assert(winsize > 0);
    }
    int addframe(const std::string& name) {
        int fno = name2no(name);
        // fno2name[fno] = name;
        fno2name[fno] = fno;
        int wno = fno / winsize;
        wincount[wno]++;
        return wno;
    }
    bool iscomplete(int wno) const {
        return wincount.find(wno) != wincount.end()
               && winsize == wincount.find(wno)->second;
    }
    int lastwin() const {
        return (wincount.size() == 1) ? wincount.begin()->first : -1;
    }
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

int watchFolder(string& path, const int &winsize, int &max_img) {
    int length;
    int fd;
    int wd;
    int count = 0;
    char buffer[BUF_LEN];
    // const int max_img = 10;
    stringVec v;
    // init window
    FrameWindows window(winsize);
    fd = inotify_init();
    if ( fd < 0 ) {
        perror( "inotify_init" );
    }
    wd = inotify_add_watch( fd, path.c_str(), IN_CLOSE_WRITE ); // better to wait for file close instead of IN_CREATE
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
                        if( regex_match (event->name, base_regex )) {
                            int wno = window.addframe(event->name);
                            bool complete = window.iscomplete(wno);
                            std::cout << event->name << "\twno=" << wno << "\tcomplete=" << complete << '\n';
                            if (complete) {
                                v = window.flush(wno);
                                printwin(wno, v);
                            }
                            count++;
                            // printf( "%d files created.\n", count );
                        }
                    }
                }
            }
            i += EVENT_SIZE + event->len;
            if( count == (max_img) ) {
                int lastwin = window.lastwin();
                if (lastwin >= 0) {
                    v = window.flush(lastwin);
                    printwin(lastwin, v);
                }
                (void) inotify_rm_watch(fd, wd);
                (void) close(fd);
                exit(1);
            }
        }
    }
}
