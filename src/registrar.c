#include <x11.h>
#include <unistd.h>

Atom XA_REGISTRAR, XA_REGISTER, XA_DEREGISTER;

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void clientmsg(XConf* x, XEvent* e) {
    if (XA_REGISTER == e->xclient.message_type) {
        puts("register");
    } else if (XA_DEREGISTER == e->xclient.message_type) {
        puts("dergister");
    }
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    XA_REGISTRAR = XInternAtom(x.display, "REGISTRAR", 0);
    XA_REGISTER = XInternAtom(x.display, "REGISTER", 0);
    XA_DEREGISTER = XInternAtom(x.display, "DEREGISTER", 0);
    x.eventfns[SelectionClear] = selclear;
    x.eventfns[ClientMessage] = clientmsg;
    if (None == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
        XSetSelectionOwner(x.display, XA_REGISTRAR, x.self, CurrentTime);
        if (x.self == XGetSelectionOwner(x.display, XA_REGISTRAR)) {
            x11_event_loop(&x);
        }
    }
    return 1;
}
