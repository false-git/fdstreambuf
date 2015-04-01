#ifndef __FDSTREAMBUF_H
#define __FDSTREAMBUF_H

#include <streambuf>

namespace wl {
    class fdstreambuf: public std::streambuf {
    public:
	fdstreambuf(int fd = -1);
	virtual ~fdstreambuf();

	std::streambuf* pubsetwbuf(char *s, std::streamsize n);
    protected:
	virtual int underflow();
	virtual int overflow(int c = traits_type::eof());
	virtual std::streambuf* setbuf(char *s, std::streamsize n);
	virtual std::streamsize xsputn(const char *s, std::streamsize n);
	virtual int sync();

	virtual std::streambuf* setwbuf(char *s, std::streamsize n);
    private:
	int m_fd;
	char *m_inbuf;
	size_t m_inbufsize;
	bool m_owns_inbuf;
	char *m_outbuf;
	size_t m_outbufsize;
	bool m_owns_outbuf;
    };
}

#endif // __FDSTREAMBUF_H
