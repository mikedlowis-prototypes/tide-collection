#include <x11.h>
#include <unistd.h>

char* SelText;  // The text of the clipboard selection
Atom SelType;   // The X11 selection name. Always CLIPBOARD
Atom SelTarget; // The conversion target for the selection (always string or utf8)

void plumb(XConf* x, Window plumber, char* msg) {
    printf("sent\n");
    Atom xa_text = XInternAtom(x->display, "TEXT", True);
    XChangeProperty(
        x->display, plumber, SelType,
        xa_text, 8, PropModeAppend,
        (unsigned char*)msg, strlen(msg));
    XFlush(x->display);
}

int main(int argc, char** argv) {
    XConf x;
    x11_init(&x);
    x11_mkwin(&x, 1, 1, 0);
    SelType = XInternAtom(x.display, "PLUMB", 0);
    //x.eventfns[SelectionRequest] = selrequest;
    Window win;
    if (None != (win = XGetSelectionOwner(x.display, SelType))) {
        plumb(&x, win, "hi!");
    }
    return 0;
}
