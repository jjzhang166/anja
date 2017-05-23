//@	{
//@	"targets":[{"name":"application.o","type":"object"}]
//@	}

#include "application.hpp"
#include "filenameselect.hpp"

using namespace Anja;

static void title_update(const Session& session,Window& win)
	{
	String title(session.titleGet());
	title.append("—Anja");
	win.title(title.begin());
	}

void Application::save_ask(ConfirmSaveDialogId id)
	{
	const char* title="";
	switch(id)
		{
		case ConfirmSaveDialogId::EXIT:
			title="Anja: Closing Anja";
			break;
		case ConfirmSaveDialogId::SESSION_LOAD:
			title="Anja: Loading another session";
			break;
		case ConfirmSaveDialogId::SESSION_RELOAD:
			title="Anja: Reloading session";
			break;
		case ConfirmSaveDialogId::SESSION_NEW:
			title="Anja: Creating a new session";
			break;
		}
	String msg("Do you want to save changes to ");
	msg.append(m_session.titleGet()).append("?");
	m_confirm.reset(new Dialog<Message,ConfirmSaveDialog>(m_mainwin,title,msg.begin()
		,Message::Type::WARNING));
	m_confirm->callback(*this,id);
	}

void Application::dismiss(Dialog<Message,ConfirmSaveDialog>& dlg,ConfirmSaveDialogId id)
	{
	m_confirm.reset();
	}

void Application::confirmPositive(Dialog<Message,ConfirmSaveDialog>& dlg,ConfirmSaveDialogId id)
	{
	if(sessionSave())
		{confirmNegative(dlg,id);}
	}

void Application::confirmNegative(Dialog<Message,ConfirmSaveDialog>& dlg,ConfirmSaveDialogId id)
	{
	m_confirm.reset();
	switch(id)
		{
		case ConfirmSaveDialogId::EXIT:
			m_ctx.exit();
			break;
		case ConfirmSaveDialogId::SESSION_LOAD:
			sessionLoad();
			break;
		case ConfirmSaveDialogId::SESSION_RELOAD:
			sessionLoad(m_session.filenameGet().begin());
			break;
		case ConfirmSaveDialogId::SESSION_NEW:
			m_session.clear();
			m_session_editor.sessionUpdated();
			title_update(m_session,m_mainwin);
			break;
		}
	}

bool Application::sessionSave()
	{
	if(m_session.filenameGet().length()!=0)
		{
		m_session.save(m_session.filenameGet().begin());
		return 1;
		}
	return sessionSaveAs();
	}

bool Application::sessionSaveAs()
	{
	auto name=std::string(m_session.filenameGet().begin());
	if(filenameSelect(m_mainwin,m_session.directoryGet().begin()
		,name,Anja::FilenameSelectMode::SAVE))
		{
		m_session.save(name.c_str());
		return 1;
		}
	return 0;
	}

Application& Application::sessionLoad()
	{
	auto name=std::string(m_session.filenameGet().begin());
	if(filenameSelect(m_mainwin,m_session.directoryGet().begin()
		,name,Anja::FilenameSelectMode::OPEN,[this](const char* name)
			{return m_session.loadPossible(name);},"Anja session files"))
		{sessionLoad(name.c_str());}
	return *this;
	}



void Application::closing(Window& win,int id)
	{
	save_ask(ConfirmSaveDialogId::EXIT);
	}

UiContext::RunStatus Application::idle(UiContext& ctx)
	{
	return UiContext::RunStatus::WAIT;
	}

void Application::keyDown(Anja::Window& win,int scancode,Anja::keymask_t keymask,int id)
	{
	printf("%d down\n",scancode);
	}

void Application::keyUp(Anja::Window& win,int scancode,Anja::keymask_t keymask,int id)
	{
	printf("%d up\n",scancode);
	}


void Application::clicked(ButtonList& buttons,int id,Button& btn)
	{
	switch(btn.id())
		{
		case 0:
			save_ask(ConfirmSaveDialogId::SESSION_NEW);
			break;
		case 1:
			save_ask(ConfirmSaveDialogId::SESSION_LOAD);
			break;
		case 2:
			save_ask(ConfirmSaveDialogId::SESSION_RELOAD);
			break;
		case 3:
			sessionSave();
			break;
		case 4:
			sessionSaveAs();
			break;
		case 5:
			//TODO: Engine (re)start
			break;
		case 6:
			//TODO: Engine stop
			break;
		case 7:
			m_fullscreen=!m_fullscreen;
			m_mainwin.fullscreen(m_fullscreen);
			btn.label(m_fullscreen?"Windowed":"Fullscreen");
			break;
		case 8:
			save_ask(ConfirmSaveDialogId::EXIT);
			break;
		case 9:
			//TODO: About
			break;
		}
	btn.state(0);
	}

void Application::titleChanged(SessionPropertiesEditor& editor,int id)
	{
	title_update(m_session,m_mainwin);
	}

void Application::descriptionChanged(SessionPropertiesEditor& editor,int id)
	{}

void Application::optionChanged(SessionPropertiesEditor& editor,int id,int option)
	{}


Application& Application::sessionLoad(const char* filename)
	{
	m_session.load(filename);
	m_session_editor.sessionUpdated();
	title_update(m_session,m_mainwin);
	return *this;
	}



Application::Application():
	m_mainwin("New session--Anja")
		,m_cols(m_mainwin,false)
			,m_session_control(m_cols,true)
			,m_session_editor(m_cols.insertMode({2,Anja::Box::EXPAND|Anja::Box::FILL}),m_session)
	,m_fullscreen(0)
	{
	m_session_control.append("New session","Load session","Reload session","Save session"
		,"Save session as","","Start engine","Stop engine","","Fullscreen"
		,"Exit","About Anja");
	m_session_control.callback(*this,0);
	m_mainwin.callback(*this,0);
	m_session_editor.sessionChangedCallback(*this,0);
	title_update(m_session,m_mainwin);
	m_mainwin.show();
	}

