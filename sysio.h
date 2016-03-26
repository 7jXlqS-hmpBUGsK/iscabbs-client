/*
 * This is the file for I/O defines, different systems do certain things
 * differently, such system-specific stuff should be put here.
 */

#define INPUT_LEFT(__fp) (ptyifp - ptyibuf < ptyilen)
#define ptyget()	  (INPUT_LEFT(stdin) ? *ptyifp++ : ((ptyilen = read(0, ptyibuf, sizeof ptyibuf)) < 0 ? -1 : ((ptyifp = ptyibuf), *ptyifp++)))

#define NET_INPUT_LEFT() (netifp - netibuf < netilen)
extern int netget (void);
extern void netflush (void);
