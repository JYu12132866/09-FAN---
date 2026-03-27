
#ifndef	_TYPE_H
#define	_TYPE_H

/*+-----------------------------------------------------------------------------+*/
/*|	ƒf[ƒ^Œ^‚Ì’è‹`								|*/
/*+-----------------------------------------------------------------------------+*/
//typedef	unsigned char	uchar;
//typedef	unsigned int16_t	uint;
//typedef	unsigned short	ushort;
//typedef	unsigned long	ulong;
#define	swap( a, b) ( a ^= b, b ^= a, a ^= b)	/* ’l‚C‚‚‚Ì“ü‘Öƒ}ƒNƒ		*/
#define	abs(x)	((x) < 0 ? -(x) : (x))		/* ‚˜‚Ìâ‘Î’l‚ğ‹‚ß‚éƒ}ƒNƒ	*/
#ifndef	max
#define	max( a, b)	((a)>(b) ? (a) : (b))
#endif
#ifndef	min
#define	min( a, b)	((a)<=(b) ? (a) : (b))
#endif

/*+-----------------------------------------------------------------------------+*/
/*|	˜_—’l‚Ì’è‹`								|*/
/*+-----------------------------------------------------------------------------+*/
#define		ON		1
#define		OFF		0
#ifndef		TRUE
#define		TRUE		(1 == 1)
#endif
#ifndef		FALSE
#define		FALSE		(!TRUE)
#endif
#define		OK		1
#define		NG		(~OK)

/*+-----------------------------------------------------------------------------+*/
/*|	‚`‚r‚b‚h‚h•¶š‚Ì’è‹`							|*/
/*+-----------------------------------------------------------------------------+*/
#ifndef		NULL
#define		NULL		0		/* ‚m‚t‚k‚k			*/
#endif
#define		FF		0x0c		/* ‚e‚e				*/
#define		ESC		0x1b		/* ‚d‚r‚b			*/
#define		ACK		0x06		/* ‚`‚b‚j			*/
#define		NAK		0x15		/* ‚m‚`‚j			*/
#define		STX		0x02		/* ‚r‚s‚w			*/
#define		ETX		0x03		/* ‚d‚s‚w			*/


#define		BYTE_1		1		/* ‚PƒoƒCƒg			*/
#define		BYTE_2		2		/* ‚QƒoƒCƒg			*/
#define		BYTE_3		3		/* ‚RƒoƒCƒg			*/
#define		BYTE_4		4		/* ‚SƒoƒCƒg			*/

#endif