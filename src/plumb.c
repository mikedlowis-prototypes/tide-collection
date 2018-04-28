#include <x11.h>
#include <unistd.h>

char* fdgets(int fd) {
    char buf[256];
    size_t len = 0, nread = 0;
    char* str = NULL;
    while ((nread = read(fd, buf, 256)) > 0) {
        str = realloc(str, len + nread + 1);
        memcpy(str+len, buf, nread);
        len += nread;
    }
    if (str) str[len] = '\0';
    return str;
}

void plumb(XConf* x, Atom xa_plumb, Window plumber, char* msg) {
    XChangeProperty(
        x->display, plumber, xa_plumb,
        XA_STRING, 8, PropModeAppend,
        (unsigned char*)msg, strlen(msg)+1);
    XFlush(x->display);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    Window win;
    Atom xa_plumb = XInternAtom(x.display, "PLUMB", False);
    char* msg = fdgets(STDIN_FILENO);
    if (msg && None != (win = XGetSelectionOwner(x.display, xa_plumb)))
        plumb(&x, xa_plumb, win, msg);
    return 0;
}
