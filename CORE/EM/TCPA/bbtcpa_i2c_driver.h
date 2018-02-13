#define BIT0	(1<<0)
#define BIT1	(1<<1)
#define BIT2	(1<<2)
#define BIT3	(1<<3)
#define BIT4	(1<<4)
#define BIT5	(1<<5)
#define BIT6	(1<<6)
#define BIT7	(1<<7)
#define BIT8	(1<<8)
#define BIT9	(1<<9)
#define BIT10	(1<<10)
#define BIT11	(1<<11)
#define BIT12	(1<<12)
#define BIT13	(1<<13)
#define BIT14	(1<<14)
#define BIT15	(1<<15)
#define BIT16	(1<<16)
#define BIT17	(1<<17)
#define BIT18	(1<<18)
#define BIT19	(1<<19)
#define BIT20	(1<<20)
#define BIT21	(1<<21)
#define BIT22	(1<<22)
#define BIT23	(1<<23)
#define BIT24	(1<<24)
#define BIT25	(1<<25)
#define BIT26	(1<<26)
#define BIT27	(1<<27)
#define BIT28	(1<<28)
#define BIT29	(1<<29)
#define BIT30	(1<<30)
#define BIT31	(1<<31)

typedef unsigned char BYTE;
typedef unsigned int WORD;
typedef	unsigned long	DWORD;

typedef enum
{
  AccWidthUint8 = 0,
  AccWidthUint16,
  AccWidthUint32,
} ACC_WIDTH;

extern void out80p(BYTE bCode);

	BYTE	ReadIO8 (WORD Address);
	WORD	ReadIO16 (WORD Address);
	DWORD	ReadIO32 (WORD Address);
	void	WriteIO8 (WORD Address, BYTE Data);
	void	WriteIO16 (WORD Address, WORD Data);
	void	WriteIO32 (WORD Address, DWORD Data);

	BYTE	ReadMEM8(DWORD Address);
	WORD	ReadMEM16(DWORD Address);
	DWORD	ReadMEM32(DWORD Address);
	void	WriteMEM8(DWORD Address, BYTE Data);
	void	WriteMEM16(DWORD Address, WORD Data);
	void	WriteMEM32(DWORD Address, DWORD Data);

void	ReadIO (WORD  Address, WORD OpFlag, void *Value);
void	WriteIO (WORD Address, WORD OpFlag, void *Value);
void	ReadMEM(DWORD  Address, WORD OpFlag, void *Value);
void	WriteMEM(DWORD  Address, WORD OpFlag, void *Value);
void	dbg80_delay(WORD OpFlag, void* Value);
void	atom_delay();
void	send_al_to_COM_port(BYTE bCode);
void	send_al_ascii_to_COM_port(BYTE bCode);
