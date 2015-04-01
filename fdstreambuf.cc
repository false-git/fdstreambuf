#include "fdstreambuf.h"
#include <iostream>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace wl {
    fdstreambuf::fdstreambuf(int fd): m_fd(fd), m_inbuf(0), m_inbufsize(0), m_owns_inbuf(false), m_outbuf(0), m_outbufsize(0), m_owns_outbuf(false) {
	setbuf(0, 1024);
	// デフォルトは出力バッファリングなし
	setwbuf(0, 0);
    }
    fdstreambuf::~fdstreambuf() {
	if (m_owns_inbuf) {
	    delete[] m_inbuf;
	}
	if (m_owns_outbuf) {
	    delete[] m_outbuf;
	}
    }
    int fdstreambuf::underflow() {
	//std::cout << "underflow" << std::endl;
	int __c = traits_type::eof();
	if (m_fd == -1) {
	    return __c;
	}
	bool initial = false;
	if (eback() == 0) {
	    setg(m_inbuf, m_inbuf + m_inbufsize, m_inbuf + m_inbufsize);
	    initial = true;
	    //std::cout << "initial" << std::endl;
	}
	const size_t unget_sz = initial ? 0 : std::min<size_t>((egptr() - eback()) / 2, 4);
	if (gptr() == egptr()) {
	    memmove(eback(), egptr() - unget_sz, unget_sz);
	    size_t nmemb = static_cast<size_t>(egptr() - eback() - unget_sz);
	    //std::cout << "before read: " << nmemb << std::endl;
	    ssize_t readed = read(m_fd, eback() + unget_sz, nmemb);
	    if (readed > 0) {
		setg(eback(), eback() + unget_sz, eback() + unget_sz + readed);
		__c = traits_type::to_int_type(*gptr());
	    }
	    //std::cout << "after read: " << readed << std::endl;
	} else {
	    __c = traits_type::to_int_type(*gptr());
	}
	return __c;
    }
    int fdstreambuf::overflow(int c) {
	//std::cout << "overflow: " << c << std::endl;
	if (m_fd == -1) {
	    return traits_type::eof();
	}
	if (pbase()) {
	    // バッファあり
	    char *pb_save = pbase();
	    char *epb_save = epptr();
	    if (c != traits_type::eof()) {
		*pptr() = traits_type::eof();
		pbump(1);
	    }
	    if (pptr() != pbase()) {
		size_t nmemb = static_cast<size_t>(pptr() - pbase());
		//std::cout << "before write: " << nmemb << std::endl;
		ssize_t writed = write(m_fd, pbase(), nmemb);
		//std::cout << "after write: " << writed << std::endl;
		if (writed != nmemb) {
		    return traits_type::eof();
		}
		setp(pb_save, epb_save);
	    }
	} else {
	    // バッファなし
	    char b = c;
	    ssize_t writed = write(m_fd, &b, 1);
	    //std::cout << "after write: " << writed << std::endl;
	    if (writed != 1) {
		return traits_type::eof();
	    }
	}
	return traits_type::not_eof(c);
    }
    std::streambuf* fdstreambuf::setbuf(char *s, std::streamsize n) {
	//std::cout << "setbuf" << std::endl;
	setg(0, 0, 0);
	if (m_owns_inbuf) {
	    delete [] m_inbuf;
	}
	m_inbufsize = n;
	if (s) {
	    m_inbuf = s;
	    m_owns_inbuf = false;
	} else {
	    m_inbuf = new char[m_inbufsize];
	    m_owns_inbuf = true;
	}
	return this;
    }
    std::streambuf* fdstreambuf::pubsetwbuf(char *s, std::streamsize n) {
	return setwbuf(s, n);
    }
    std::streambuf* fdstreambuf::setwbuf(char *s, std::streamsize n) {
	//std::cout << "setwbuf" << std::endl;
	if (m_owns_outbuf) {
	    delete [] m_outbuf;
	}
	m_outbufsize = n;
	if (s) {
	    m_outbuf = s;
	    m_owns_outbuf = false;
	} else {
	    if (m_outbufsize) {
		m_outbuf = new char[m_outbufsize];
		m_owns_outbuf = true;
	    } else {
		m_outbuf = 0;
		m_owns_outbuf = false;
	    }
	}
	if (m_outbufsize) {
	    setp(m_outbuf, m_outbuf + m_outbufsize - 1);
	} else {
	    setp(0, 0);
	}
	return this;
    }
    std::streamsize fdstreambuf::xsputn(const char *s, std::streamsize n) {
	//std::cout << "xsputn: " << s << std::endl;
	if (m_fd == -1) {
	    return 0;
	}
	if (pbase() == 0) {
	    // 出力バッファリングなし
	    //std::cout << "before write: " << n << std::endl;
	    ssize_t writed = write(m_fd, s, n);
	    //std::cout << "after write: " << writed << std::endl;
	    return writed;
	} else {
	    // 出力バッファリングあり
	    std::streamsize result = std::streambuf::xsputn(s, n);
	    //std::cout << result << std::endl;
	    return result;
	}
    }
    int fdstreambuf::sync() {
	//std::cout << "sync" << std::endl;
	int result = 0;
	if (pbase()) {
	    // 出力バッファありのときはバッファフラッシュ
	    result = overflow();
	}

	// 入力バッファのクリアをした方が良いかどうか不明
	// setg(0, 0, 0)
	return result;
    }
}
