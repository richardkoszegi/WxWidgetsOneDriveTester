#ifndef WebLogin_H
#define WebLogin_H

#ifndef WX_PRECOMP
#include <wx/dialog.h>
#endif
#include <wx/webview.h>

// Window containing an embedded browser for authenticating with OAuth to certain (almost all) cloud provider.

class WebLogin : public wxDialog
{
public:
	WebLogin(const wxString url, wxString compareToClose, wxSize size = wxSize(500, 400));
	WebLogin(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const wxString title = "Login");
	virtual ~WebLogin();

	void OnTitleChanged(wxWebViewEvent& evt);
	void OnPageLoaded(wxWebViewEvent& evt);

	wxWebView* m_browser;
protected:
	static const long ID_WEBVIEW;
private:
	wxString compareToClose;
	void Init();
};


#endif

