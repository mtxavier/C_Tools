#include <StackEnv.h>
#include <Classes.h>
#include <Browser.h>
#include <stdio.h>

void Brw100Test(const char *Title,BrwRSerial *s) {
	unsigned int Buff[100],*c,*e;
	unsigned char sBuff[2000],*q;
	const unsigned char *p;
	e = Call(s,Read,2(Buff,Buff+100));
	q = sBuff; p = Title;
	while (*p) { *q++=*p++; } 
	c = Buff; 
	while (c<e) {
		sprintf(q," %d",*c);
		c++; 
		while (*q) *q++;
	}
	*q++ = '\n'; *q = 0;
	printf(sBuff);
}

main() {
	unsigned char shuffle36[36],shuffle144[144];
	BrwRSerial *rand0,*rand6,*rand12,*srand6,*srand12;
	EnvOpen(4096,4096);
	rand0 = BrwSerialRandom(BrwRandomSeed());
	rand6 = BrwCappedDice(6,256,rand0);
	rand12 = BrwCappedDice(12,256,rand0);
    {
		unsigned char *c,*e,i;
		c = shuffle36; e = c+36; i = 1;
		while (c<e) { *c++=i; i++; if (i>6) i=1; }
		srand6 = BrwShuffledDice(shuffle36,shuffle36+36,256,rand0);
		c = shuffle144; e = c+144; i = 1;
		while (c<e) { *c++=i; i++; if (i>12) i=1; }
		srand12 = BrwShuffledDice(shuffle144,shuffle144+144,256,rand0);
	}
	Brw100Test("D256:",rand0);
    Brw100Test("D6:",rand6);
    Brw100Test("D12:",rand12);
    Brw100Test("Shuffle 6:",srand6);
    Brw100Test("Shuffle 12:",srand12);
	EnvClose();
}


