#include "tools/utils.h"
#include <chrono>
#include <iomanip>
#include <string>
#include <iconv.h>
#include "spdlog/fmt/fmt.h"
#include <csignal>
#include <cerrno>
#include <execinfo.h>
#include <cxxabi.h>
namespace rookietrader
{
SignalHandler::SignalLogCallback g_signalLogCb = nullptr;
SignalHandler::ExitCallback g_exitCb = nullptr;

void SignalHandler::PrintStackTrace(SignalLogCallback logCb)
{
	unsigned int max_frames = 127;
	// storage array for stack trace address data
	void *addrlist[max_frames + 1];

	// retrieve current stack addresses
	unsigned int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

	if (addrlen == 0) {
		logCb("no trace fetched");
		return;
	}

	// resolve addresses into strings containing "filename(function+address)",
	// Actually it will be ## program address function + offset
	// this array must be free()-ed
	char **symbollist = backtrace_symbols(addrlist, addrlen);

	// iterate over the returned symbol lines. skip the first, it is the
	// address of this function.
	for (unsigned int i = 4; i < addrlen; i++) {
		char *begin_name = nullptr;
		char *begin_offset = nullptr;
		char *end_offset = nullptr;

		// ./module(function+0x15c) [0x8048a6d]
		for (char *p = symbollist[i]; *p; ++p) {
			if (*p == '(')
				begin_name = p;
			else if (*p == '+')
				begin_offset = p;
			else if (*p == ')' && (begin_offset || begin_name))
				end_offset = p;
		}

		if (begin_name && end_offset && (begin_name > end_offset)) {
			*begin_name++ = '\0';
			*end_offset++ = '\0';
			if (begin_offset)
				*begin_offset++ = '\0';

			// mangled name is now in [begin_name, begin_offset) and caller
			// offset in [begin_offset, end_offset). now apply
			// __cxa_demangle():

			int status = 0;
			size_t funcnamesize = 8192;
			char funcname[8192];
			char *ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
			logCb(ret);
			char buf[256] = { 0 };
			sprintf(buf, "%30s ( %40s  + %6s) %s", symbollist[i], status == 0 ? ret : begin_name, begin_offset ? begin_offset : "", end_offset);
			logCb(buf);
		} else {
			// couldn't parse the line? print the whole line.
			logCb(symbollist[i]);
		}
	}
	free(symbollist);
}

void SignalHandler::OnSystemSignal(int signum)
{
    switch (signum)
    {
        case SIGURG:       // discard signal       urgent condition present on socket
            g_signalLogCb("signal SIGURG received");
            break;
        case SIGCONT:      // discard signal       continue after stop
            g_signalLogCb("signal SIGCONT received");
            break;        
        case SIGCHLD:      // discard signal       child status has changed
            g_signalLogCb("signal SIGCHLD received");
            break;
        case SIGIO:        // discard signal       I/O is possible on a descriptor (see fcntl(2))
            g_signalLogCb("signal SIGIO received");
            break;
        case SIGWINCH:     // discard signal       Window size change
            g_signalLogCb("signal SIGWINCH received");
            break;
        case SIGSTOP:      // stop process         stop (cannot be caught or ignored)
            g_signalLogCb("signal SIGSTOP received");
            break;
        case SIGTSTP:      // stop process         stop signal generated from keyboard
            g_signalLogCb("signal SIGTSTP received");
            break;
        case SIGTTIN:      // stop process         background read attempted from control terminal
            g_signalLogCb("signal SIGTTIN received");
            break;
        case SIGTTOU:      // stop process         background write attempted to control terminal
            g_signalLogCb("signal SIGTTOU received");
            break;
        case SIGINT:       // terminate process    interrupt program
            g_signalLogCb("signal SIGINT received");
            if (g_exitCb)
                g_exitCb(signum);
            else
                exit(signum);
            break;
        case SIGTERM:      // terminate process    software termination signal
            g_signalLogCb("signal SIGTERM received");
            break;
        case SIGKILL:      // terminate process    kill program
            g_signalLogCb("signal SIGKILL received");
            break;
        case SIGHUP:       // terminate process    terminal line hangup
            g_signalLogCb("signal SIGHUP received");
            break;
        case SIGPIPE:      // terminate process    write on a pipe with no reader
            g_signalLogCb("signal SIGPIPE received");
            break;
        case SIGALRM:      // terminate process    real-time timer expired
            g_signalLogCb("signal SIGALRM received");
            break;
        case SIGXCPU:      // terminate process    cpu time limit exceeded (see setrlimit(2))
            g_signalLogCb("signal SIGXCPU received");
            break;
        case SIGXFSZ:      // terminate process    file size limit exceeded (see setrlimit(2))
            g_signalLogCb("signal SIGXFSZ received");
            break;
        case SIGVTALRM:    // terminate process    virtual time alarm (see setitimer(2))
            g_signalLogCb("signal SIGVTALRM received");
            break;
        case SIGPROF:      // terminate process    profiling timer alarm (see setitimer(2))
            g_signalLogCb("signal SIGPROF received");
            break;
        case SIGUSR1:      // terminate process    User defined signal 1
            g_signalLogCb("signal SIGUSR1 received");
            break;
        case SIGUSR2:      // terminate process    User defined signal 2
            g_signalLogCb("signal SIGUSR2 received");
            break;
        case SIGQUIT:      // create core image    quit program
            g_signalLogCb("signal SIGQUIT received");
            break;
        case SIGILL:       // create core image    illegal instruction
            g_signalLogCb("signal SIGILL received");
            break;
        case SIGTRAP:      // create core image    trace trap
            g_signalLogCb("signal SIGTRAP received");
            break;
        case SIGABRT:      // create core image    abort program (formerly SIGIOT)
            g_signalLogCb("signal SIGABRT received");
            break;
        case SIGFPE:       // create core image    floating-point exception
            g_signalLogCb("signal SIGFPE received");
            break;
        case SIGBUS:       // create core image    bus error
            g_signalLogCb("signal SIGBUS received");
            break;
        case SIGSEGV:      // create core image    segmentation violation
            g_signalLogCb("signal SIGSEGV received");
            PrintStackTrace(g_signalLogCb);
            exit(signum);
            break;
        case SIGSYS:       // create core image    non-existent system call invoked
            g_signalLogCb("signal SIGSYS received");
            break;
        default:
            g_signalLogCb(fmt::format("signal {} received", signum));
            break;
    }
}
void SignalHandler::RegisterSignalCallbacks(SignalLogCallback logCb, ExitCallback exitCb)
{
    g_signalLogCb = logCb;
    g_exitCb = exitCb;
	for (int s = 1; s < NSIG; s++)
	{
		signal(s, SignalHandler::OnSystemSignal);
	}
}

std::string TimeUtils::GetCurrTradingDay()
{
    return "";
}

std::string TimeUtils::GetNextTradingDay()
{
    return "";
}

std::string TimeUtils::GetCurrDate()
{
    return "";
}

std::string TimeUtils::GetNextDate()
{
    return "";
}
std::string TimeUtils::GetTimeNow()
{
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::tm tmTime;
    localtime_r(&time, &tmTime);
    std::ostringstream oss;
    oss << std::put_time(&tmTime, "%H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << millisecs.count();
    return oss.str();
}
static int code_convert(const char* from_charset, const char* to_charset, char* inbuf, size_t inlen, char* outbuf, size_t outlen)
{
    iconv_t cd;
    char** pIn = &inbuf;
    char** pOut = &outbuf;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) return -1;

    int res = iconv(cd, pIn, &inlen, pOut, &outlen);
    iconv_close(cd);
    return res;
}
std::string StringUtils::GBKToUTF8(const std::string& inStr)
{
    int len = inStr.size() * 2 + 1;
    char* buf = (char*)std::malloc(sizeof(char)*len);
    
    if (code_convert("gb2312", "utf-8", (char*)inStr.c_str(), inStr.size(), buf, len) >= 0)
    {
        std::string res;
        res.append(buf);
        return res;
    }
    else 
    {
        std::free(buf);
        return "";
    }
}

};