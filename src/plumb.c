#include <x11.h>
#include <unistd.h>

char* SelText;  // The text of the clipboard selection
Atom SelType;   // The X11 selection name. Always CLIPBOARD
Atom SelTarget; // The conversion target for the selection (always string or utf8)

void selrequest(XConf* x, XEvent* e) {
    XEvent s;
    s.xselection.type      = SelectionNotify;
    s.xselection.property  = e->xselectionrequest.property;
    s.xselection.requestor = e->xselectionrequest.requestor;
    s.xselection.selection = e->xselectionrequest.selection;
    s.xselection.target    = e->xselectionrequest.target;
    s.xselection.time      = e->xselectionrequest.time;
    Atom target    = e->xselectionrequest.target;
    Atom xatargets = XInternAtom(x->display, "TARGETS", 0);
    Atom xastring  = XInternAtom(x->display, "STRING", 0);
    if (target == xatargets) {
        /* respond with the supported type */
        XChangeProperty(
            x->display, s.xselection.requestor, s.xselection.property,
            XA_ATOM, 32, PropModeReplace,
            (unsigned char*)&SelTarget, 1);
    } else if (target == SelTarget || target == xastring) {
        XChangeProperty(
            x->display, s.xselection.requestor, s.xselection.property,
            SelTarget, 8, PropModeReplace,
            (unsigned char*)SelText, strlen(SelText));
    }
    XSendEvent(x->display, s.xselection.requestor, True, 0, &s);
}

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    SelType   = XInternAtom(x.display, "PLUMB", 0);
    SelTarget = XInternAtom(x.display, "UTF8_STRING", 0);
    x.eventfns[SelectionRequest] = selrequest;
    x.eventfns[SelectionClear]   = selclear;
    XSetSelectionOwner(x.display, SelType, x.self, CurrentTime);
    while (1) x11_process_events(&x);
    return 0;
}
