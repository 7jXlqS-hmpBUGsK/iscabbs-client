#include "defs.h"
#include "ext.h"
#include <stdarg.h>


void
feed_pager (int startrow, ...)
{
    int     row;
    char   *s;
    va_list ap;

    row = startrow;
    va_start (ap, startrow);
    for (;;) {
        s = va_arg (ap, char *);

        if (!s)
            break;
        std_printf ("%s", s);
        row++;
        if (row == rows) {
            /* More prompt */
            printf ("--MORE-- ");
            fflush (stdout);
            switch (inkey ()) {
            case 'n':
            case 'N':
            case 's':
            case 'S':
            case 'q':
            case 'Q':
                printf ("\r        \r");
                va_end (ap);
                return;
                /* NOTREACHED */
            case '\n':
                row--;
                printf ("\r        \r");
                break;
            default:
                row = 1;
                printf ("\r        \r");
                break;
            }
        }
    }
    va_end (ap);
    return;
}


void
copyright (void)
{
    std_printf ("Copyright\r\n\n");

    feed_pager (3, "Copyright (C)\r\n",
                "  1995-2003 Michael Hampton.      (GPL: Cool stuff; current maintainer)\r\n",
                "  1993-1994 Doug Siebert.         (GPL: Client core)\r\n", "Portions Copyright (C)\r\n",
#ifdef ENABLE_SAVE_PASSWORD
                "  1995 Jonathan Pickard.          (GPL: Saved-password algorithm)\r\n",
#endif
                "  1994 David Bailey.              (GPL: Expanded friend list)\r\n",
                "  1994 Marc Dionne.               (GPL: Early patches to client core)\r\n",
#ifdef ENABLE_SOCKS
                "  1993-1995 NEC Systems Laboratory.             (BSD: SOCKS)\r\n",
                "  1989 Regents of the University of California. (BSD: SOCKS)\r\n",
#endif
                "  Above portions used with permission.\r\n", NULL);
}


void
license (void)
{
    std_printf ("License\r\n\n");

    feed_pager (3,
                "                    GNU GENERAL PUBLIC LICENSE\r\n",
                "   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\r\n",
                "\r\n",
                "  0. This License applies to any program or other work which contains\r\n",
                "a notice placed by the copyright holder saying it may be distributed\r\n",
                "under the terms of this General Public License.  The \"Program\", below,\r\n",
                "refers to any such program or work, and a \"work based on the Program\"\r\n",
                "means either the Program or any derivative work under copyright law:\r\n",
                "that is to say, a work containing the Program or a portion of it,\r\n",
                "either verbatim or with modifications and/or translated into another\r\n",
                "language.  (Hereinafter, translation is included without limitation in\r\n",
                "the term \"modification\".)  Each licensee is addressed as \"you\".\r\n",
                "\r\n",
                "Activities other than copying, distribution and modification are not\r\n",
                "covered by this License; they are outside its scope.  The act of\r\n",
                "running the Program is not restricted, and the output from the Program\r\n",
                "is covered only if its contents constitute a work based on the\r\n",
                "Program (independent of having been made by running the Program).\r\n",
                "Whether that is true depends on what the Program does.\r\n",
                "\r\n",
                "  1. You may copy and distribute verbatim copies of the Program's\r\n",
                "source code as you receive it, in any medium, provided that you\r\n",
                "conspicuously and appropriately publish on each copy an appropriate\r\n",
                "copyright notice and disclaimer of warranty; keep intact all the\r\n",
                "notices that refer to this License and to the absence of any warranty;\r\n",
                "and give any other recipients of the Program a copy of this License\r\n",
                "along with the Program.\r\n",
                "\r\n",
                "You may charge a fee for the physical act of transferring a copy, and\r\n",
                "you may at your option offer warranty protection in exchange for a fee.\r\n",
                "\r\n",
                "  2. You may modify your copy or copies of the Program or any portion\r\n",
                "of it, thus forming a work based on the Program, and copy and\r\n",
                "distribute such modifications or work under the terms of Section 1\r\n",
                "above, provided that you also meet all of these conditions:\r\n",
                "\r\n",
                "    a) You must cause the modified files to carry prominent notices\r\n",
                "    stating that you changed the files and the date of any change.\r\n",
                "\r\n",
                "    b) You must cause any work that you distribute or publish, that in\r\n",
                "    whole or in part contains or is derived from the Program or any\r\n",
                "    part thereof, to be licensed as a whole at no charge to all third\r\n",
                "    parties under the terms of this License.\r\n",
                "\r\n",
                "    c) If the modified program normally reads commands interactively\r\n",
                "    when run, you must cause it, when started running for such\r\n",
                "    interactive use in the most ordinary way, to print or display an\r\n",
                "    announcement including an appropriate copyright notice and a\r\n",
                "    notice that there is no warranty (or else, saying that you provide\r\n",
                "    a warranty) and that users may redistribute the program under\r\n",
                "    these conditions, and telling the user how to view a copy of this\r\n",
                "    License.  (Exception: if the Program itself is interactive but\r\n",
                "    does not normally print such an announcement, your work based on\r\n",
                "    the Program is not required to print an announcement.)\r\n",
                "\r\n",
                "These requirements apply to the modified work as a whole.  If\r\n",
                "identifiable sections of that work are not derived from the Program,\r\n",
                "and can be reasonably considered independent and separate works in\r\n",
                "themselves, then this License, and its terms, do not apply to those\r\n",
                "sections when you distribute them as separate works.  But when you\r\n",
                "distribute the same sections as part of a whole which is a work based\r\n",
                "on the Program, the distribution of the whole must be on the terms of\r\n",
                "this License, whose permissions for other licensees extend to the\r\n",
                "entire whole, and thus to each and every part regardless of who wrote it.\r\n",
                "\r\n",
                "Thus, it is not the intent of this section to claim rights or contest\r\n",
                "your rights to work written entirely by you; rather, the intent is to\r\n",
                "exercise the right to control the distribution of derivative or\r\n",
                "collective works based on the Program.\r\n",
                "\r\n",
                "In addition, mere aggregation of another work not based on the Program\r\n",
                "with the Program (or with a work based on the Program) on a volume of\r\n",
                "a storage or distribution medium does not bring the other work under\r\n",
                "the scope of this License.\r\n",
                "\r\n",
                "  3. You may copy and distribute the Program (or a work based on it,\r\n",
                "under Section 2) in object code or executable form under the terms of\r\n",
                "Sections 1 and 2 above provided that you also do one of the following:\r\n",
                "\r\n",
                "    a) Accompany it with the complete corresponding machine-readable\r\n",
                "    source code, which must be distributed under the terms of Sections\r\n",
                "    1 and 2 above on a medium customarily used for software interchange; or,\r\n",
                "\r\n",
                "    b) Accompany it with a written offer, valid for at least three\r\n",
                "    years, to give any third party, for a charge no more than your\r\n",
                "    cost of physically performing source distribution, a complete\r\n",
                "    machine-readable copy of the corresponding source code, to be\r\n",
                "    distributed under the terms of Sections 1 and 2 above on a medium\r\n",
                "    customarily used for software interchange; or,\r\n",
                "\r\n",
                "    c) Accompany it with the information you received as to the offer\r\n",
                "    to distribute corresponding source code.  (This alternative is\r\n",
                "    allowed only for noncommercial distribution and only if you\r\n",
                "    received the program in object code or executable form with such\r\n",
                "    an offer, in accord with Subsection b above.)\r\n",
                "\r\n",
                "The source code for a work means the preferred form of the work for\r\n",
                "making modifications to it.  For an executable work, complete source\r\n",
                "code means all the source code for all modules it contains, plus any\r\n",
                "associated interface definition files, plus the scripts used to\r\n",
                "control compilation and installation of the executable.  However, as a\r\n",
                "special exception, the source code distributed need not include\r\n",
                "anything that is normally distributed (in either source or binary\r\n",
                "form) with the major components (compiler, kernel, and so on) of the\r\n",
                "operating system on which the executable runs, unless that component\r\n",
                "itself accompanies the executable.\r\n",
                "\r\n",
                "If distribution of executable or object code is made by offering\r\n",
                "access to copy from a designated place, then offering equivalent\r\n",
                "access to copy the source code from the same place counts as\r\n",
                "distribution of the source code, even though third parties are not\r\n",
                "compelled to copy the source along with the object code.\r\n",
                "\r\n",
                "  4. You may not copy, modify, sublicense, or distribute the Program\r\n",
                "except as expressly provided under this License.  Any attempt\r\n",
                "otherwise to copy, modify, sublicense or distribute the Program is\r\n",
                "void, and will automatically terminate your rights under this License.\r\n",
                "However, parties who have received copies, or rights, from you under\r\n",
                "this License will not have their licenses terminated so long as such\r\n",
                "parties remain in full compliance.\r\n",
                "\r\n",
                "  5. You are not required to accept this License, since you have not\r\n",
                "signed it.  However, nothing else grants you permission to modify or\r\n",
                "distribute the Program or its derivative works.  These actions are\r\n",
                "prohibited by law if you do not accept this License.  Therefore, by\r\n",
                "modifying or distributing the Program (or any work based on the\r\n",
                "Program), you indicate your acceptance of this License to do so, and\r\n",
                "all its terms and conditions for copying, distributing or modifying\r\n",
                "the Program or works based on it.\r\n",
                "\r\n",
                "  6. Each time you redistribute the Program (or any work based on the\r\n",
                "Program), the recipient automatically receives a license from the\r\n",
                "original licensor to copy, distribute or modify the Program subject to\r\n",
                "these terms and conditions.  You may not impose any further\r\n",
                "restrictions on the recipients' exercise of the rights granted herein.\r\n",
                "You are not responsible for enforcing compliance by third parties to\r\n",
                "this License.\r\n",
                "\r\n",
                "  7. If, as a consequence of a court judgment or allegation of patent\r\n",
                "infringement or for any other reason (not limited to patent issues),\r\n",
                "conditions are imposed on you (whether by court order, agreement or\r\n",
                "otherwise) that contradict the conditions of this License, they do not\r\n",
                "excuse you from the conditions of this License.  If you cannot\r\n",
                "distribute so as to satisfy simultaneously your obligations under this\r\n",
                "License and any other pertinent obligations, then as a consequence you\r\n",
                "may not distribute the Program at all.  For example, if a patent\r\n",
                "license would not permit royalty-free redistribution of the Program by\r\n",
                "all those who receive copies directly or indirectly through you, then\r\n",
                "the only way you could satisfy both it and this License would be to\r\n",
                "refrain entirely from distribution of the Program.\r\n",
                "\r\n",
                "If any portion of this section is held invalid or unenforceable under\r\n",
                "any particular circumstance, the balance of the section is intended to\r\n",
                "apply and the section as a whole is intended to apply in other\r\n",
                "circumstances.\r\n",
                "\r\n",
                "It is not the purpose of this section to induce you to infringe any\r\n",
                "patents or other property right claims or to contest validity of any\r\n",
                "such claims; this section has the sole purpose of protecting the\r\n",
                "integrity of the free software distribution system, which is\r\n",
                "implemented by public license practices.  Many people have made\r\n",
                "generous contributions to the wide range of software distributed\r\n",
                "through that system in reliance on consistent application of that\r\n",
                "system; it is up to the author/donor to decide if he or she is willing\r\n",
                "to distribute software through any other system and a licensee cannot\r\n",
                "impose that choice.\r\n",
                "\r\n",
                "This section is intended to make thoroughly clear what is believed to\r\n",
                "be a consequence of the rest of this License.\r\n",
                "\r\n",
                "  8. If the distribution and/or use of the Program is restricted in\r\n",
                "certain countries either by patents or by copyrighted interfaces, the\r\n",
                "original copyright holder who places the Program under this License\r\n",
                "may add an explicit geographical distribution limitation excluding\r\n",
                "those countries, so that distribution is permitted only in or among\r\n",
                "countries not thus excluded.  In such case, this License incorporates\r\n",
                "the limitation as if written in the body of this License.\r\n",
                "\r\n",
                "  9. The Free Software Foundation may publish revised and/or new versions\r\n",
                "of the General Public License from time to time.  Such new versions will\r\n",
                "be similar in spirit to the present version, but may differ in detail to\r\n",
                "address new problems or concerns.\r\n",
                "\r\n",
                "Each version is given a distinguishing version number.  If the Program\r\n",
                "specifies a version number of this License which applies to it and \"any\r\n",
                "later version\", you have the option of following the terms and conditions\r\n",
                "either of that version or of any later version published by the Free\r\n",
                "Software Foundation.  If the Program does not specify a version number of\r\n",
                "this License, you may choose any version ever published by the Free Software\r\n",
                "Foundation.\r\n",
                "\r\n",
                "  10. If you wish to incorporate parts of the Program into other free\r\n",
                "programs whose distribution conditions are different, write to the author\r\n",
                "to ask for permission.  For software which is copyrighted by the Free\r\n",
                "Software Foundation, write to the Free Software Foundation; we sometimes\r\n",
                "make exceptions for this.  Our decision will be guided by the two goals\r\n",
                "of preserving the free status of all derivatives of our free software and\r\n",
                "of promoting the sharing and reuse of software generally.\r\n",
                "\r\n",
                "                            NO WARRANTY\r\n",
                "\r\n",
                "  11. BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\r\n",
                "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\r\n",
                "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\r\n",
                "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\r\n",
                "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\r\n",
                "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\r\n",
                "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\r\n",
                "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\r\n",
                "REPAIR OR CORRECTION.\r\n",
                "\r\n",
                "  12. IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\r\n",
                "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\r\n",
                "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\r\n",
                "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\r\n",
                "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\r\n",
                "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\r\n",
                "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\r\n",
                "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\r\n",
                "POSSIBILITY OF SUCH DAMAGES.\r\n",
                "\r\n", "                     END OF TERMS AND CONDITIONS\r\n",
#ifdef ENABLE_SOCKS
                "\r\n",
                "SOCKS library is covered by the following copyrights:\r\n",
                "\r\n",
                "Copyright (c) 1989 Regents of the University of California.\r\n",
                "All rights reserved.\r\n",
                "	 \r\n",
                "Redistribution and use in source and binary forms, with or without\r\n",
                "modification, are permitted provided that the following conditions\r\n",
                "are met:\r\n",
                "1. Redistributions of source code must retain the above copyright\r\n",
                "   notice, this list of conditions and the following disclaimer.\r\n",
                "2. Redistributions in binary form must reproduce the above copyright\r\n",
                "   notice, this list of conditions and the following disclaimer in the\r\n",
                "   documentation and/or other materials provided with the distribution.\r\n",
                "3. All advertising materials mentioning features or use of this software\r\n",
                "   must display the following acknowledgement:\r\n",
                "        This product includes software developed by the University of\r\n",
                "        California, Berkeley and its contributors.\r\n",
                "4. Neither the name of the University nor the names of its contributors\r\n",
                "   may be used to endorse or promote products derived from this software\r\n",
                "   without specific prior written permission.\r\n",
                "					   \r\n",
                "THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND\r\n",
                "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\r\n",
                "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\r\n",
                "ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE\r\n",
                "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\r\n",
                "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\r\n",
                "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\n",
                "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\r\n",
                "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\r\n",
                "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\r\n",
                "SUCH DAMAGE.\r\n",
                "\r\n",
                "Portions Copyright (c) 1993, 1994, 1995 by NEC Systems Laboratory.\r\n",
                "\r\n",
                "Permission to use, copy, modify, and distribute this software for\r\n",
                "any purpose with or without fee is hereby granted, provided that\r\n",
                "the above copyright notice and this permission notice appear in all\r\n",
                "copies, and that the name of NEC Systems Laboratory not be used in\r\n",
                "advertising or publicity pertaining to distribution of the document\r\n",
                "or software without specific, written prior permission.\r\n",
                "\r\n",
                "THE SOFTWARE IS PROVIDED ``AS IS'' AND NEC SYSTEMS LABORATORY DISCLAIMS\r\n",
                "ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED\r\n",
                "WARRANTIES OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL NEC\r\n",
                "SYSTEMS LABORATORY BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR\r\n",
                "CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS\r\n",
                "OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE\r\n",
                "OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE\r\n",
                "OR PERFORMANCE OF THIS SOFTWARE.\r\n",
#endif /* ENABLE_SOCKS */
                NULL);
}


void
warranty (void)
{
    std_printf ("Warranty\r\n\n");

    feed_pager (3,
                "BECAUSE THE PROGRAM IS LICENSED FREE OF CHARGE, THERE IS NO WARRANTY\r\n",
                "FOR THE PROGRAM, TO THE EXTENT PERMITTED BY APPLICABLE LAW.  EXCEPT WHEN\r\n",
                "OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR OTHER PARTIES\r\n",
                "PROVIDE THE PROGRAM \"AS IS\" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED\r\n",
                "OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\r\n",
                "MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS\r\n",
                "TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU.  SHOULD THE\r\n",
                "PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,\r\n",
                "REPAIR OR CORRECTION.\r\n",
                "\r\n",
                "IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN WRITING\r\n",
                "WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY AND/OR\r\n",
                "REDISTRIBUTE THE PROGRAM AS PERMITTED ABOVE, BE LIABLE TO YOU FOR DAMAGES,\r\n",
                "INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING\r\n",
                "OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT NOT LIMITED\r\n",
                "TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY\r\n",
                "YOU OR THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER\r\n",
                "PROGRAMS), EVEN IF SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE\r\n",
                "POSSIBILITY OF SUCH DAMAGES.\r\n",
#ifdef ENABLE_SOCKS
                "\r\n",
                "THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND\r\n",
                "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\r\n",
                "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\r\n",
                "ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE\r\n",
                "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\r\n",
                "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\r\n",
                "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\r\n",
                "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT\r\n",
                "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY\r\n",
                "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF\r\n",
                "SUCH DAMAGE.\r\n",
                "\r\n",
                "THE SOFTWARE IS PROVIDED ``AS IS'' AND NEC SYSTEMS LABORATORY DISCLAIMS\r\n",
                "ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED\r\n",
                "WARRANTIES OF MERCHANTABILITY AND FITNESS.   IN NO EVENT SHALL NEC\r\n",
                "SYSTEMS LABORATORY BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR\r\n",
                "CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS\r\n",
                "OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE\r\n",
                "OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE\r\n",
                "OR PERFORMANCE OF THIS SOFTWARE.\r\n",
#endif
                NULL);
}


/*
 * information() displays general info about the client
 */
void
information (void)
{
    char    c;
    int     invalid = 0;

    std_printf ("Information\r\n");

    for (;;) {
        if (flags.useansi)
            colorize
                ("\r\n@YC@Copyright  @YL@Cicense  @YW@Carranty  @YT@Cechnical  @YQ@Cuit\r\n@YClient information -> @G");
        else
            std_printf ("\r\n<C>opyright <L>icense <W>arranty  <T>echnical <Q>uit\r\nClient information -> ");
        for (invalid = 0;;) {
            c = inkey ();
            if (!mystrchr ("CcLlOoWwTtQq \n", c)) {
                if (invalid++)
                    flush_input (invalid);
                continue;
            }
            break;
        }
        switch (c) {
        case 'c':
        case 'C':
            copyright ();
            break;
        case 'l':
        case 'L':
            license ();
            break;
        case 'o':
        case 'O':
            otherinfo ();
            break;
        case 't':
        case 'T':
            techinfo ();
            break;
        case 'w':
        case 'W':
            warranty ();
            break;
        case 'q':
        case 'Q':
        case ' ':
        case '\n':
            std_printf ("Quit\r\n");
            return;
            /* NOTREACHED */
        default:
            break;
        }
    }
}

void
otherinfo (void)
{
    /* You shouldn't be reading this... */

    /* I mean it!  Stop reading now! */

    /* Dammit, you're going to ruin all my fun! */

    /* Oh well, if you insist. */

    feed_pager (1,
                "\r\n",
                "\r\n",
                "Important things to remember about ISCA BBS:\r\n",
                "\r\n",
                "1.  The content in Babble> strictly follows the Webster's dictionary\r\n",
                "    definition.\r\n",
                "2.  Regarding ISCA BBS women, the odds may be good, but the goods may\r\n",
                "    be odd.\r\n",
                "3.  Houston used to be gay.\r\n",
                "4.  Non Sequitur> usually isn't.  And neither was Weird>.\r\n",
                "5.  To get into Kama Sutra>, you have to be 18 or have a good fake ID.\r\n",
                "6.  If you make your information public, God Man Megatron will post it\r\n",
                "    on the Web within 24 hours.  Guaranteed.\r\n",
                "7.  The Policy Board is your friend.  The Policy Board has always been\r\n",
                "    your friend.  The Policy Board will always be your friend.\r\n",
                "8.  The BBS is never moving to Gestalt.\r\n",
                "9.  The performers may change, but the song is always the same.\r\n", NULL);
}
