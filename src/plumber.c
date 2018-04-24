#include <x11.h>
#include <unistd.h>

char* SelText;  // The text of the clipboard selection
Atom SelType;   // The X11 selection name. Always CLIPBOARD
Atom SelTarget; // The conversion target for the selection (always string or utf8)

void selnotify(XConf* x, XEvent* e) {
    /* bail if the selection cannot be converted */
    if (e->xselection.property == None) return;
    Atom rtype;
    unsigned long format = 0, nitems = 0, nleft = 0;
    unsigned char* propdata = NULL;
    XGetWindowProperty(x->display, x->self, SelType, 0, -1, False, AnyPropertyType,
                       &rtype, (int*)&format, &nitems, &nleft, &propdata);
    if (e->xselection.target == SelTarget) {
        printf("rcvd: '%s'\n", (char*)propdata);
    }
    /* cleanup */
    if (propdata) XFree(propdata);
    XSetSelectionOwner(x->display, SelType, x->self, CurrentTime);
}

void selclear(XConf* x, XEvent* e) {
    XConvertSelection(x->display, SelType, SelTarget, SelType, x->self, CurrentTime);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    SelType   = XInternAtom(x.display, "PLUMB", 0);
    SelTarget = XInternAtom(x.display, "UTF8_STRING", 0);
    x.eventfns[SelectionNotify] = selnotify;
    x.eventfns[SelectionClear]  = selclear;
    XSetSelectionOwner(x.display, SelType, x.self, CurrentTime);
    while (1) x11_process_events(&x);

    return 0;
}
