
#ifndef _WXRAYTRACER_H_
#define _WXRAYTRACER_H_

/**
 * Ray Tracer skeleton
 *
 * Author : Sverre Kvaale <sverre@kvaale.com>
 * Version: 0.8
 *
 */

#include <wx/wx.h>
#include <vector>
#include <algorithm>
#include "wxraytracerFrame.h"

#include "../scene.h"
#include "../job.h"


class wxraytracerFrame;
class RenderCanvas;
class RenderThread;
class RenderPixel;
class Scene;


class WorkerThread : public wxThread
{
public:
	WorkerThread(Queue* pQueue, RenderCanvas* canvas, int id = 0) : wxThread(wxTHREAD_DETACHED), queue(pQueue), canvas(canvas), id(id) { assert(pQueue); wxThread::Create(); }
	~WorkerThread();
	virtual tJOB::tCOMMANDS OnJob();
	unsigned int getID() const;

private:
	virtual wxThread::ExitCode Entry();

	Queue* queue;
	unsigned int id;
	RenderCanvas* canvas;


}; // class WorkerThread : public wxThread


class RenderCanvas : public wxScrolledWindow
{
public:
	RenderCanvas(wxWindow* parent);
	virtual ~RenderCanvas(void);

	void SetImage(wxImage& image);
	wxImage GetImage(void);

	virtual void OnDraw(wxDC& dc);
	void renderStart(void);
	void renderPause(void);
	void renderResume(void);
	void OnRenderCompleted(wxCommandEvent& event);
	void OnTimerUpdate(wxTimerEvent& event);
	void OnNewPixel(wxCommandEvent& event);
	void OnQuit();

	RenderThread* manager;
	wxCriticalSection managerCS;

	void OnThread(wxCommandEvent& event);
	void StartOneWorker();
	vector<wxCriticalSection*> threadsCS;    // protects the thread pointers
	vector<WorkerThread*> theThreads;

	unsigned int totalThreads;
	unsigned int threadNumber;
	unsigned int divisions;

protected:
	wxBitmap* theImage;
	Queue* queue;
	std::list<unsigned int> threads;
	wxStopWatch* timer;

#if SAMPLE_HACK>0
	vector<World*> theWorlds;
#endif
	vector<WorkerThread*>::iterator iter;

private:


	unsigned long pixelsRendered;
	unsigned long pixelsToRender;
	wxTimer updateTimer;

	DECLARE_EVENT_TABLE()
};


class RenderPixel
{
public:
	RenderPixel()
		: x(0), y(0), red(0), green(0), blue(0)
	{}

	RenderPixel(unsigned int x, unsigned int y, unsigned int red, unsigned int green, unsigned int blue);

public:
	unsigned int x, y;
	unsigned int red, green, blue;
};


DECLARE_EVENT_TYPE(wxEVT_RENDER, -1)
#define ID_RENDER_COMPLETED 100
#define ID_RENDER_NEWPIXEL  101
#define ID_RENDER_UPDATE    102

class RenderThread : public wxThread
{
public:
	RenderThread(RenderCanvas* c) : wxThread(), canvas(c) {}
	virtual void* Entry();
	virtual void OnExit();
	virtual void setPixel(int x, int y, int red, int green, int blue);
	virtual void setPixel(const vector<RenderPixel*>& rendered);
	~RenderThread();
	bool stop;
	void StopRendering();
private:
	void NotifyCanvas();

	RenderCanvas* canvas;

	std::vector<RenderPixel*>* pixels;
	wxStopWatch* timer;
	long lastUpdateTime;

	// wxMutex mutex;
	wxCriticalSection critical;
};

#endif
