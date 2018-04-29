#include <x11.h>
#include <unistd.h>

char* getcurrdir(size_t* size) {
    char *buf = NULL, *ptr = NULL;
    for (; ptr == NULL; *size *= 2) {
        buf = realloc(buf, *size);
        ptr = getcwd(buf, *size);
        if (ptr == NULL && errno != ERANGE)
            return (free(buf), NULL);
    }
    return buf;
}

char* read_stdin(int fd, char* str, size_t* len) {
    char buf[256];
    size_t nread = 0;
    if (str) {
        str = realloc(str, *len + 1);
        str[*len] = '\n';
        *len = *len + 1;
    }
    while ((nread = read(fd, buf, 256)) > 0) {
        str = realloc(str, *len + nread + 1);
        memcpy(str + *len, buf, nread);
        *len = *len + nread;
    }
    if (str) str[*len] = '\0';
    return str;
}

char* make_payload(int fd) {
    size_t size = 4096;
    char* payload = getcurrdir(&size);
    if (payload) {
        size = strlen(payload);
        payload = read_stdin(fd, payload, &size);
    }
    return payload;
}
int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    Window win;
    Atom xa_plumb = XInternAtom(x.display, "PLUMB", False);
    char* msg = make_payload(STDIN_FILENO);
    if (msg && None != (win = XGetSelectionOwner(x.display, xa_plumb))) {
        XChangeProperty(
            x.display, win, xa_plumb,
            XA_STRING, 8, PropModeAppend,
            (unsigned char*)msg, strlen(msg)+1);
        XFlush(x.display);
    }
    return 0;
}
