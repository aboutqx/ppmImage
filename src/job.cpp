#include <wx/wx.h>
#include "job.h"
/******************************************************************************/
/********************* Queue *******************************************/
/******************************************************************************/

void Queue::AddJob(const tJOB& job, const tPRIORITY& priority) // push a job with given priority class onto the FIFO
{
	wxMutexLocker lock(mutexQueue); // lock the queue
	jobs.insert(std::make_pair(priority, job)); // insert the prioritized entry into the multimap
	queueCount.Post(); // new job has arrived: increment semaphore counter
} // void AddJob(const tJOB& job, const tPRIORITY& priority=eNORMAL)

tJOB Queue::Pop()
{
	tJOB element;
	queueCount.Wait(); // wait for semaphore (=queue count to become positive)
	mutexQueue.Lock(); // lock queue
	element = (jobs.begin())->second; // get the first entry from queue (higher priority classes come first)
	jobs.erase(jobs.begin()); // erase it
	mutexQueue.Unlock(); // unlock queue
	return element; // return job entry
} // tJOB Pop()

void Queue::Report(const tJOB::tCOMMANDS& cmd, const wxString& sArg, int iArg) // report back to parent
{
	wxCommandEvent evt(wxEVT_THREAD, cmd); // create command event object
	evt.SetString(sArg); // associate string with it
	evt.SetInt(iArg);
	parent->AddPendingEvent(evt); // and add it to parent's event queue
} // void Report(const tJOB::tCOMMANDS& cmd, const wxString& arg=wxEmptyString)

size_t Queue::Stacksize() // helper function to return no of pending jobs
{
	wxMutexLocker lock(mutexQueue); // lock queue until the size has been read
	return jobs.size();
}

