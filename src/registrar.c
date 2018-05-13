#include <x11.h>
#include <unistd.h>

Atom XA_REGISTRAR, XA_ADD, XA_DEL, XA_OPEN;

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void clientmsg(XConf* x, XEvent* e) {
    if (XA_REGISTRAR != e->xclient.message_type)
        return;
    if (XA_ADD == e->xclient.data.l[0]) {
        puts("add");
    } else if (XA_DEL == e->xclient.data.l[0]) {
        puts("del");
    } else if (XA_OPEN == e->xclient.data.l[0]) {
        puts("open");
    }
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    XA_REGISTRAR = XInternAtom(x.display, "TIDE_REGISTRAR", 0);
    XA_ADD = XInternAtom(x.display, "ADD", 0);
    XA_DEL = XInternAtom(x.display, "DEL", 0);
    XA_OPEN = XInternAtom(x.display, "OPEN", 0);
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
