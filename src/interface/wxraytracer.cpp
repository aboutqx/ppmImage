#include <wx/wx.h>
#include <wx/dcbuffer.h>
#include "wxraytracer.h"
#include <algorithm>
#include <random>
#include "bg.xpm"
#include "black.xpm"

wxDEFINE_EVENT(wxEVT_THREAD, wxThreadEvent);
wxIMPLEMENT_APP(wxraytracerapp);

Scene* scene = new Scene();


/******************************************************************************/
/********************* RenderCanvas *******************************************/
/******************************************************************************/


RenderCanvas::RenderCanvas(wxWindow *parent)
   : wxScrolledWindow(parent), theImage(NULL), manager(NULL),
   timer(NULL), updateTimer(this, ID_RENDER_UPDATE)
{
   SetOwnBackgroundColour(wxColour(143,144,150));
   queue = new Queue(this);
}

RenderCanvas::~RenderCanvas(void)
{
    if (!threads.empty())
    {
        for (size_t t = 0; t < threads.size(); ++t)
            queue->AddJob(tJOB(tJOB::eID_THREAD_EXIT, wxEmptyString), Queue::eHIGHEST); // send all running threads the "EXIT" signal
    }
    if (manager != NULL)
        manager->StopRendering();


    if (queue != NULL)
    {
        if (threads.size() != 0)
            OnQuit();
        wxThread::Sleep(50);
        delete queue;
        queue = NULL;
    }

    {  // scope for CS locker
        wxCriticalSectionLocker enter(managerCS);
        if (manager != NULL)
        {
            if (manager->Delete() != wxTHREAD_NO_ERROR)
                wxLogError("Can't delete the manager thread!");
        }
        // exit from the critical section to give the thread
        // the possibility to enter its destructor
        // (which is guarded with managerCS critical section!)
    }

    while (1)
    {
        { // was the ~RenderManager() dtor executed?
            wxCriticalSectionLocker enter(managerCS);
            if (manager == NULL) break;  // if it has been cleaned up
        }

        // wait for thread completion
        wxThread::This()->Sleep(1);
    }

    if (theImage != NULL)
    {
        delete theImage;
        theImage = NULL;
    }


#if SAMPLE_HACK>0
    if (!theWorlds.empty())
    {
        for (vector<World*>::iterator iter = theWorlds.begin(); iter < theWorlds.end(); ++iter)
            if ((*iter) != NULL)
            {
                delete (*iter);
            }
        theWorlds.clear();
    }
#endif    

    if (timer != NULL)
    {
        delete timer;
        timer = NULL;
    }
}

void RenderCanvas::SetImage(wxImage& image)
{
    if (theImage != NULL)
        delete theImage;

    theImage = new wxBitmap(image);

    SetScrollbars(10, 10, (int)(theImage->GetWidth() / 10.0f),
        (int)(theImage->GetHeight() / 10.0f), 0, 0, true);

    Refresh();
}

wxImage RenderCanvas::GetImage(void)
{
    if (theImage != NULL)
        return theImage->ConvertToImage();

    return wxImage();
}

void RenderCanvas::OnDraw(wxDC& dc)
{
    if (theImage != NULL && theImage->IsOk())
        wxBufferedDC bdc(&dc, *theImage);
}

void RenderCanvas::OnRenderCompleted(wxCommandEvent& event)
{
    wxGetApp().SetStatusText(wxT("Render Completed: Cleaning up memory"));
    OnQuit();

    if (manager != NULL)
    {
        manager->Delete();
        delete manager;
        manager = NULL;
    }

#if SAMPLE_HACK>0
    if (!theWorlds.empty())
    {
        for (vector<World*>::iterator iter = theWorlds.begin(); iter < theWorlds.end(); ++iter)
            if ((*iter) != NULL)
            {
                delete (*iter);
            }
        theWorlds.clear();
    }
#endif

    if (timer != NULL)
    {
        long interval = timer->Time();

        wxTimeSpan timeElapsed(0, 0, 0, interval);
        wxString timeString = timeElapsed.Format(wxT("Elapsed Time: %H:%M:%S"));
        wxGetApp().SetStatusText(timeString, 1);

        delete timer;
        timer = NULL;
    }

    wxGetApp().SetStatusText(wxT("Render Completed"));

}

void RenderCanvas::OnNewPixel(wxCommandEvent& event)
{
    //set up double buffered device context
    wxClientDC cdc(this);
    DoPrepareDC(cdc);
    wxBufferedDC bufferedDC(&cdc, *theImage);
   
   //iterate over all pixels in the event
   std::vector<RenderPixel*> *pixelsUpdate =
                        (std::vector<RenderPixel*> *)event.GetClientData();
   
   for(std::vector<RenderPixel*>::iterator itr = pixelsUpdate->begin();
                        itr != pixelsUpdate->end(); itr++)
   {
      RenderPixel* pixel = *itr;
      
      wxPen pen(wxColour(pixel->red, pixel->green, pixel->blue));
      bufferedDC.SetPen(pen);
      bufferedDC.DrawPoint(pixel->x, pixel->y);
     
      pixelsRendered++;
      delete pixel;
      pixel = NULL;
   }
   
   pixelsUpdate->clear();
   delete pixelsUpdate;
   pixelsUpdate = NULL;
}

void RenderCanvas::renderPause(void)
{
   if(manager != NULL)
      manager->Pause();
   
   updateTimer.Stop();
   
   if(timer != NULL)
      timer->Pause();
}

void RenderCanvas::renderResume(void)
{
   if(manager != NULL)
      manager->Resume();
   
   updateTimer.Start();
   
   if(timer != NULL)
      timer->Resume();
}

void RenderCanvas::OnTimerUpdate( wxTimerEvent& event )
{
   if(timer == NULL)
      return;
   
   //percent
   float completed = (float)pixelsRendered / (float)pixelsToRender;

   wxString progressString = wxString::Format(wxT("Rendering...%d%%"),
                                             (int)(completed*100));
   wxGetApp().SetStatusText( progressString , 0);
   
   //time elapsed
   long interval = timer->Time();
   
   wxTimeSpan timeElapsed(0, 0, 0, interval);
   
   //time remaining
   float remaining = 1.0f - completed;
   long msecRemain = (long)
                       (((double)interval / (completed*100)) * 100 * remaining);
   
   wxTimeSpan timeRemaining(0, 0, 0, msecRemain);
   
   wxString timeRemainString = timeRemaining.Format(wxT(" / ETA: %H:%M:%S"));
   wxString timeString = timeElapsed.Format(wxT("Elapsed Time: %H:%M:%S"));
   
   //only display ETA if something has been completed
   if(msecRemain >= 0)
      wxGetApp().SetStatusText( timeString + timeRemainString, 1);
   else
      wxGetApp().SetStatusText( timeString, 1);

   if (completed >= 1)  // If we have reached 1 (100%) render call completed
   {
       wxCommandEvent event(wxEVT_RENDER, ID_RENDER_COMPLETED);
       this->GetEventHandler()->AddPendingEvent(event);
       this->GetParent()->GetEventHandler()->AddPendingEvent(event);
   }
}

void RenderCanvas::renderStart(void)
{
   wxGetApp().SetStatusText( wxT( "Rendering...") );
   scene->setup();

   pixelsRendered = 0;
   pixelsToRender = scene->nx * scene->ny;

   //set the background
   wxBitmap bitmap(scene->nx, scene->ny, -1);
   wxMemoryDC dc;
   dc.SelectObject(bitmap);
   dc.SetBackground(*wxGREY_BRUSH);
   dc.Clear();
   
   wxBitmap tile(black_xpm);

   for (int x = 0; x < scene->nx; x += 16)
   {
       for (int y = 0; y < scene->ny; y += 16)
           dc.DrawBitmap(tile, x, y, FALSE);
   }
   
   dc.SelectObject(wxNullBitmap);
   
   wxImage temp = bitmap.ConvertToImage();
   SetImage(temp);

   updateTimer.Start(250);

   //start timer
   timer = new wxStopWatch();
   
   manager = new RenderThread(this);
   manager->Create();
   scene->paintArea = manager;
   manager->SetPriority(100);
   manager->Run();

   divisions = 8;
   totalThreads = 2;// wxThread::GetCPUCount();
   // Assign job division sizes
   unsigned int jobWidth = 0;
   unsigned int jobHeight = 0;

   if ((unsigned)scene->nx < divisions)
       jobWidth = (unsigned)scene->nx;
   else
       jobWidth = (unsigned)scene->nx / divisions;
   if ((unsigned)scene->ny < divisions)
       jobHeight = (unsigned)scene->ny;
   else
       jobHeight = (unsigned)scene->ny / divisions;

   float xRemainder = (unsigned)scene->nx % divisions;
   float yRemainder = (unsigned)scene->ny % divisions;

   std::vector<Pixel> toRender;
   toRender.reserve(pixelsToRender);
   for (unsigned int y = 0; y < (unsigned)scene->ny; y++)
   {
       for (unsigned int x = 0; x < (unsigned)scene->nx; x++)
       {
           toRender.push_back(Pixel(x, y));
       }
   }
   std::default_random_engine generator{ std::random_device{}() };
   std::shuffle(toRender.begin(), toRender.end(), generator);

   unsigned int totalQ = jobWidth * jobHeight;

   unsigned int id = 0;
   unsigned int iJob = 0;

   std::vector<Pixel> current;
   current.reserve(totalQ);
   for (unsigned int y = 0; y < divisions; y++)
   {
       for (unsigned int x = 0; x < divisions; x++)
       {
           if (id < toRender.size())
           {
               for (unsigned int i = 0; i < totalQ; i++)
               {
                   if (id >= toRender.size())
                       break;
                   current.push_back(toRender[id]);
                   id++;
               }
               iJob++;
               queue->AddJob(tJOB(tJOB::eID_THREAD_JOB, wxString::Format(wxT("%u"), iJob), current));
               current.clear();
           }
       }

   }
   if (id < pixelsToRender)
   {
       while (id < pixelsToRender)
       {
           for (unsigned int i = 0; i < totalQ; i++)
           {
               current.push_back(toRender[id]);
               id++;
               if (id == pixelsToRender)
                   i = totalQ;
           }
           iJob++;
           queue->AddJob(tJOB(tJOB::eID_THREAD_JOB, wxString::Format(wxT("%u"), iJob), current));
           current.clear();
       }
   }
   toRender.clear();

   threadNumber = totalThreads;
   for (unsigned int i = 0; i < threadNumber; i++)
   {
#if SAMPLE_HACK>0
       World* wo = new World();
       wo->build();
       wo->paintArea = manager;
       theWorlds.push_back(wo);
#endif
       StartOneWorker();
   }
   // add CriticalSelections for the threads
   for (unsigned int i = 0; i < theThreads.size(); i++)
   {
       threadsCS.push_back(new wxCriticalSection());
   }

   try
   {
       if (theThreads.size() > 2)
           bool concurrent = wxThread::SetConcurrency(theThreads.size() + 2); // 1 for manager 1 for frame

   }
   catch (...)
   {
       wxMessageBox(wxT("RenderStart() could not be completed - Try and comment out wxThread::SetConcurrency in RenderCanvas::renderStart"));
   }

   wxGetApp().SetStatusText(wxT("Rendering..."));

}

void RenderCanvas::StartOneWorker() // start one worker thread
{
    int id = threads.empty() ? 1 : threads.back() + 1;
    threads.push_back(id);

#if SAMPLE_HACK>0
    WorkerThread* thread = new WorkerThread(queue, this, theWorlds.back(), id); // create a new worker thread, increment thread counter (this implies, thread will start OK)
#else
    WorkerThread* thread = new WorkerThread(queue, this, id);
#endif

    theThreads.push_back(thread);
    // totalCPU cores 100% minus manager priority and the frames priority (100) divided by how many threads being used
    int priority = (totalThreads * 100 - manager->GetPriority() - 100) / this->threadNumber;
    priority = max(min((priority), 100), 5); // make sure not above 100, or less than 5
    thread->SetPriority(priority);
    if (thread->Run() != wxTHREAD_NO_ERROR)
    {
        wxLogError((wxChar*)"Can't create the thread!");
        delete theThreads.back();
        threads.back() = NULL;
        threads.pop_back();
    }
}

void RenderCanvas::OnThread(wxCommandEvent& event) // handler for thread notifications
{
    switch (event.GetId())
    {
    case tJOB::eID_THREAD_JOB:
        //SetStatusText(wxString::Format(wxT("[%i]: %s"), event.GetInt(), event.GetString().c_str())); // progress display
        break;
    case tJOB::eID_THREAD_EXIT:
        //SetStatusText(wxString::Format(wxT("[%i]: Stopped."), event.GetInt()));
        threads.remove(event.GetInt()); // thread has exited: remove thread ID from list
        break;
    case tJOB::eID_THREAD_STARTED:
        wxGetApp().SetStatusText(wxString::Format(wxT("[%i]: Ready."), event.GetInt()));
        break;
    default: event.Skip();
    }
}

void RenderCanvas::OnQuit()
{

    if (!threads.empty())
    {
        for (size_t t = 0; t < threads.size(); ++t)
            queue->AddJob(tJOB(tJOB::eID_THREAD_EXIT, wxEmptyString), Queue::eHIGHEST); // send all running threads the "EXIT" signal
    }
    wxThread::This()->Sleep(10);

    if (!theThreads.empty())
    {
        if (theThreads.size() == 1)
        {
            theThreads.pop_back();
            theThreads.clear();
        }
        else
        {
            unsigned int id = 0;
            for (iter = theThreads.begin(); iter < theThreads.end(); iter++, id++)
            {
                {   wxCriticalSectionLocker enter(*threadsCS[id]);
                if ((*iter) != NULL)
                {
                    if ((*iter)->Delete() != wxTHREAD_NO_ERROR)
                        wxLogError((wxChar*)"Can't delete the thread!");
                }
                }
            }
            while (1)
            {
                {   // was the ~WorkerThread() function executed?
                    unsigned int id = 0;
                    unsigned int cleaned = 0;
                    for (iter = theThreads.begin(); iter < theThreads.end(); iter++, id++)
                    {
                        {
                            wxCriticalSectionLocker enter(*threadsCS[id]);
                            if ((*iter) == NULL)
                            {
                                cleaned++;
                            }
                        }
                    }
                    if (cleaned == theThreads.size())
                        break;
                }

                // wait for thread completion
                wxThread::This()->Sleep(1);
            }

            theThreads.clear();

            // Cleanup the CriticalSelections that were dynamically allocated
            for (unsigned int i = 0; i < threadsCS.size(); i++)
            {
                if (threadsCS[i] != NULL)
                {
                    delete threadsCS[i];
                    threadsCS[i] = NULL;
                }
            }
            threadsCS.clear();
        }
    }

}
/******************************************************************************/
/********************* RenderPixel ********************************************/
/******************************************************************************/


RenderPixel::RenderPixel(unsigned int _x, unsigned int _y, unsigned int _red, unsigned int _green, unsigned int _blue)
    : x(*new int(_x)), y(*new int(_y)), red(*new int(_red)), green(*new int(_green)), blue(*new int(_blue))
{}


/******************************************************************************/
/********************* RenderThread *******************************************/
/******************************************************************************/

DEFINE_EVENT_TYPE(wxEVT_RENDER)

BEGIN_EVENT_TABLE( RenderCanvas, wxScrolledWindow )
   EVT_COMMAND(wxID_ANY, wxEVT_THREAD, RenderCanvas::OnThread)
   EVT_COMMAND(ID_RENDER_NEWPIXEL, wxEVT_RENDER,
                     RenderCanvas::OnNewPixel)
   EVT_COMMAND(ID_RENDER_COMPLETED, wxEVT_RENDER,
                     RenderCanvas::OnRenderCompleted)
   EVT_TIMER(ID_RENDER_UPDATE, RenderCanvas::OnTimerUpdate)
END_EVENT_TABLE()

void RenderThread::setPixel(int x, int y, int red, int green, int blue)
{
   wxCriticalSectionLocker locker(critical);
   pixels->push_back(new RenderPixel(x, y, red, green, blue));

   if(timer->Time() - lastUpdateTime > 40)
      NotifyCanvas();
    
   TestDestroy();
}

void RenderThread::setPixel(const vector<RenderPixel*>& re)
{
    wxCriticalSectionLocker locker(critical);
    vector<RenderPixel*>::const_iterator it;
    pixels->reserve(pixels->size() + re.size());
    for (it = re.begin(); it != re.end(); it++)
    {
        pixels->push_back(*it);

    }
    return;
}

void RenderThread::NotifyCanvas()
{
   wxCriticalSectionLocker locker(critical);
   lastUpdateTime = timer->Time();
   
   //copy rendered pixels into a new vector and reset
   //std::vector<RenderPixel*> *pixelsUpdate = new std::vector<RenderPixel*>(pixels);
   //pixels.clear();
   
   wxCommandEvent event(wxEVT_RENDER, ID_RENDER_NEWPIXEL);
   event.SetClientData(pixels);
   canvas->GetEventHandler()->AddPendingEvent(event);

   pixels = new vector<RenderPixel*>();
   return;
}

void RenderThread::OnExit()
{
   NotifyCanvas();
   
   wxCommandEvent event(wxEVT_RENDER, ID_RENDER_COMPLETED);
   
   canvas->GetEventHandler()->AddPendingEvent(event);
   
   canvas->GetParent()->GetEventHandler()->AddPendingEvent(event);
}

void *RenderThread::Entry()
{
   lastUpdateTime = 0;
   timer = new wxStopWatch();
   pixels = new vector<RenderPixel*>();

   while (!TestDestroy())
   {
       NotifyCanvas();	     // Send pixels to canvas
       wxThread::Sleep(50); // Sleep for 50 milliseconds		
   }

   return NULL;
}

void RenderThread::StopRendering()
{
    stop = true;
}

RenderThread::~RenderThread()
{
    wxCriticalSectionLocker enter(canvas->managerCS);

    // the thread is being destroyed; make sure not to leave dangling pointers around
    canvas->manager = NULL;
}


/******************************************************************************/
/********************* WorkerThread *******************************************/
/******************************************************************************/
WorkerThread::~WorkerThread()
{
    wxCriticalSectionLocker enter(*canvas->threadsCS[id - 1]);
    // the thread is being destroyed; make sure not to leave dangling pointers around
    canvas->theThreads[id - 1] = NULL;
}

unsigned int WorkerThread::getID() const
{
    return id;
}

wxThread::ExitCode WorkerThread::Entry()
{
    tJOB::tCOMMANDS iErr;
    queue->Report(tJOB::eID_THREAD_STARTED, wxEmptyString, id); // tell main thread that worker thread has successfully started
    iErr = OnJob();  // this is the main loop: process jobs until a job handler throws
    //catch(tJOB::tCOMMANDS& i) 
   // { 
    queue->Report(iErr, wxEmptyString, id); // report from error condition
    if (iErr == tJOB::eID_THREAD_EXIT)
        iErr = tJOB::eID_THREAD_EXIT_SUCESS;
    return (wxThread::ExitCode)iErr; // and return exit code
} // virtual wxThread::ExitCode Entry()

tJOB::tCOMMANDS WorkerThread::OnJob()
{
    while (!TestDestroy()) //while this thread is not asked to be destroyed
    {
        tJOB job = queue->Pop(); // pop a job from the queue. this will block the worker thread if queue is empty
        switch (job.cmd)
        {
        case tJOB::eID_THREAD_EXIT: // thread should exit	
            queue->Report(tJOB::eID_THREAD_JOB, wxString::Format(wxT("Ending #%s Thread."), job.arg.c_str()), id);
            //Sleep(1000); // wait a while
            return tJOB::eID_THREAD_EXIT; // confirm exit command
        case tJOB::eID_THREAD_JOB: // process a standard job	
            /*if (scene->cam != NULL)
                scene->render(*world, job.theJobs); // Orthographic render:
            else*/
            scene->render(job.theJobs);

            queue->Report(tJOB::eID_THREAD_JOB, wxString::Format(wxT("Job #%s done."), job.arg.c_str()), id); // report successful completion
            break;
        case tJOB::eID_THREAD_JOBERR: // process a job that terminates with an error
            queue->Report(tJOB::eID_THREAD_JOB, wxString::Format(wxT("Job #%s errorneous."), job.arg.c_str()), id);
            //Sleep(1000);
            return tJOB::eID_THREAD_JOBERR; // report exit of worker thread
            break;
        case tJOB::eID_THREAD_NULL: // dummy command
        default: break; // default
        } // switch(job.cmd)
    }
    return tJOB::eID_THREAD_EXIT;
} // virtual void OnJob()
