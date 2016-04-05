/*
 * This is the main procedure, the details are elsewhere.  Since it is so
 * small, it is the only function in this whole program I can confidently say
 * is maybe probably sort of a bit slightly bug-free. 
 *
 *
 * ISCA BBS client v1.5    02/15/94 (des)
 *
 * Revision history:
 *
 * v1.0    06/20/93 (des)  Initial release of Unix client.
 * v1.01   07/06/93 (des)  (Hopefully) made it more portable, added automatic X
 *                         message wrapping from Marc Dionne (Marco Polo)
 * v1.02   07/12/93 (des)  Portable for more machines, fixed X wrapping bugs.
 * v1.03   07/26/93 (des)  Fixed another X wrap problem, added VMS porting code
 *                         from Marc Dionne (Marco Polo), added code for
 *                         dropping invalid characters as done in the BBS.
 * v1.1    07/27/93 (des)  Added NeXT window titles from Dave Lacey (Doctor
 *                         Dave).  Added 'shell' key to shell out of client.
 *                         Initial public release of VMS client.
 * v1.11   08/11/93 (des)  Several fairly minor bug fixes for Unix and VMS. VMS
 *                         version should now work on Wollongong's TCP.
 * v1.2    10/12/93 (des)  A number of small bug fixes, ctrl-W lists will now
 *                         save the previous listing to avoid seeing nothing
 *                         when hitting ctrl-W, and capture now captures forum
 *                         info and message number.  .bbsrc now not required.
 * v1.21   10/14/93 (des)  Small bug fixes, added ability to use BBSRC and
 *                         BBSTMP environment variables in the Unix version,
 *                         which had been added at 1.2 for VMS by Marc Dionne.
 * v1.3    10/24/93 (des)  Small bug fixes (as always!)  Added bbsrc config
 *                         stuff, bbsrc is now self-generating and no longer
 *                         required to be installed when first setting up the
 *                         client, making things hopefully much easier for the
 *                         less than clueful hordes. 
 * v1.31   11/01/93 (des)  Minor bug fixes.  Unix version now truncates file to
 *                         avoid extra blank lines.  Unreleased test version.
 * v1.4    11/08/93 (des)  Usual minor bug fixes.  Marco Polo comes through in
 *                         the crunch with UCX compatability for the VMS
 *                         version!  Capture after posting doesn't ask if you
 *                         want to clear the temp file.  BBSRC and BBSTMP files
 *                         chmod'ed to 0600 in the Unix version by default.
 *                         Gave in and let people use any key they want for
 *                         their hotkeys as long as it isn't a return and does
 *                         not conflict with any other hotkey.
 * v1.41   12/26/93 (des)  Minor bug fixes, added in ctrl-W line erase, plus
 *                         the character translation and stupid-term-program
 *                         protection of the real BBS (see inkey.c)  Finally
 *                         nabbed that real X message line wrapping bug, plus a
 *                         few others.  This release is more for testing these
 *                         fixes while I continue to look at the feasibility
 *                         usefulness of adding compression for the data coming
 *                         to the client from the BBS.  Made .bbsrc parsing
 *                         more forgiving.
 * v1.42   12/31/93 (des)  Bug fixes, added compability for a few more
 *                         machines, including Amigas running Unix, BSDI, and
 *                         SCO/Xenix.  Added "term" compatibility for Michael
 *                         O'Reilly's term package courtesy of Ozy.
 * v1.5    02/15/94 (des)  Fixed nasty character dropping bug, made enemy list
 *                         work for > 1 screen posts properly, plus it now
 *                         works correctly when in ANSI mode.  A couple of VMS
 *                         specific bugfixes.  Added ctrl-R ala Unix reprint to
 *                         match change in BBS code, changed 'bad character
 *                         eating' code used to keep safe from runaway pastes
 *                         and line noise to make it less obtrusive and work
 *                         better.  The character translation for \r\n -> \n
 *                         was updated to work well on older BSD systems as
 *                         well as VMS and termio-based systems (this was the
 *                         source of the nasty character dropping bug)
 *
 *                         This is likely to be pretty much it as far as my
 *                         [Serendipity's] contribution to the client, other
 *                         than fixing bugs present in 1.5, or fixes to bring
 *                         the client in line with any changes in how the BBS
 *                         itself does things.  Any new functionality will be
 *                         added by others...
 */
#include "defs.h"
#include "ext.h"

int
main (int argc, char *argv[])
{
    if (*argv[0] == '-')
        login_shell = true;
    else
        login_shell = false;
    initialize ();
    findhome ();
    readbbsrc ();
    opentmpfile ();
    arguments (argc, argv);
    connectbbs ();
    siginit ();
    telinit ();
    setterm ();
    looper ();
    exit (0);
    return 0;
}
