/*
 * This is the file for I/O defines, different systems do certain things
 * differently, such system-specific stuff should be put here.
 */

#define INPUT_LEFT(__fp) (ptyifp - ptyibuf < ptyilen)
#define ptyget()	  (INPUT_LEFT(stdin) ? *ptyifp++ : ((ptyilen = read(0, ptyibuf, sizeof ptyibuf)) < 0 ? -1 : ((ptyifp = ptyibuf), *ptyifp++)))

#define NET_INPUT_LEFT() (netifp - netibuf < netilen)
#define netget()         (NET_INPUT_LEFT() ? *netifp++ : ((netilen = read(net, netibuf, sizeof netibuf)) <= 0 ? -1 : ((netifp = netibuf), *netifp++)))
#define netput(__c)      (putc(__c, netofp))
#define netflush()	  (fflush(netofp))
