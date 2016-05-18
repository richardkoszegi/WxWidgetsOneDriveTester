#include "cURL_Thread.h"
#include <wx/log.h>  // wxLogDebug
using namespace Network;

bool cURL_Thread::curl_global_inited = false;
cURL_Thread* cURL_Thread::instance = 0;
wxCriticalSection cURL_Thread::classCS;

// Thread loop.
wxThread::ExitCode cURL_Thread::Entry()
{
	// wxEvtHandler* m_pHandler
	// wxQueueEvent(m_pHandler, new wxThreadEvent(wxEVT_COMMAND_MYTHREAD_UPDATE));
	long wait;
	int M, Q, U = -1;
	fd_set R, W, E;
	struct timeval T;
	CURLMsg* msg;
begin:
	if (!curl_global_inited) {
		curl_global_inited = true;
		curl_global_init(CURL_GLOBAL_ALL);
	}
	cm = curl_multi_init();
	curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_CONNECTIONS);

	unsigned int i = 0;
	while (i < MAX_CONNECTIONS)
	{
		if (!LoadRequest()) { break; } i++;
	}

	int running_handles = -1;
	while (running_handles)
	{
		curl_multi_perform(cm, &running_handles);

		if (running_handles)
		{
			FD_ZERO(&R);
			FD_ZERO(&W);
			FD_ZERO(&E);

			if (curl_multi_fdset(cm, &R, &W, &E, &M)) {
				wxLogDebug("E: curl_multi_fdset\n");
				return (wxThread::ExitCode)EXIT_FAILURE;
			}

			if (curl_multi_timeout(cm, &wait)) {
				wxLogDebug("E: curl_multi_timeout\n");
				return (wxThread::ExitCode)EXIT_FAILURE;
			}
			if (wait == -1) wait = 100;

			if (M == -1) {
#ifdef WIN32
				Sleep(wait);
#else
				sleep(wait / 1000);
#endif
			}
			else {
				T.tv_sec = wait / 1000;
				T.tv_usec = (wait % 1000) * 1000;

				if (0 > select(M + 1, &R, &W, &E, &T)) {
					wxLogDebug("E: select(%i,,,,%li): %i: %s\n",
						M + 1, wait, errno, strerror(errno));
					return (wxThread::ExitCode)EXIT_FAILURE;
				}
			}
		}

		while ((msg = curl_multi_info_read(cm, &Q)))
		{
			if (msg->msg == CURLMSG_DONE) { // Only MSG_DONE is used as a message.
				Request* req;
				CURL* e = msg->easy_handle;
				curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &req);
				wxLogDebug("R: %d - %s <%s>\n",
					msg->data.result, curl_easy_strerror(msg->data.result), req->url);
				if (msg->data.result && req->error_callback) {
					req->error_callback(req, (int)(msg->data.result), curl_easy_strerror(msg->data.result));
				}
				if (req->send_chunks == false) {
					if (req->terminate_str) {
						// Append zero to the received string.
						Network::collector_receive_handler("", 1, 1, req);
					}
					req->receive_callback(req->rxbuf, 1, req->rxused, req);
					if (req->free_rxbuf) free(req->rxbuf);
				}
				curl_multi_remove_handle(cm, e);
				curl_easy_cleanup(e);
				delete req;
			}
			else {
				wxLogDebug("E: CURLMsg (%d)\n", msg->msg);
			}
			//LoadRequest moved from here to 1 "}" outer.
		}

		while (running_handles < MAX_CONNECTIONS)
		{
			if (!LoadRequest()) { break; } running_handles++;
		}
	}

	curl_multi_cleanup(cm);

	// On thread killing the critical section only left
	// when the instance pointer is set to zero in the destructor.
	classCS.Enter();
	if (!requests.empty()) { classCS.Leave(); goto begin; }

	// TODO: add global cleanup: curl_global_cleanup();
	//curl_global_inited = false; curl_global_cleanup();
	return (wxThread::ExitCode)0;
}

// Called from cURL_Thread context on initialization and on request completeion with more requests waiting.
// Return value shows if the load was successful.
bool cURL_Thread::LoadRequest()
{
	classCS.Enter();
	if (requests.empty()) { classCS.Leave(); return false; }
	Request* req = requests.front(); requests.pop_front();
	classCS.Leave();

	CURL *eh = curl_easy_init();

	if (req->hdrs) curl_easy_setopt(eh, CURLOPT_HTTPHEADER, req->hdrs);

	if (req->send_chunks)
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, req->receive_callback);
	else
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, Network::collector_receive_handler);
	curl_easy_setopt(eh, CURLOPT_WRITEDATA, req);
	curl_easy_setopt(eh, CURLOPT_HEADER, 0L);
	curl_easy_setopt(eh, CURLOPT_URL, (const char*)(req->url.c_str()));
	if (req->post != wxEmptyString)
	{
		if (req->alternative_verb) curl_easy_setopt(eh, CURLOPT_PUT, 1L);
		curl_easy_setopt(eh, CURLOPT_POSTFIELDS, (const char*)(req->post.c_str()));
	}
	else if (req->alternative_verb) curl_easy_setopt(eh, CURLOPT_CUSTOMREQUEST, "DELETE");

	curl_easy_setopt(eh, CURLOPT_PRIVATE, req);
	curl_easy_setopt(eh, CURLOPT_VERBOSE, 0L);

	curl_multi_add_handle(cm, eh);
	return true;
}

void cURL_Thread::AddRequest(Request* request)
{
	wxCriticalSectionLocker enter(classCS);
	this->requests.push_back(request);

	if (!this->Get().IsAlive()) {
		this->Get().Run();
	}
}


size_t Network::dummy_receive_handler(char *d, size_t n, size_t l, Request* self)
{
	size_t size = n*l;
	//wxLogDebug("aaaaa"); // Writes to console output.
	//wxLogMessage("bbbb"); // Writes to message box.
	return size;
}
void Network::dummy_error_handler(Request* self, int err_no, const char* err_str)
{

}

// Creates a growing array and copies there the received bytes.
// TODO: Check if using list<char*> can be faster than this.
size_t Network::collector_receive_handler(char *d, size_t n, size_t l, Request* self)
{
	char* rxbuf = self->rxbuf;
	size_t size = n*l;

	if (!rxbuf) {
		int malloc_size;
		if (self->terminate_str) { malloc_size = size + 1; }
		else { malloc_size = size; }

		rxbuf = (char*)malloc(malloc_size);
		memmove(rxbuf, d, size);

		self->rxbuf = rxbuf;
		self->rxsize = malloc_size;
		self->rxused = size;
	}
	else {
		size_t buf_free = self->rxsize - self->rxused;
		if (buf_free > size) {
			memmove(rxbuf + self->rxused, d, size);

			self->rxused += size;
		}
		else {
			size_t newsize = (self->rxsize + size) * 2;
			char* buf2 = (char*)malloc(newsize);
			memmove(buf2, rxbuf, self->rxused);
			memmove(buf2 + self->rxused, d, size);

			free(rxbuf);
			self->rxbuf = buf2;
			self->rxsize = newsize;
			self->rxused += size;
		}
	}
	return size;
}