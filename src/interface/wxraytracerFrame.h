#pragma once
#include <wx/wx.h>

class RenderCanvas;
class wxraytracerFrame;


class wxraytracerapp : public wxApp
{
public:
	virtual bool OnInit();
	virtual int OnExit();
	virtual void SetStatusText(const wxString& text, int number = 0);

private:
	wxraytracerFrame* frame;
	DECLARE_EVENT_TABLE()
};

class wxraytracerFrame : public wxFrame
{
public:
	wxraytracerFrame(const wxPoint& pos, const wxSize& size);

	//event handlers
	void OnQuit(wxCommandEvent& event);
	void OnOpenFile(wxCommandEvent& event);
	void OnSaveFile(wxCommandEvent& event);
	void OnRenderStart(wxCommandEvent& event);
	void OnRenderCompleted(wxCommandEvent& event);
	void OnRenderPause(wxCommandEvent& event);
	void OnRenderResume(wxCommandEvent& event);

private:
	RenderCanvas* canvas; //where the rendering takes place
	wxString currentPath; //for file dialogues
	DECLARE_EVENT_TABLE()
};

//IDs for menu items
enum
{
	Menu_File_Quit = 100,
	Menu_File_Open,
	Menu_File_Save,
	Menu_Render_Start,
	Menu_Render_Pause,
	Menu_Render_Resume
};
