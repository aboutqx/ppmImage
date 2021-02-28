#ifndef JOB_H
#define JOB_H
#include <map>

//#include "Point2D.h"
//#include "RGBColor.h"
//
//struct RenderedPixel {
//    Point2D xy;
//    RGBColor color;
//};
using namespace std;

struct RenderedInt {
    unsigned int x, y, red, green, blue;
};

struct Pixel
{
public:
    Pixel(unsigned short xi, unsigned short yi) : x(xi), y(yi)
    { }

    unsigned short x, y;
};


enum RenderDisplay { EVERY_PIXEL, EVERY_ROW, EVERY_JOB };



class tJOB
{
public:
    enum tCOMMANDS                    // list of commands that are currently implemented
    {
        eID_THREAD_EXIT_SUCESS = 0,
        eID_THREAD_EXIT = wxID_EXIT,	  // thread should exit or wants to exit
        eID_THREAD_NULL = wxID_HIGHEST + 1, // dummy command
        eID_THREAD_STARTED,		      // worker thread has started OK
        eID_THREAD_JOB,				  // process normal job
        eID_THREAD_JOBERR				  // process errorneous job after which thread likes to exit
    }; // enum tCOMMANDS

    tJOB() : cmd(eID_THREAD_NULL) {}
    tJOB(tCOMMANDS cmd, const wxString& arg) : cmd(cmd), arg(arg) {}
    tJOB(tCOMMANDS cmd, const wxString& arg, vector<Pixel>& jobID) : cmd(cmd), arg(arg)
    {
        theJobs = jobID;
    }
    tCOMMANDS cmd; wxString arg;
    vector<Pixel> theJobs;
}; // class tJOB

class Queue
{
public:
    enum tPRIORITY { eHIGHEST, eHIGHER, eNORMAL, eBELOW_NORMAL, eLOW, eIDLE }; // priority classes

    Queue(wxEvtHandler* parent) : parent(parent) {}

    void AddJob(const tJOB& job, const tPRIORITY& priority = eLOW);			   // push a job with given priority class onto the FIFO

    tJOB Pop();

    void Report(const tJOB::tCOMMANDS& cmd, const wxString& sArg = wxEmptyString, int iArg = 0); // report back to parent  

    size_t Stacksize();

    std::multimap<tPRIORITY, tJOB> jobs; // multimap to reflect prioritization: values with lower keys come first, newer values with same key are appended


private:
    wxEvtHandler* parent;
    wxMutex mutexQueue;					 // protects queue access
    wxSemaphore queueCount;				 // semaphore count reflects number of queued jobs
};
#endif