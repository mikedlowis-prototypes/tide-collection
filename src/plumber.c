#include <x11.h>
#include <unistd.h>

Atom SelType;

void plumb(char* str) {
    if (!str || !str[0]) return;
    printf("plumb('%s')\n", str);
}

void selclear(XConf* x, XEvent* e) {
    exit(0);
}

void propnotify(XConf* x, XEvent* e) {
    int success = 0, nreturn = 0;
    unsigned long nleft;
    XTextProperty prop = {0};
    if (e->xproperty.atom != SelType) return;
    success = XGetWindowProperty(
        x->display, x->self, SelType, 0, -1, True, AnyPropertyType,
        &prop.encoding, &prop.format, &prop.nitems, &nleft, &prop.value
    );
    if (Success == success) {
        char** strlist = NULL;
        if (Success == XmbTextPropertyToTextList(x->display, &prop, &strlist, &nreturn))
            for (int i = 0; i < nreturn; i++)
                plumb(strlist[i]);
        if (strlist) XFreeStringList(strlist);
    }
    if (prop.value) XFree(prop.value);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, PropertyChangeMask);
    SelType = XInternAtom(x.display, "PLUMB", 0);
    x.eventfns[SelectionClear] = selclear;
    x.eventfns[PropertyNotify] = propnotify;
    if (None == XGetSelectionOwner(x.display, SelType)) {
        XSetSelectionOwner(x.display, SelType, x.self, CurrentTime);
        if (x.self == XGetSelectionOwner(x.display, SelType)) {
            x11_event_loop(&x);
        }
    }
    return 1;
}
