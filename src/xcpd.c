#include <x11.h>
#include <unistd.h>

char* SelText;  // The text of the clipboard selection
Atom SelType;   // The X11 selection name. Always CLIPBOARD
Atom SelTarget; // The conversion target for the selection (always string or utf8)

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

int daemonize(void) {
    pid_t pid;
    if (chdir("/") < 0) return -1;
    close(0), close(1), close(2);
    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) _exit(0);
    if (setsid() < 0) return -1;
    pid = fork();
    if (pid < 0) return -1;
    if (pid > 0) _exit(0);
    return 0;
}

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

void serve_selection(void) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    SelType   = XInternAtom(x.display, "CLIPBOARD", 0);
    SelTarget = XInternAtom(x.display, "UTF8_STRING", 0);
    x.eventfns[SelectionRequest] = selrequest;
    x.eventfns[SelectionClear]   = selclear;
    if (SelTarget == None)
        SelTarget = XInternAtom(x.display, "STRING", 0);
    XSetSelectionOwner(x.display, SelType, x.self, CurrentTime);
    x11_event_loop(&x);
}

int main(int argc, char** argv) {
    SelText = fdgets(STDIN_FILENO);
    if (SelText) {
        if (daemonize() == 0) {
            serve_selection();
        } else {
            perror("daemonize() :");
            return 1;
        }
    }
    return 0;
}
