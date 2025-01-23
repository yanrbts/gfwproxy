/*
 * Copyright (c) 2024-2024, yanruibinghxu@gmail.com All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include <gf_core.h>
#include <gf_conf.h>
#include <gf_signal.h>

#define GF_CONF_PATH        "conf/nutcracker.yml"
#define GF_LOG_DEFAULT      LOG_NOTICE
#define GF_LOG_MIN          LOG_EMERG
#define GF_LOG_MAX          LOG_PVERB
#define GF_LOG_PATH         NULL
#define GF_STATS_PORT       STATS_PORT
#define GF_STATS_ADDR       STATS_ADDR
#define GF_STATS_INTERVAL   STATS_INTERVAL
#define GF_PID_FILE         NULL
#define GF_MBUF_SIZE        MBUF_SIZE
#define GF_MBUF_MIN_SIZE    MBUF_MIN_SIZE
#define GF_MBUF_MAX_SIZE    MBUF_MAX_SIZE

static int show_help;
static int show_version;
static int test_conf;
static int daemonize;
static int describe_stats;

static const struct option long_options[] = {
    { "help",           no_argument,        NULL,   'h' },
    { "version",        no_argument,        NULL,   'V' },
    { "test-conf",      no_argument,        NULL,   't' },
    { "daemonize",      no_argument,        NULL,   'd' },
    { "describe-stats", no_argument,        NULL,   'D' },
    { "verbose",        required_argument,  NULL,   'v' },
    { "output",         required_argument,  NULL,   'o' },
    { "conf-file",      required_argument,  NULL,   'c' },
    { "stats-port",     required_argument,  NULL,   's' },
    { "stats-interval", required_argument,  NULL,   'i' },
    { "stats-addr",     required_argument,  NULL,   'a' },
    { "pid-file",       required_argument,  NULL,   'p' },
    { "mbuf-size",      required_argument,  NULL,   'm' },
    { NULL,             0,                  NULL,    0  }
};

static const char short_options[] = "hVtdDv:o:c:s:i:a:p:m:";

static rstatus_t
gf_daemonize(int dump_core)
{
    rstatus_t status;
    pid_t pid, sid;
    int fd;

    pid = fork();
    switch (pid) {
    case -1:
        break;
    case 0:
        break;
    default:
        /* parent terminates */
        _exit(0);
    }

    /* 1st child continues and becomes the session leader */
    sid = setsid();
    if (sid < 0) {
        log_error("setsid() failed: %s", strerror(errno));
        return GF_ERROR;
    }

    /* When a controlling process loses its terminal connection, 
     * the kernel sends it a SIGHUP signal to inform it of this fact.*/
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        log_error("signal(SIGHUP, SIG_IGN) failed: %s", strerror(errno));
        return GF_ERROR;
    }

    pid = fork();
    switch (pid) {
    case -1:
        log_error("fork() failed: %s", strerror(errno));
        return GF_ERROR;
    case 0:
        break;
    default: /* 1st child terminates */
        _exit(0);
    }

    /* 2nd child continues */
    /* change working directory */
    if (dump_core == 0) {
        status = chdir("/");
        if (status < 0) {
            log_error("chdir(\"/\") failed: %s", strerror(errno));
            return GF_ERROR;
        }
    }

    /* clear file mode creation mask */
    umask(0);

    /* redirect stdin, stdout and stderr to "/dev/null" */
    fd = open("/dev/null", O_RDWR);
    if (fd < 0) {
        log_error("open(\"/dev/null\") failed: %s", strerror(errno));
        return GF_ERROR;
    }

    status = dup2(fd, STDIN_FILENO);
    if (status < 0) {
        log_error("dup2(%d, STDIN) failed: %s", fd, strerror(errno));
        close(fd);
        return GF_ERROR;
    }

    status = dup2(fd, STDOUT_FILENO);
    if (status < 0) {
        log_error("dup2(%d, STDOUT) failed: %s", fd, strerror(errno));
        close(fd);
        return GF_ERROR;
    }

    status = dup2(fd, STDERR_FILENO);
    if (status < 0) {
        log_error("dup2(%d, STDERR) failed: %s", fd, strerror(errno));
        close(fd);
        return GF_ERROR;
    }

    if (fd > STDERR_FILENO) {
        status = close(fd);
        if (status < 0) {
            log_error("close(%d) failed: %s", fd, strerror(errno));
            return GF_ERROR;
        }
    }

    return GF_OK;
}

static void
gf_print_run(const struct instance *nci)
{
    int status;
    struct utsname name;

    /* The uname() system call returns a range of 
     * identifying information about the host
     * system on which an application is running, 
     * in the structure pointed to by utsbuf. */
    status = uname(&name);

    if (status < 0) {
        loga("gfw-%s started on pid %d", GF_VERSION_STRING, nci->pid);
    } else {
        loga("gfw-%s built for %s %s %s started on pid %d",
             GF_VERSION_STRING, name.sysname, name.release, name.machine, nci->pid);
    }

    loga("run, rabbit run / dig that hole, forget the sun / "
         "and when at last the work is done / don't sit down / "
         "it's time to dig another one");
}

static void
gf_print_done(void)
{
    loga("done, rabbit done");
}

static void
gf_show_usage(void)
{
    log_stderr(
        "Usage: gfw [-?hVdDt] [-v verbosity level] [-o output file]" CRLF
        "           [-c conf file] [-s stats port] [-a stats addr]" CRLF
        "           [-i stats interval] [-p pid file] [-m mbuf size]" CRLF
        "");
    log_stderr(
        "Options:" CRLF
        "  -h, --help             : this help" CRLF
        "  -V, --version          : show version and exit" CRLF
        "  -t, --test-conf        : test configuration for syntax errors and exit" CRLF
        "  -d, --daemonize        : run as a daemon" CRLF
        "  -D, --describe-stats   : print stats description and exit");
    log_stderr(
        "  -v, --verbose=N        : set logging level (default: %d, min: %d, max: %d)" CRLF
        "  -o, --output=S         : set logging file (default: %s)" CRLF
        "  -c, --conf-file=S      : set configuration file (default: %s)" CRLF
        "  -s, --stats-port=N     : set stats monitoring port (default: %d)" CRLF
        "  -a, --stats-addr=S     : set stats monitoring ip (default: %s)" CRLF
        "  -i, --stats-interval=N : set stats aggregation interval in msec (default: %d msec)" CRLF
        "  -p, --pid-file=S       : set pid file (default: %s)" CRLF
        "  -m, --mbuf-size=N      : set size of mbuf chunk in bytes (default: %d bytes)" CRLF
        "",
        GF_LOG_DEFAULT, GF_LOG_MIN, GF_LOG_MAX,
        GF_LOG_PATH != NULL ? GF_LOG_PATH : "stderr",
        GF_CONF_PATH,
        GF_STATS_PORT, GF_STATS_ADDR, GF_STATS_INTERVAL,
        GF_PID_FILE != NULL ? GF_PID_FILE : "off",
        GF_MBUF_SIZE);
}

static rstatus_t
gf_create_pidfile(struct instance *nci)
{
    char pid[GF_UINTMAX_MAXLEN];
    int fd, pid_len;
    ssize_t n;

    fd = open(nci->pid_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        log_error("opening pid file '%s' failed: %s", 
                  nci->pid_filename,
                  strerror(errno));
        return GF_ERROR;
    }
    nci->pidfile = 1;

    pid_len = gf_snprintf(pid, GF_UINTMAX_MAXLEN, "%d", nci->pid);
    n = gf_write(fd, pid, pid_len);
    if (n < 0) {
        log_error("write to pid file '%s' failed: %s", 
                  nci->pid_filename,
                  strerror(errno));
        return GF_ERROR;
    }
    close(fd);

    return GF_OK;
}

static void
gf_remove_pidfile(struct instance *nci)
{
    int status;

    status = unlink(nci->pid_filename);
    if (status < 0) {
        log_error("unlink of pid file '%s' failed, ignored: %s",
                  nci->pid_filename, strerror(errno));
    }
}

static void
gf_set_default_options(struct instance *nci)
{
    int status;

    nci->ctx = NULL;

    nci->log_level = GF_LOG_DEFAULT;
    nci->log_filename = GF_LOG_PATH;
    nci->conf_filename = GF_CONF_PATH;
    nci->stats_port = GF_STATS_PORT;
    nci->stats_addr = GF_STATS_ADDR;
    nci->stats_interval = GF_STATS_INTERVAL;

    status = gf_gethostname(nci->hostname, GF_MAXHOSTNAMELEN);
    if (status < 0) {
        log_warn("gethostname failed, ignored: %s", strerror(errno));
        gf_snprintf(nci->hostname, GF_MAXHOSTNAMELEN, "unknown");
    }

    nci->hostname[GF_MAXHOSTNAMELEN-1] = '\0';
    nci->mbuf_chunk_size = GF_MBUF_SIZE;
    nci->pid = (pid_t)-1;
    nci->pid_filename = NULL;
    nci->pidfile = 0;
}

static rstatus_t
gf_get_options(int argc, char **argv, struct instance *nci)
{
    int c, value;
    opterr = 0;

    for (;;) {
        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) {
            /* no more options */
            break;
        }

        switch (c) {
        case 'h':
            show_version = 1;
            show_help = 1;
            break;
        case 'V':
            show_version = 1;
            break;
        case 't':
            test_conf = 1;
            break;
        case 'd':
            daemonize = 1;
            break;
        case 'D':
            describe_stats = 1;
            show_version = 1;
            break;
        case 'v':
            value = gf_atoi(optarg, strlen(optarg));
            if (value < 0) {
                log_stderr("nutcracker: option -v requires a number");
                return GF_ERROR;
            }
            nci->log_level = value;
            break;
        case 'o':
            nci->log_filename = optarg;
            break;
        case 'c':
            nci->conf_filename = optarg;
            break;
        case 's':
            value = gf_atoi(optarg, strlen(optarg));
            if (value < 0) {
                log_stderr("nutcracker: option -s requires a number");
                return GF_ERROR;
            }
            if (!gf_valid_port(value)) {
                log_stderr("nutcracker: option -s value %d is not a valid "
                           "port", value);
                return GF_ERROR;
            }
            nci->stats_port = (uint16_t)value;
            break;
        case 'i':
            value = gf_atoi(optarg, strlen(optarg));
            if (value < 0) {
                log_stderr("nutcracker: option -i requires a number");
                return GF_ERROR;
            }
            nci->stats_interval = value;
            break;
        case 'a':
            nci->stats_addr = optarg;
            break;
        case 'p':
            nci->pid_filename = optarg;
            break;
        case 'm':
            value = gf_atoi(optarg, strlen(optarg));
            if (value <= 0) {
                log_stderr("gfw: option -m requires a non-zero number");
                return GF_ERROR;
            }

            if (value < GF_MBUF_MIN_SIZE || value > GF_MBUF_MAX_SIZE) {
                log_stderr("gfw: mbuf chunk size must be between %d and"
                           " %d bytes", GF_MBUF_MIN_SIZE, GF_MBUF_MAX_SIZE);
                return GF_ERROR;
            }
            nci->mbuf_chunk_size = (size_t)value;
            break;
        case '?':
            switch (optopt) {
            case 'o':
            case 'c':
            case 'p':
                log_stderr("gfw: option -%c requires a file name", optopt);
                break;
            case 'm':
            case 'v':
            case 's':
            case 'i':
                log_stderr("gfw: option -%c requires a number", optopt);
                break;
            case 'a':
                log_stderr("gfw: option -%c requires a string", optopt);
                break;
            default:
                log_stderr("gfw: invalid option -- '%c'", optopt);
                break;
            }
            return GF_ERROR;
        default:
            log_stderr("gfw: invalid option -- '%c'", optopt);
            return GF_ERROR;
        }
    }
    return GF_OK;
}

static bool
gf_test_conf(const struct instance *nci)
{
    struct conf *cf;

    cf = conf_create(nci->conf_filename);
    if (cf == NULL) {
        log_stderr("gfw: configuration file '%s' syntax is invalid",
                   nci->conf_filename);
        return false;
    }
    conf_destroy(cf);

    log_stderr("gfw: configuration file '%s' syntax is ok",
               nci->conf_filename);
    return true;
}

static rstatus_t
gf_pre_run(struct instance *nci)
{
    rstatus_t status;

    status = log_init(nci->log_level, nci->log_filename);
    if (status != GF_OK) {
        return status;
    }

    if (daemonize) {
        status = gf_daemonize(1);
        if (status != GF_OK) {
            return status;
        }
    }

    nci->pid = getpid();

    status = signal_init();
    if (status != GF_OK) {
        return status;
    }

    if (nci->pid_filename) {
        status = gf_create_pidfile(nci);
        if (status != GF_OK) {
            return status;
        }
    }
    gf_print_run(nci);

    return GF_OK;
}

static void
gf_post_run(struct instance *nci)
{
    if (nci->pidfile) {
        gf_remove_pidfile(nci);
    }
    signal_deinit();
    gf_print_done();
    log_deinit();
}

static void
gf_run(struct instance *nci)
{
    rstatus_t status;
    struct context *ctx;

    ctx = core_start(nci);
    if (ctx == NULL) {
        return;
    }

    /* run rabbit run */
    for (;;) {
        status = core_loop(ctx);
        if (status != GF_OK) {
            break;
        }
    }

    core_stop(ctx);
}

int main(int argc, char *argv[])
{
    rstatus_t status;
    struct instance nci;

    gf_set_default_options(&nci);

    status = gf_get_options(argc, argv, &nci);
    if (status != GF_OK) {
        gf_show_usage();
        exit(1);
    }

    if (show_version) {
        log_stderr("This is gfw-%s", GF_VERSION_STRING);
#if GF_HAVE_EPOLL
        log_stderr("async event backend: epoll");
#elif GF_HAVE_KQUEUE
        log_stderr("async event backend: kqueue");
#elif GF_HAVE_EVENT_PORTS
        log_stderr("async event backend: event_ports");
#else
        log_stderr("async event backend: unknown");
#endif
#if HAVE_ASSERT_PANIC || HAVE_ASSERT_LOG
        log_stderr("debugging assertions are enabled (--enable-debug=yes|full), nutcracker may be less efficient");
#endif
        // Log a blank line after the version
        log_stderr(" ");

        if (show_help) {
            gf_show_usage();
        }

        if (describe_stats) {
            stats_describe();
        }

        exit(0);
    }

    if (test_conf) {
        if (!gf_test_conf(&nci)) {
            exit(1);
        }
        exit(0);
    }

    status = gf_pre_run(&nci);
    if (status != GF_OK) {
        gf_post_run(&nci);
        exit(1);
    }
    gf_run(&nci);
    gf_post_run(&nci);

    exit(1);
}