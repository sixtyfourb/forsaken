/*
 * A backtrace on a fatal signal.
 *
 * This engine is played on handhelds that carry no debugger, and its crashes
 * arrive as a core file that systemd truncates and whose mappings point into an
 * AppImage mount that is gone by the time anyone looks. Printing the stack from
 * inside the process sidesteps all of that: it needs no gdb, no core, and no
 * second run.
 *
 * Build with -rdynamic or the frames outside this file have no names.
 *
 * Linux only, and not Android: there the platform's own tombstone is better
 * than anything this can print, and bionic has no execinfo.h.
 */
#if defined(__linux__) && !defined(__ANDROID__)

#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* A pointer as 16 hex digits, without printf: this runs inside a handler. */
static void write_hex( unsigned long v )
{
	char buf[19] = "0x0000000000000000";
	for ( int i = 0; i < 16; i++ )
		buf[17 - i] = "0123456789abcdef"[( v >> ( i * 4 ) ) & 0xf];
	write( STDERR_FILENO, buf, 18 );
}

static void crash_handler( int sig, siginfo_t *info, void *ctx )
{
	void *frames[64];
	int n;

	(void) ctx;

	/*
	 * Only async-signal-safe calls from here on - the process is already in a
	 * bad way, and a malloc inside a handler that fired mid-malloc hangs
	 * instead of reporting. backtrace_symbols_fd writes without allocating;
	 * backtrace_symbols, which returns strings, does not.
	 */
	const char *name = strsignal( sig );

	write( STDERR_FILENO, "\n*** ", 5 );
	if ( name )
		write( STDERR_FILENO, name, strlen( name ) );
	write( STDERR_FILENO, " at ", 4 );
	write_hex( (unsigned long) ( info ? info->si_addr : 0 ) );
	write( STDERR_FILENO, " - backtrace follows ***\n", 25 );

	n = backtrace( frames, (int) ( sizeof(frames) / sizeof(frames[0]) ) );
	backtrace_symbols_fd( frames, n, STDERR_FILENO );

	/* Die the way we would have died, so the exit status is honest. */
	signal( sig, SIG_DFL );
	raise( sig );
}

void crash_backtrace_init( void )
{
	struct sigaction sa;

	memset( &sa, 0, sizeof(sa) );
	sa.sa_sigaction = crash_handler;
	sa.sa_flags = SA_SIGINFO;
	sigemptyset( &sa.sa_mask );

	sigaction( SIGSEGV, &sa, NULL );
	sigaction( SIGBUS,  &sa, NULL );
	sigaction( SIGILL,  &sa, NULL );
	sigaction( SIGFPE,  &sa, NULL );
	sigaction( SIGABRT, &sa, NULL );
}

#else

void crash_backtrace_init( void ) {}

#endif
