// ---------------------------------------------------------------------------
//	OPM-like Sound Generator
//	Copyright (C) cisc 1998, 2003.
// ---------------------------------------------------------------------------
//	$Id: opm.h,v 1.14 2003/06/07 08:25:53 cisc Exp $

#ifndef FM_OPM_H
#define FM_OPM_H

#include "fmgen.h"
#include "fmtimer.h"
#include "psg.h"

// ---------------------------------------------------------------------------
//	class OPM
//	OPM $B$KNI$/;w$?(B(?)$B2;$r@8@.$9$k2;8;%f%K%C%H(B
//	
//	interface:
//	bool Init(uint clock, uint rate, bool);
//		$B=i4|2=!%$3$N%/%i%9$r;HMQ$9$kA0$K$+$J$i$:8F$s$G$*$/$3$H!%(B
//		$BCm0U(B: $B@~7AJd40%b!<%I$OGQ;_$5$l$^$7$?(B
//
//		clock:	OPM $B$N%/%m%C%/<~GH?t(B(Hz)
//
//		rate:	$B@8@.$9$k(B PCM $B$NI8K\<~GH?t(B(Hz)
//
//				
//		$BJVCM(B	$B=i4|2=$K@.8y$9$l$P(B true
//
//	bool SetRate(uint clock, uint rate, bool)
//		$B%/%m%C%/$d(B PCM $B%l!<%H$rJQ99$9$k(B
//		$B0z?tEy$O(B Init $B$HF1MM!%(B
//	
//	void Mix(Sample* dest, int nsamples)
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
//	uint ReadStatus()
//		$B2;8;$N%9%F!<%?%9%l%8%9%?$rFI$_=P$9(B
//		busy $B%U%i%0$O>o$K(B 0
//	
//	bool Count(uint32 t)
//		$B2;8;$N%?%$%^!<$r(B t [10^(-6) $BIC(B] $B?J$a$k!%(B
//		$B2;8;$NFbIt>uBV$KJQ2=$,$"$C$?;~(B(timer $B%*!<%P!<%U%m!<(B)
//		true $B$rJV$9(B
//
//	uint32 GetNextEvent()
//		$B2;8;$N%?%$%^!<$N$I$A$i$+$,%*!<%P!<%U%m!<$9$k$^$G$KI,MW$J(B
//		$B;~4V(B[$B&LIC(B]$B$rJV$9(B
//		$B%?%$%^!<$,Dd;_$7$F$$$k>l9g$O(B 0 $B$rJV$9!%(B
//	
//	void SetVolume(int db)
//		$B3F2;8;$N2;NL$r!\!]J}8~$KD4@a$9$k!%I8=`CM$O(B 0.
//		$BC10L$OLs(B 1/2 dB$B!$M-8zHO0O$N>e8B$O(B 20 (10dB)
//
//	$B2>A[4X?t(B:
//	virtual void Intr(bool irq)
//		IRQ $B=PNO$KJQ2=$,$"$C$?>l9g8F$P$l$k!%(B
//		irq = true:  IRQ $BMW5a$,H/@8(B
//		irq = false: IRQ $BMW5a$,>C$($k(B
//
namespace FM
{
	//	YM2151(OPM) ----------------------------------------------------
	class OPM : public Timer
	{
	public:
		OPM();
		~OPM() {}

		bool	Init(uint c, uint r, bool=false);
		bool	SetRate(uint c, uint r, bool);
		void	SetLPFCutoff(uint freq);
		void	Reset();
		
		void 	SetReg(uint addr, uint data);
		uint	GetReg(uint addr);
		uint	ReadStatus() { return status & 0x03; }
		
		void 	Mix(Sample* buffer, int nsamples);
		
		void	SetVolume(int db);
		void	SetChannelMask(uint mask);
		
	private:
		virtual void Intr(bool) {}
	
	private:
		enum
		{
			OPM_LFOENTS = 512,
		};
		
		void	SetStatus(uint bit);
		void	ResetStatus(uint bit);
		void	SetParameter(uint addr, uint data);
		void	TimerA();
		void	RebuildTimeTable();
		void	MixSub(int activech, ISample**);
		void	MixSubL(int activech, ISample**);
		void	LFO();
		uint	Noise();
		
		int		fmvolume;

		uint	clock;
		uint	rate;
		uint	pcmrate;

		uint	pmd;
		uint	amd;
		uint	lfocount;
		uint	lfodcount;

		uint	lfo_count_;
		uint	lfo_count_diff_;
		uint	lfo_step_;
		uint	lfo_count_prev_;

		uint	lfowaveform;
		uint	rateratio;
		uint	noise;
		int32	noisecount;
		uint32	noisedelta;
		
		bool	interpolation;
		uint8	lfofreq;
		uint8	status;
		uint8	reg01;

		uint8	kc[8];
		uint8	kf[8];
		uint8	pan[8];

		Channel4 ch[8];
		Chip	chip;

		static void	BuildLFOTable();
		static int amtable[4][OPM_LFOENTS];
		static int pmtable[4][OPM_LFOENTS];

	public:
		int		dbgGetOpOut(int c, int s) { return ch[c].op[s].dbgopout_; }
		Channel4* dbgGetCh(int c) { return &ch[c]; }

	};
}

#endif // FM_OPM_H
