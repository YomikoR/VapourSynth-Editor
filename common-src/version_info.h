#ifndef VERSION_INFO_H_INCLUDED
#define VERSION_INFO_H_INCLUDED

#define VSE_VERSION_STR "R19-mod-6"

void print_version();

#if defined(Q_OS_WIN)
void hide_tty();
#endif

#endif
