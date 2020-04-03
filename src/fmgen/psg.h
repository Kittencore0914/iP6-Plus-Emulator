// ---------------------------------------------------------------------------
//	PSG-like sound generator
//	Copyright (C) cisc 1997, 1999.
// ---------------------------------------------------------------------------
//	$Id: psg.h,v 1.8 2003/04/22 13:12:53 cisc Exp $

#ifndef PSG_H
#define PSG_H

#define PSG_SAMPLETYPE		int32		// int32 or int16


// ---------------------------------------------------------------------------
//	class PSG
//	PSG $B$KNI$/;w$?2;$r@8@.$9$k2;8;%f%K%C%H(B
//	
//	interface:
//	bool SetClock(uint clock, uint rate)
//		$B=i4|2=!%$3$N%/%i%9$r;HMQ$9$kA0$K$+$J$i$:8F$s$G$*$/$3$H!%(B
//		PSG $B$N%/%m%C%/$d(B PCM $B%l!<%H$r@_Dj$9$k(B
//
//		clock:	PSG $B$NF0:n%/%m%C%/(B
//		rate:	$B@8@.$9$k(B PCM $B$N%l!<%H(B
//		retval	$B=i4|2=$K@.8y$9$l$P(B true
//
//	void Mix(Sample* dest, int nsamples)
//		PCM $B$r(B nsamples $BJ,9g@.$7!$(B dest $B$G;O$^$kG[Ns$K2C$($k(B($B2C;;$9$k(B)
//		$B$"$/$^$G2C;;$J$N$G!$:G=i$KG[Ns$r%<%m%/%j%"$9$kI,MW$,$"$k(B
//	
//	void Reset()
//		$B%j%;%C%H$9$k(B
//
//	void SetReg(uint reg, uint8 data)
//		$B%l%8%9%?(B reg $B$K(B data $B$r=q$-9~$`(B
//	
//	uint GetReg(uint reg)
//		$B%l%8%9%?(B reg $B$NFbMF$rFI$_=P$9(B
//	
//	void SetVolume(int db)
//		$B3F2;8;$N2;NL$rD4@a$9$k(B
//		$BC10L$OLs(B 1/2 dB
//
class PSG
{
public:
	typedef PSG_SAMPLETYPE Sample;
	
	enum
	{
		noisetablesize = 1 << 11,	// $B"+%a%b%j;HMQNL$r8:$i$7$?$$$J$i8:$i$7$F(B
		toneshift = 24,
		envshift = 22,
		noiseshift = 14,
		oversampling = 2,		// $B"+(B $B2;<A$h$jB.EY$,M%@h$J$i8:$i$9$H$$$$$+$b(B
	};

public:
	PSG();
	~PSG();

	void Mix(Sample* dest, int nsamples);
	void SetClock(int clock, int rate);
	
	void SetVolume(int vol);
	void SetChannelMask(int c);
	
	void Reset();
	void SetReg(uint regnum, uint8 data);
	uint GetReg(uint regnum) { return reg[regnum & 0x0f]; }

protected:
	void MakeNoiseTable();
	void MakeEnvelopTable();
	static void StoreSample(Sample& dest, int32 data);
	
	uint8 reg[16];

	const uint* envelop;
	uint olevel[3];
	uint32 scount[3], speriod[3];
	uint32 ecount, eperiod;
	uint32 ncount, nperiod;
	uint32 tperiodbase;
	uint32 eperiodbase;
	uint32 nperiodbase;
	int volume;
	int mask;

	static uint enveloptable[16][64];
	static uint noisetable[noisetablesize];
	static int EmitTable[32];
};

#endif // PSG_H
