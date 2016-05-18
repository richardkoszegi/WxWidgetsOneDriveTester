#ifndef _cURL_THREAD_H_
#define _cURL_THREAD_H_

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#endif

#include <curl/multi.h>

#include <wx/string.h>
#include <wx/thread.h>

#include <list>
using namespace std;

#ifdef WIN32
//#pragma comment (lib, "LIBCURL.lib")
#pragma comment (lib, "libcurl_debug.lib")
#endif

#define MAX_CONNECTIONS 10

namespace Network
{
	enum e { HTTP_DEFAULT, HTTP_PUT, HTTP_DELETE };

	struct Request
	{
		wxString url;
		wxString post; // Contains the POST message. Needs to be url_encoded with libcurl.
		struct curl_slist* hdrs; // Header list.
		void* user_data;    // Pointer to the user defined data.
		bool send_chunks;   // False: Only sends receive callback when the whole content is received. (default:true)
		char* rxbuf; size_t rxsize, rxused; // Receive buffer for storing the received content when send_chuncks is set to false.
		bool free_rxbuf;    // False: rxbuf will not be freed (can be used outside rx_callback but needs to be manually freed). (default:true)
		bool terminate_str; // Adds a \0 to the end of the received content (rapidjson's insitu parsing needs this). (default:false)
		bool alternative_verb; // Changes the GET request to DELETE and the POST to PUT (default:false).

		size_t(*receive_callback)(char* d, size_t n, size_t l, Request* self);

		void(*error_callback)(Request* self, int err_no, const char* err_str);

		Request(wxString url,
			size_t(*receive_callback)(char* d, size_t n, size_t l, Request* self),
			void(*error_callback)(Request* self, int err_no, const char* err_str)
		)
			:url(url), receive_callback(receive_callback), error_callback(error_callback),
			post(wxEmptyString), hdrs(0), user_data(0), send_chunks(true), rxbuf(0), rxsize(0), rxused(0), free_rxbuf(true),
			terminate_str(false), alternative_verb(false)
		{}
		~Request()
		{
			if (hdrs) curl_slist_free_all(hdrs);
		}

		// Post string parameter needs to be url-encoded.
		void SetPOST(const wxString& post)
		{
			this->post = post;
		}
		// URL-encoding is done inside the function.
		void AddPOST(const char* field, const char* value)
		{
			if (this->post != wxEmptyString) this->post += '&';
			char* encfield = curl_easy_escape(0, field, strlen(field));
			char* encvalue = curl_easy_escape(0, value, strlen(value));
			this->post += encfield;
			this->post += '=';
			this->post += encvalue;
			curl_free(encfield);
			curl_free(encvalue);
		}
		void AddHeader(const char* header)
		{
			hdrs = curl_slist_append(hdrs, header);
		}
		void SetVerb(Network::e verb)
		{
			if (verb == Network::HTTP_DEFAULT) alternative_verb = false;
			else if (verb == Network::HTTP_DELETE || verb == Network::HTTP_PUT) alternative_verb = true;
		}
		void SetUserData(void* user_data)
		{
			this->user_data = user_data;
		}
		void DontSendChunks(bool dont_send_chunks = true)
		{
			this->send_chunks = !dont_send_chunks;
		}
		void DontFreeRXBuf(bool dont_free_rxbuf = true)
		{
			this->free_rxbuf = !dont_free_rxbuf;
		}
		void TerminateString(bool terminate_str = true)
		{
			this->terminate_str = terminate_str;
		}
	};

	class cURL_Thread : public wxThread
	{
	private:
		CURLM* cm;
		list<Request*> requests;
		//wxCriticalSection threadCS;

		static bool curl_global_inited;
		static cURL_Thread* instance;
		static wxCriticalSection classCS;
	public:
		static cURL_Thread& Get()
		{
			wxCriticalSectionLocker enter(classCS);
			if (!instance) instance = new cURL_Thread();
			return *instance;
		}

		cURL_Thread() : wxThread(wxTHREAD_DETACHED)
		{}
		~cURL_Thread()
		{
			//wxCriticalSectionLocker enter(threadCS);
			//classCS.Enter(); Critical section's Enter is called from Thread::Entry's method before termination.
			while (!requests.empty()) { delete requests.front(); requests.pop_front(); }
			instance = 0;
			classCS.Leave();
		}
		void AddRequest(Request* request);
	protected:
		virtual ExitCode Entry();
		bool LoadRequest();
	};

	size_t dummy_receive_handler(char *d, size_t n, size_t l, Request* self);
	void dummy_error_handler(Request* self, int err_no, const char* err_str);

	// Used when send_chunks is disabled (collects request parts).
	size_t collector_receive_handler(char *d, size_t n, size_t l, Request* self);
}

#endif

