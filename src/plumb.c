#include <x11.h>
#include <unistd.h>

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
    if (None != (win = XGetSelectionOwner(x.display, xa_plumb)))
        plumb(&x, xa_plumb, win, "hi!");
    return 0;
}
