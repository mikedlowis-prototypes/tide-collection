#include <x11.h>
#include <unistd.h>

Atom XA_REGISTRAR;

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void propnotify(XConf* x, XEvent* e) {
}

void clientmsg(XConf* x, XEvent* e) {
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    XA_REGISTRAR = XInternAtom(x.display, "REGISTRAR", 0);
    x.eventfns[SelectionClear] = selclear;
    x.eventfns[PropertyNotify] = propnotify;
    x.eventfns[ClientMessage] = clientmsg;
    if (None == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
        XSetSelectionOwner(x.display, XA_REGISTRAR, x.self, CurrentTime);
        if (x.self == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
            x11_event_loop(&x);
        }
    }
    return 1;
}
