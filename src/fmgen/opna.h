// ---------------------------------------------------------------------------
//	OPN/A/B interface with ADPCM support
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opna.h,v 1.33 2003/06/12 13:14:37 cisc Exp $

#ifndef FM_OPNA_H
#define FM_OPNA_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPN/OPNA
//	OPN/OPNA $B$KNI$/;w$?2;$r@8@.$9$k2;8;%f%K%C%H(B
//	
//	interface:
//	bool Init(uint clock, uint rate, bool, const char* path);
//		$B=i4|2=!%$3$N%/%i%9$r;HMQ$9$kA0$K$+$J$i$:8F$s$G$*$/$3$H!%(B
//		OPNA $B$N>l9g$O$3$N4X?t$G%j%:%`%5%s%W%k$rFI$_9~$`(B
//
//		clock:	OPN/OPNA/OPNB $B$N%/%m%C%/<~GH?t(B(Hz)
//
//		rate:	$B@8@.$9$k(B PCM $B$NI8K\<~GH?t(B(Hz)
//
//		path:	$B%j%:%`%5%s%W%k$N%Q%9(B(OPNA $B$N$_M-8z(B)
//				$B>JN,;~$O%+%l%s%H%G%#%l%/%H%j$+$iFI$_9~$`(B
//				$BJ8;zNs$NKvHx$K$O(B '\' $B$d(B '/' $B$J$I$r$D$1$k$3$H(B
//
//		$BJV$jCM(B	$B=i4|2=$K@.8y$9$l$P(B true
//
//	bool LoadRhythmSample(const char* path)
//		(OPNA ONLY)
//		Rhythm $B%5%s%W%k$rFI$_D>$9!%(B
//		path $B$O(B Init $B$N(B path $B$HF1$8!%(B
//		
//	bool SetRate(uint clock, uint rate, bool)
//		$B%/%m%C%/$d(B PCM $B%l!<%H$rJQ99$9$k(B
//		$B0z?tEy$O(B Init $B$r;2>H$N$3$H!%(B
//	
//	void Mix(FM_SAMPLETYPE* dest, int nsamples)
//		Stereo PCM $B%G!<%?$r(B nsamples $BJ,9g@.$7!$(B dest $B$G;O$^$kG[Ns$K(B
//		$B2C$($k(B($B2C;;$9$k(B)
//		$B!&(Bdest $B$K$O(B sample*2 $B8DJ,$NNN0h$,I,MW(B
//		$B!&3JG<7A<0$O(B L, R, L, R... $B$H$J$k!%(B
//		$B!&$"$/$^$G2C;;$J$N$G!$$"$i$+$8$aG[Ns$r%<%m%/%j%"$9$kI,MW$,$"$k(B
//		$B!&(BFM_SAMPLETYPE $B$,(B short $B7?$N>l9g%/%j%C%T%s%0$,9T$o$l$k(B.
//		$B!&$3$N4X?t$O2;8;FbIt$N%?%$%^!<$H$OFHN)$7$F$$$k!%(B
//		  Timer $B$O(B Count $B$H(B GetNextEvent $B$GA`:n$9$kI,MW$,$"$k!%(B
//	
//	void Reset()
//		$B2;8;$r%j%;%C%H(B($B=i4|2=(B)$B$9$k(B
//
//	void SetReg(uint reg, uint data)
//		$B2;8;$N%l%8%9%?(B reg $B$K(B data $B$r=q$-9~$`(B
//	
//	uint GetReg(uint reg)
//		$B2;8;$N%l%8%9%?(B reg $B$NFbMF$rFI$_=P$9(B
//		$BFI$_9~$`$3$H$,=PMh$k%l%8%9%?$O(B PSG, ADPCM $B$N0lIt!$(BID(0xff) $B$H$+(B
//	
//	uint ReadStatus()/ReadStatusEx()
//		$B2;8;$N%9%F!<%?%9%l%8%9%?$rFI$_=P$9(B
//		ReadStatusEx $B$O3HD%%9%F!<%?%9%l%8%9%?$NFI$_=P$7(B(OPNA)
//		busy $B%U%i%0$O>o$K(B 0
//	
//	bool Count(uint32 t)
//		$B2;8;$N%?%$%^!<$r(B t [$B&LIC(B] $B?J$a$k!%(B
//		$B2;8;$NFbIt>uBV$KJQ2=$,$"$C$?;~(B(timer $B%*!<%P!<%U%m!<(B)
//		true $B$rJV$9(B
//
//	uint32 GetNextEvent()
//		$B2;8;$N%?%$%^!<$N$I$A$i$+$,%*!<%P!<%U%m!<$9$k$^$G$KI,MW$J(B
//		$B;~4V(B[$B&LIC(B]$B$rJV$9(B
//		$B%?%$%^!<$,Dd;_$7$F$$$k>l9g$O(B ULONG_MAX $B$rJV$9!D(B $B$H;W$&(B
//	
//	void SetVolumeFM(int db)/SetVolumePSG(int db) ...
//		$B3F2;8;$N2;NL$r!\!]J}8~$KD4@a$9$k!%I8=`CM$O(B 0.
//		$BC10L$OLs(B 1/2 dB$B!$M-8zHO0O$N>e8B$O(B 20 (10dB)
//
namespace FM
{
	//	OPN Base -------------------------------------------------------
	class OPNBase : public Timer
	{
	public:
		OPNBase();
		
		bool	Init(uint c, uint r);
		virtual void Reset();
		
		void	SetVolumeFM(int db);
		void	SetVolumePSG(int db);
		void	SetLPFCutoff(uint freq) {}	// obsolete

	protected:
		void	SetParameter(Channel4* ch, uint addr, uint data);
		void	SetPrescaler(uint p);
		void	RebuildTimeTable();
		
		int		fmvolume;
		
		uint	clock;				// OPN $B%/%m%C%/(B
		uint	rate;				// FM $B2;8;9g@.%l!<%H(B
		uint	psgrate;			// FMGen  $B=PNO%l!<%H(B
		uint	status;
		Channel4* csmch;
		

		static  uint32 lfotable[8];
	
	private:
		void	TimerA();
		uint8	prescale;
		
	protected:
		Chip	chip;
		PSG		psg;
	};

	//	OPN2 Base ------------------------------------------------------
	class OPNABase : public OPNBase
	{
	public:
		OPNABase();
		~OPNABase();
		
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx();
		void	SetChannelMask(uint mask);
	
	private:
		virtual void Intr(bool) {}

		void	MakeTable2();
	
	protected:
		bool	Init(uint c, uint r, bool);
		bool	SetRate(uint c, uint r, bool);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		void	SetADPCMBReg(uint reg, uint data);
		uint	GetReg(uint addr);	
	
	protected:
		void	FMMix(Sample* buffer, int nsamples);
		void 	Mix6(Sample* buffer, int nsamples, int activech);
		
		void	MixSubS(int activech, ISample**);
		void	MixSubSL(int activech, ISample**);

		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	UpdateStatus();
		void	LFO();

		void	DecodeADPCMB();
		void	ADPCMBMix(Sample* dest, uint count);

		void	WriteRAM(uint data);
		uint	ReadRAM();
		int		ReadRAMN();
		int		DecodeADPCMBSample(uint);
		
	// FM $B2;8;4X78(B
		uint8	pan[6];
		uint8	fnum2[9];
		
		uint8	reg22;
		uint	reg29;		// OPNA only?
		
		uint	stmask;
		uint	statusnext;

		uint32	lfocount;
		uint32	lfodcount;
		
		uint	fnum[6];
		uint	fnum3[3];
		
	// ADPCM $B4X78(B
		uint8*	adpcmbuf;		// ADPCM RAM
		uint	adpcmmask;		// $B%a%b%j%"%I%l%9$KBP$9$k%S%C%H%^%9%/(B
		uint	adpcmnotice;	// ADPCM $B:F@8=*N;;~$K$?$D%S%C%H(B
		uint	startaddr;		// Start address
		uint	stopaddr;		// Stop address
		uint	memaddr;		// $B:F@8Cf%"%I%l%9(B
		uint	limitaddr;		// Limit address/mask
		int		adpcmlevel;		// ADPCM $B2;NL(B
		int		adpcmvolume;
		int		adpcmvol;
		uint	deltan;			// $B-y(BN
		int		adplc;			// $B<~GH?tJQ49MQJQ?t(B
		int		adpld;			// $B<~GH?tJQ49MQJQ?t:9J,CM(B
		uint	adplbase;		// adpld $B$N85(B
		int		adpcmx;			// ADPCM $B9g@.MQ(B x
		int		adpcmd;			// ADPCM $B9g@.MQ(B $B-y(B
		int		adpcmout;		// ADPCM $B9g@.8e$N=PNO(B
		int		apout0;			// out(t-2)+out(t-1)
		int		apout1;			// out(t-1)+out(t)

		uint	adpcmreadbuf;	// ADPCM $B%j!<%IMQ%P%C%U%!(B
		bool	adpcmplay;		// ADPCM $B:F@8Cf(B
		int8	granuality;		
		bool	adpcmmask_;

		uint8	control1;		// ADPCM $B%3%s%H%m!<%k%l%8%9%?#1(B
		uint8	control2;		// ADPCM $B%3%s%H%m!<%k%l%8%9%?#2(B
		uint8	adpcmreg[8];	// ADPCM $B%l%8%9%?$N0lItJ,(B

		int		rhythmmask_;

		Channel4 ch[6];

		static void	BuildLFOTable();
		static int amtable[FM_LFOENTS];
		static int pmtable[FM_LFOENTS];
		static int32 tltable[FM_TLENTS+FM_TLPOS];
		static bool	tablehasmade;
	};

	//	YM2203(OPN) ----------------------------------------------------
	class OPN : public OPNBase
	{
	public:
		OPN();
		virtual ~OPN() {}
		
		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool=false);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint mask);
		
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }
	
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		
		uint	fnum[3];
		uint	fnum3[3];
		uint8	fnum2[6];
		
		Channel4 ch[3];
	};

	//	YM2608(OPNA) ---------------------------------------------------
	class OPNA : public OPNABase
	{
	public:
		OPNA();
		virtual ~OPNA();
		
		bool	Init(uint c, uint r, bool  = false, const char* rhythmpath=0);
		bool	LoadRhythmSample(const char*);
	
		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);

		void	SetVolumeADPCM(int db);
		void	SetVolumeRhythmTotal(int db);
		void	SetVolumeRhythm(int index, int db);

		uint8*	GetADPCMBuffer() { return adpcmbuf; }

		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		int		dbgGetPGOut(int c, int s) { return ch[c].op[s].dbgpgout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

		
	private:
		struct Rhythm
		{
			uint8	pan;		// $B$Q$s(B
			int8	level;		// $B$*$s$j$g$&(B
			int		volume;		// $B$*$s$j$g$&$;$C$F$$(B
			int16*	sample;		// $B$5$s$W$k(B
			uint	size;		// $B$5$$$:(B
			uint	pos;		// $B$$$A(B
			uint	step;		// $B$9$F$C$W$A(B
			uint	rate;		// $B$5$s$W$k$N$l!<$H(B
		};
	
		void	RhythmMix(Sample* buffer, uint count);

	// $B%j%:%`2;8;4X78(B
		Rhythm	rhythm[6];
		int8	rhythmtl;		// $B%j%:%`A4BN$N2;NL(B
		int		rhythmtvol;		
		uint8	rhythmkey;		// $B%j%:%`$N%-!<(B
	};

	//	YM2610/B(OPNB) ---------------------------------------------------
	class OPNB : public OPNABase
	{
	public:
		OPNB();
		virtual ~OPNB();
		
		bool	Init(uint c, uint r, bool = false,
					 uint8 *_adpcma = 0, int _adpcma_size = 0,
					 uint8 *_adpcmb = 0, int _adpcmb_size = 0);
	
		bool	SetRate(uint c, uint r, bool = false);
		void 	Mix(Sample* buffer, int nsamples);

		void	Reset();
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatusEx();

		void	SetVolumeADPCMATotal(int db);
		void	SetVolumeADPCMA(int index, int db);
		void	SetVolumeADPCMB(int db);

//		void	SetChannelMask(uint mask);
		
	private:
		struct ADPCMA
		{
			uint8	pan;		// $B$Q$s(B
			int8	level;		// $B$*$s$j$g$&(B
			int		volume;		// $B$*$s$j$g$&$;$C$F$$(B
			uint	pos;		// $B$$$A(B
			uint	step;		// $B$9$F$C$W$A(B

			uint	start;		// $B3+;O(B
			uint	stop;		// $B=*N;(B
			uint	nibble;		// $B<!$N(B 4 bit
			int		adpcmx;		// $BJQ49MQ(B
			int		adpcmd;		// $BJQ49MQ(B
		};
	
		int		DecodeADPCMASample(uint);
		void	ADPCMAMix(Sample* buffer, uint count);
		static void InitADPCMATable();
		
	// ADPCMA $B4X78(B
		uint8*	adpcmabuf;		// ADPCMA ROM
		int		adpcmasize;
		ADPCMA	adpcma[6];
		int8	adpcmatl;		// ADPCMA $BA4BN$N2;NL(B
		int		adpcmatvol;		
		uint8	adpcmakey;		// ADPCMA $B$N%-!<(B
		int		adpcmastep;
		uint8	adpcmareg[32];
 
		static int jedi_table[(48+1)*16];

		Channel4 ch[6];
	};

	//	YM2612/3438(OPN2) ----------------------------------------------------
	class OPN2 : public OPNBase
	{
	public:
		OPN2();
		virtual ~OPN2() {}
		
		bool	Init(uint c, uint r, bool=false, const char* =0);
		bool	SetRate(uint c, uint r, bool);
		
		void	Reset();
		void 	Mix(Sample* buffer, int nsamples);
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		uint	ReadStatusEx() { return 0xff; }
		
		void	SetChannelMask(uint mask);
		
	private:
		virtual void Intr(bool) {}
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		
		uint	fnum[3];
		uint	fnum3[3];
		uint8	fnum2[6];
		
	// $B@~7AJd4VMQ%o!<%/(B
		int32	mixc, mixc1;
		
		Channel4 ch[3];
	};
}

// ---------------------------------------------------------------------------

inline void FM::OPNBase::RebuildTimeTable()
{
	int p = prescale;
	prescale = -1;
	SetPrescaler(p);
}

inline void FM::OPNBase::SetVolumePSG(int db)
{
	psg.SetVolume(db);
}

#endif // FM_OPNA_H
