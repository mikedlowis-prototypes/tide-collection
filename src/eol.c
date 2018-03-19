#include <posix.h>
#include <stdc.h>

enum { DOS, UNIX } EolStyle = UNIX;

void eol(int fd) {
    static char ibuf[BUFSIZ];
    static char obuf[BUFSIZ*2]; // Double to account for worst-case size increase
    for (long nin = 0, nout = 0; (nin = read(fd, ibuf, sizeof(ibuf))) > 0; nout = 0) {
        for (long curr = 0; curr < nin; curr++) {
            switch (ibuf[curr]) {
                case '\r':
                    curr++; /* Always skip these. We only care about \n */
                    break;

                case '\n': /* transalte newlines to the proper style */
                    if (EolStyle == DOS) {
                        obuf[nout++] = '\r';
                        obuf[nout++] = '\n';
                    } else {
                        obuf[nout++] = '\n';
                    }
                    break;

                default:
                    obuf[nout++] = ibuf[curr];
                    break;
            }
        }
        writebuf(STDOUT_FILENO, obuf, nout);
    }
}

int main(int argc, char** argv) {
    OPTBEGIN {
        case 'u': EolStyle = UNIX; break;
        case 'd': EolStyle = DOS;  break;
        default:  exit(1);
    } OPTEND;
    if (argc == 0) {
        eol(STDIN_FILENO);
    } else {
        for (; argc > 0; argc--, argv++) {
            int fd = open(*argv, O_RDONLY);
            if (fd >= 0) {
                eol(fd);
                close(fd);
            }
        }
    }
}
