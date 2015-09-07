#include "pthread.h"
#include "semaphore.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>

#include "aes256.h"
/*===============import from melp_ana==============================*/

#include "sc1200.h"
#include "mathhalf.h"
#include "macro.h"
#include "lpc_lib.h"
#include "mat_lib.h"
#include "vq_lib.h"
#include "fs_lib.h"
#include "fft_lib.h"
#include "pit_lib.h"
#include "math_lib.h"
#include "constant.h"
#include "cprv.h"
#include "global.h"
#include "pitch.h"
#include "qnt12_cb.h"
#include "qnt12.h"
#include "msvq_cb.h"
#include "fsvq_cb.h"
#include "melp_sub.h"
#include "dsp_sub.h"
#include "coeff.h"
#if NPP
#include "npp.h"
#endif

/* compiler constants */

#define BWFACT_Q15			32571

#define HF_CORR_Q15			4	/* 0.0001220703125 in Q15 */

#define PEAK_THRESH_Q12		5488	/* 1.34 * (1 << 12) */
#define PEAK_THR2_Q12		6553	/* 1.6 * (1 << 12) */
#define SILENCE_DB_Q8		7680	/* 30.0 * (1 << 8) */
#define SIG_LENGTH			(LPF_ORD + PITCH_FR)	/* 327 */

#define X01_Q15				3277	/* 0.1 * (1 << 15) */
#define X03_Q11				614	/* 0.3 * (1 << 11) */
#define X04_Q14				6554	/* 0.4 * (1 << 14) */
#define X045_Q14			7273	/* 0.45 * (1 << 14) */
#define X05_Q11				1024	/* 0.5 * (1 << 11) */
#define X07_Q14				11469	/* 0.7 * (1 << 11) */
#define X08_Q11				1638	/* 0.8 * (1 << 11) */
#define X12_Q11				2458	/* 1.2 * (1 << 11) */
#define X28_Q11				5734	/* 2.8 * (1 << 11) */
#define MX01_Q15			-3277	/* -0.1 * (1 << 15) */
#define MX02_Q15			-6554	/* -0.2 * (1 << 15) */


/*===============other globals==============================*/
int16_t lpc[LPC_ORD + 1];
int16_t num_frames;
/*=============== function prototypes==============================*/
void melp_ana(int16_t sp_in[], struct melp_param *par,
		     int16_t subnum);

#define time 1.08
#pragma comment(lib, "winmm.lib")
short int waveIn[8640 /** 100*/]; // 8000 * time
#define tp 1.08
/*=========================receiver end globals======================*/
sem_t melp_start;
sem_t play_start;
sem_t reciever_start;
sem_t decryptor_start;

unsigned char h2_buffer[176];
int eof_reached2 = 0;
int file_id = 0;
/*=========================receiver end globals======================*/
#define FRM 540
sem_t mutx;
sem_t empty;
sem_t full;
sem_t last;

sem_t ana_post;
sem_t id_analyzer;

short buff[540];
unsigned char aes_buffer[36];
unsigned char array2[176];
//melp defines and includes
#include "melpe.h"
HANDLE h1;
int last_file = 0;
int flag = 0;
short ana_arr[540] = {0};
char in_name[80]="input.raw", out_name[80]="out.raw", out_bit[80]="out_bit.bit";

int i = 0;//, length,count;
int length; int count = 1;
int eof_reached = 0;

clock_t t1, t2, ft, t_ovr;
float exe_time;
int start = 0;
char key_hit;

int inner_melp_post_count = 0;
int length_count = 0;

sem_t wait_read;
sem_t wait_melp;

sem_t rec_start;

sem_t start_melp_1ana;
sem_t start_melp_2ana;
sem_t start_melp_3ana;

sem_t melp_1ana;
sem_t melp_2ana;
sem_t melp_3ana;


void writedataTofile(LPSTR lpData,DWORD dwBufferLength,int a);
HANDLE GetSerialPort(char *p);
void aes_transmitter (unsigned char bytes_to_send[], int bytes_size, HANDLE h1);

void *producer() 
{
	clock_t prod_t1,prod_t2,prod_t3,prod_tf,t1_ovr;
	int a = 0;
	int loop_dc = 0;
	char location;
	FILE *fp;
	location = 0;
	
	printf("Producer Start\n\n");
	while(start == 1)
	{
		printf("waiting for recorder to complete file write\n");
		t1_ovr = clock();
		sem_wait(&wait_read);
		
		t_ovr = clock();
		
		if (a%2 == 0)
		{
			//t_ovr = clock();
			fp = fopen("record0.raw", "rb");
		}
		else if(a%2 == 1)
		{
			fp = fopen("record1.raw", "rb");
			//exit(1);
		}
		a++;
		
		while(eof_reached == 0)
		{
			sem_wait(&empty);

			length = fread(buff,sizeof(short),FRM,fp);
			
			npp(buff, buff);
			npp(&(buff[FRAME]), &(buff[FRAME]));
			npp(&(buff[2 * FRAME]), &(buff[2 * FRAME]));
			
			sem_post(&id_analyzer);
			inner_melp_post_count = inner_melp_post_count + 1;
			//printf("number %d melp post from producer\n",inner_melp_post_count);
			
			if (length < FRM) 
			{
				count = count + 1;
				eof_reached = 1;
				printf("eof reached\n");
				length_count = length_count + 1;
			}
			//prod_t2 = clock() - prod_t1;
			//printf ("time in prod_thread is %f secs\n",((float)prod_t2)/CLOCKS_PER_SEC);
			//exit(1);
		}
		//printf("end of File %d\n\n",a);
		eof_reached = 0;
		inner_melp_post_count = 0;
		length_count = 0;
		fclose(fp);
		
		prod_t2 = clock() - t_ovr;
		printf ("Time in 1 file operation excluding wait for file_read: %f secs\n\n",((float)prod_t2)/CLOCKS_PER_SEC);

		prod_t3 = clock() - t1_ovr;
		printf ("Time in 1 file operation including wait for file_read: %f secs\n\n",((float)prod_t3)/CLOCKS_PER_SEC);
	}
	printf("PRODUCER end\n\n");
	count = 0;
	//sem_post(&mutx);
	sem_post(&full);
	pthread_exit(NULL);
}

void show(unsigned char *object, size_t size)
{
   const unsigned char *byte;
   for ( byte = object; size--; ++byte )
   {
      unsigned char mask;
      for ( mask = 1 << (CHAR_BIT - 1); mask; mask >>= 1 )
      {
         putchar(mask & *byte ? '1' : '0');
      }
      putchar(' ');
   }
   putchar('\n');
}
 
void shiftl(unsigned char *object, size_t size)
{
   unsigned char *byte;
   for ( byte = object; size--; ++byte )
   {
      unsigned char bit = 0;
      if ( size )
      {
         bit = byte[1] & (1 << (CHAR_BIT - 1)) ? 1 : 0;
      }
      *byte <<= 1;
      *byte  |= bit;
   }
}


void *analyzer()
{
	clock_t t1, t2;
	//short ana_arr[540] = {0};
	float act_time,msec;
	unsigned char enc_speech[11];
	unsigned char array1[176] = {0x00};
	int b = 0, loc = 0, count_64blocks = 0;
	int i = 0;
	
	while(count != 0) 
	{
		sem_wait(&id_analyzer);
		//sem_wait(&mutx);
		//if (count_64blocks == 0)
		
		t1 = clock();
		
		memcpy(ana_arr, buff, 1080);
	
		sem_post(&empty);
	
		//t1 = clock();
		num_frames = (int16_t) ((rate == RATE2400) ? 1 : NF);

		//for (i = 0; i < num_frames; i++) 
		//{
		//	dc_rmv(&ana_arr[i * FRAME], &hpspeech[IN_BEG + i * FRAME],
		//			dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		//	//melp_ana(&hpspeech[i * FRAME], &melp_par[i], i);
		//}

		dc_rmv(&ana_arr[0], &hpspeech[IN_BEG + 0],
						dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[0], &melp_par[0],0);

		dc_rmv(&ana_arr[FRAME], &hpspeech[IN_BEG + FRAME],
						dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[FRAME], &melp_par[1], 1);

		dc_rmv(&ana_arr[2 * FRAME], &hpspeech[IN_BEG + 2 * FRAME],
					dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[2 * FRAME], &melp_par[2], 2);
	
		/*sem_post(&start_melp_1ana);
		sem_post(&start_melp_2ana);
		sem_post(&start_melp_3ana);


		sem_wait(&melp_1ana);
		sem_wait(&melp_2ana);
		sem_wait(&melp_3ana);*/
		

		sc_ana(melp_par);

		/* ======== Quantization ======== */

		lpc[0] = ONE_Q12;

		lsf_vq(melp_par);

		pitch_vq(melp_par);

		gain_vq(melp_par);

		quant_u(&melp_par[0].jitter, &(quant_par.jit_index[0]), 0,
				MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);
		quant_u(&melp_par[1].jitter, &(quant_par.jit_index[1]), 0,
				MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);
		quant_u(&melp_par[2].jitter, &(quant_par.jit_index[2]), 0,
				MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);

		//for (i = 0; i < NF; i++)
		//	quant_u(&melp_par[i].jitter, &(quant_par.jit_index[i]), 0,
		//		MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);

		/* Quantize bandpass voicing */

		quant_bp(melp_par, num_frames);

		quant_jitter(melp_par);

		/* Calculate Fourier coeffs of residual from quantized LPC */
		//for (i = 0; i < num_frames; i++) 
		//{
		//	/* The following fill() action is believed to be unnecessary. */
		//	//fill(melp_par[i].fs_mag, ONE_Q13, NUM_HARM);
		//	
		//	if (!melp_par[i].uv_flag) 
		//	{
		//		lpc_lsp2pred(melp_par[i].lsf, &(lpc[1]), LPC_ORD);
		//		zerflt(&hpspeech
		//			   [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
		//			   sigbuf, LPC_ORD, LPC_FRAME);
		//		
		//		window(sigbuf, win_cof, sigbuf, LPC_FRAME);
		//		
		//		find_harm(sigbuf, melp_par[i].fs_mag, melp_par[i].pitch, NUM_HARM,
		//			  LPC_FRAME);
		//	}
		//}
		i = 0;
		if (!melp_par[i].uv_flag) 
			{
				lpc_lsp2pred(melp_par[i].lsf, &(lpc[1]), LPC_ORD);
				zerflt(&hpspeech
					   [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
					   sigbuf, LPC_ORD, LPC_FRAME);
				
				window(sigbuf, win_cof, sigbuf, LPC_FRAME);
				
				find_harm(sigbuf, melp_par[i].fs_mag, melp_par[i].pitch, NUM_HARM,
					  LPC_FRAME);
			}
		i = 1;
		if (!melp_par[i].uv_flag) 
			{
				lpc_lsp2pred(melp_par[i].lsf, &(lpc[1]), LPC_ORD);
				zerflt(&hpspeech
					   [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
					   sigbuf, LPC_ORD, LPC_FRAME);
				
				window(sigbuf, win_cof, sigbuf, LPC_FRAME);
				
				find_harm(sigbuf, melp_par[i].fs_mag, melp_par[i].pitch, NUM_HARM,
					  LPC_FRAME);
			}
		i = 2;
		if (!melp_par[i].uv_flag) 
			{
				lpc_lsp2pred(melp_par[i].lsf, &(lpc[1]), LPC_ORD);
				zerflt(&hpspeech
					   [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
					   sigbuf, LPC_ORD, LPC_FRAME);
				
				window(sigbuf, win_cof, sigbuf, LPC_FRAME);
				
				find_harm(sigbuf, melp_par[i].fs_mag, melp_par[i].pitch, NUM_HARM,
					  LPC_FRAME);
			}

		quant_fsmag(melp_par);

		//for (i = 0; i < num_frames; i++)
		//	quant_par.uv_flag[i] = melp_par[i].uv_flag;
		quant_par.uv_flag[0] = melp_par[0].uv_flag;
		quant_par.uv_flag[1] = melp_par[1].uv_flag;
		quant_par.uv_flag[2] = melp_par[2].uv_flag;

		/* Write channel bitstream */

		#if !SKIP_CHANNEL
				low_rate_chn_write(&quant_par);
		#endif

		/* Update delay buffers for next block */
		v_equ(hpspeech, &(hpspeech[num_frames * FRAME]), IN_BEG);	
		
		memcpy(enc_speech,chbuf,11);



		/*======================old content of melp thread======================*/
		//melpe_a(enc_speech, buff);
		//fwrite(enc_speech,sizeof(char),11,fptr);
		//=======================================================================
		// ather_jawad work for 2400 baudrate
		//=======================================================================
		for (b=0;b<11;b++)
		{
			array1[loc] = enc_speech[b];
			loc++;
		}

		count_64blocks = count_64blocks + 1;
		if(count_64blocks == 16) 
		{
			loc = 0;
			memcpy(array2,array1,176);
			sem_post(&last);
			
			count_64blocks = 0;
		}

		t2 = clock() - t1;
		//printf("Time %d = %f\n\n",count_64blocks,(float)t2/CLOCKS_PER_SEC);
	
		//sem_post(&full);
	}
	pthread_exit(NULL);
}

void *melp_analysis1()
{
	int i = 0;
	while(1)
	{
		sem_wait(&start_melp_1ana);
		dc_rmv(&ana_arr[i * FRAME], &hpspeech[IN_BEG + i * FRAME],
						dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[i * FRAME], &melp_par[i], i);
		sem_post(&melp_1ana);
	}
}

void *melp_analysis2()
{
	int i = 1;
	while(1)
	{
		sem_wait(&start_melp_2ana);
		dc_rmv(&ana_arr[i * FRAME], &hpspeech[IN_BEG + i * FRAME],
						dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[i * FRAME], &melp_par[i], i);
		sem_post(&melp_2ana);
	}
}

void *melp_analysis3()
{
	int i = 2;
	while(1)
	{
		sem_wait(&start_melp_3ana);
		dc_rmv(&ana_arr[i * FRAME], &hpspeech[IN_BEG + i * FRAME],
						dcdelin, dcdelout_hi, dcdelout_lo, FRAME);
		melp_ana(&hpspeech[i * FRAME], &melp_par[i], i);
		sem_post(&melp_3ana);
	}
}
//void *melp()
//{
//	clock_t melp_t1, melp_t2, melp_ft;
//	int length,loc;
//    short speech_in[FRM];
//    short speech_out[FRM];
//	short melp_arr[540] = {0};
//	unsigned char enc_speech[11];
//	struct melp_param par[3];
//
//	int16_t local_sigbuf[327];
//	//int16_t *buff_melp = hpspeech;
//	int16_t buff_melp[900];
//	FILE *fptr;
//	int b=0;
//	int msec = 0;	
//	int j_i=0;
//	int alpha = 0;
//	int count_64blocks = 0;
//	unsigned char array1[176] = {0x00};
//
//	unsigned char filter_data;
//	int i = 0;
//	int	temp = 0;
//    int eof_reached;
//   // int num_frames = 0;
//
//    //unsigned int chbuf[CHSIZE];
//    int while_count = 0;
//	//FILE *fp_out;
//	melpe_i();
//	sem_wait(&wait_melp);
//	if (( fptr = fopen("padded_test.bit","wb")) == NULL ) {
//	printf("  ERROR: cannot write file padded_test.bit.\n");
//	exit(1);
//    }
//	
//
//	loc = 0;
//    //frame = 0;
//    eof_reached = 0;
//	printf("MELP starts\n\n");
//	//melp_t1 = clock();
//	while(count != 0) 
//	{
//		sem_wait(&full);
//		
//		memcpy(melp_arr, ana_arr, 1080);
//		memcpy(buff_melp, hpspeech, 1800);
//		memcpy(local_sigbuf,sigbuf, 654);
//		
//		par[0] = melp_par[0];
//		par[1] = melp_par[1];
//		par[2] = melp_par[2];
//		
//		sem_post(&mutx);
//
//		/* Perform MELP analysis */
//		sc_ana(par);
//
//		/* ======== Quantization ======== */
//
//		lpc[0] = ONE_Q12;
//
//		lsf_vq(par);
//
//		pitch_vq(par);
//
//		gain_vq(par);
//
//		for (i = 0; i < NF; i++)
//			quant_u(&par[i].jitter, &(quant_par.jit_index[i]), 0,
//				MAX_JITTER_Q15, 2, ONE_Q15, 1, 7);
//
//		/* Quantize bandpass voicing */
//
//		quant_bp(par, num_frames);
//
//		quant_jitter(par);
//
//		/* Calculate Fourier coeffs of residual from quantized LPC */
//		for (i = 0; i < num_frames; i++) 
//		{
//			/* The following fill() action is believed to be unnecessary. */
//			fill(par[i].fs_mag, ONE_Q13, NUM_HARM);
//			
//			if (!par[i].uv_flag) 
//			{
//				lpc_lsp2pred(par[i].lsf, &(lpc[1]), LPC_ORD);
//				zerflt(&hpspeech
//					   [(i * FRAME + FRAME_END - (LPC_FRAME / 2))], lpc,
//					   local_sigbuf, LPC_ORD, LPC_FRAME);
//				
//				window(local_sigbuf, win_cof, local_sigbuf, LPC_FRAME);
//				
//				find_harm(local_sigbuf, par[i].fs_mag, par[i].pitch, NUM_HARM,
//					  LPC_FRAME);
//			}
//		}
//
//		quant_fsmag(par);
//
//		for (i = 0; i < num_frames; i++)
//			quant_par.uv_flag[i] = par[i].uv_flag;
//
//		/* Write channel bitstream */
//
//		#if !SKIP_CHANNEL
//				low_rate_chn_write(&quant_par);
//		#endif
//
//		/* Update delay buffers for next block */
//		v_equ(hpspeech, &(hpspeech[num_frames * FRAME]), IN_BEG);	
//		
//		memcpy(enc_speech,chbuf,11);
//
//
//
//	/*======================old content of melp thread======================*/
//		//melpe_a(enc_speech, buff);
//		//fwrite(enc_speech,sizeof(char),11,fptr);
//		//=======================================================================
//		// ather_jawad work for 2400 baudrate
//		//=======================================================================
//		for (b=0;b<11;b++)
//		{
//			array1[loc] = enc_speech[b];
//			loc++;
//		}
//
//		count_64blocks = count_64blocks + 1;
//		if(count_64blocks == 16) 
//		{
//			loc = 0;
//			memcpy(array2,array1,176);
//			sem_post(&last);
//			count_64blocks = 0;
//		}
//
//	}
//	
//	pthread_exit(NULL);
//}

void *aes_serial()
{
	clock_t tt1,tt2;
	unsigned char arr_ser[176];
	printf("Serial start\n\n");
	
	//while (count != 0)
	while (1)
	{
		
		sem_wait(&last);
		tt1 = clock();
		memcpy(arr_ser, array2, 176);
		aes_transmitter(arr_ser, sizeof(aes_buffer), h1);
		if (last_file == 1)
		{
			flag = 1;
		}
		tt2 = clock() - tt1;
		printf("aes time: %f\n\n",(float)tt2/CLOCKS_PER_SEC);
	}

	pthread_exit(NULL);
}

void *recorder(void)
{
	
	HWAVEIN	hWaveIn;
	MMRESULT result;
	WAVEFORMATEX pFormat;
    int count_r = 0;
	WAVEHDR      WaveInHdr;
	const int NUMPTS = 8000 * time;   // 3 seconds
	int sampleRate = 8000;
	int file_count = 0;
	/* Print user message */
    printf("\n2.4 kb/s Proposed Federal Standard MELP speech coder\n");
    printf("  C simulation, version 1.2\n\n");
	
	printf("Recorder Start\n\n");
	while(1)//for(file_count = 0; file_count < 10; file_count++)
	{
		//sem_wait(&wait_write);
		// 'short int' is a 16-bit type; I request 16-bit samples below
		// for 8-bit capture, you'd use 'unsigned char' or 'BYTE' 8-bit     types
		//printf("Press C to Stop\n\n");
		//if(kbhit())	key_hit = getch();
		//if(key_hit != 'c')
		//{
		sem_wait(&rec_start);
			pFormat.wFormatTag=WAVE_FORMAT_PCM;     // simple, uncompressed format
			pFormat.nChannels=1;                    //  1=mono, 2=stereo
			pFormat.nSamplesPerSec=sampleRate;      // 8.0 kHz, 11.025 kHz, 22.05 kHz, and 44.1 kHz
			pFormat.nAvgBytesPerSec=sampleRate*2;   // =  nSamplesPerSec × nBlockAlign
			pFormat.nBlockAlign=2;                  // = (nChannels × wBitsPerSample) / 8
			pFormat.wBitsPerSample=16;              //  16 for high quality, 8 for telephone-grade
			pFormat.cbSize=0;

			// Specify recording parameters

			result = waveInOpen(&hWaveIn, WAVE_MAPPER,&pFormat,
				0L, 0L, WAVE_FORMAT_DIRECT);

			
			// Set up and prepare header for input
			WaveInHdr.lpData = (LPSTR)waveIn;
			WaveInHdr.dwBufferLength = NUMPTS*2;
			WaveInHdr.dwBytesRecorded=0;
			WaveInHdr.dwUser = 0L;
			WaveInHdr.dwFlags = 0L;
			WaveInHdr.dwLoops = 0L;
			waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));

			// Insert a wave input buffer
			result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));


			// Commence sampling input
			result = waveInStart(hWaveIn);


			//cout << "recording..." << endl;

			Sleep(tp * 1000);
			// Wait until finished recording
			writedataTofile((LPSTR)waveIn,/*36000*/17280,count_r);
			waveInClose(hWaveIn);

			printf("file available %d\n\n",count_r);
			count_r++;
		
			sem_post(&wait_melp);
			sem_post(&wait_read);

			sem_post(&rec_start);
		

			//sem_post(&wait_write);
			//writedataTofile(WaveInHdr.lpData,WaveInHdr.dwBytesRecorded);

			//PlayRecord();
			//Sleep(time * 1000);
		//}
		//else
		//{
			//break;
		//}
	}
	printf("Recorder End\n\n");
	pthread_exit(NULL);
}

void writedataTofile(LPSTR lpData,DWORD dwBufferLength,int a)
{
///*************************************************************************************/	
	FILE * fp;
	if(a%2 == 0/* && write_file0 != 1*/)
	{
		fp = fopen ("record0.raw", "wb");
		//write_file0 = 1;
	}
	else if (a%2 == 1/* && write_file1 != 1*/)
	{
		fp = fopen ("record1.raw", "wb");
		//write_file1 = 1;
	}
	else
	{
		fp = fopen ("record2.raw", "ab");
		//write_file2 = 1;
	}
	//fp = fopen ("rcrd0.raw", "wb");
	fwrite (lpData , sizeof(char), /*36000*/dwBufferLength, fp);
	fclose(fp);
//*********************************************************************************/
}

/*===============================receiver threads================================*/
void PlayRecord(LPSTR  input_data)
{
	const int NUMPTS = 8000 * tp;   // 2.25 seconds
	int sampleRate = 8000;
	// 'short int' is a 16-bit type; I request 16-bit samples below
	// for 8-bit capture, you'd    use 'unsigned char' or 'BYTE' 8-bit types
	HWAVEIN  hWaveIn;
	HWAVEOUT hWaveOut;
	WAVEFORMATEX pFormat;
	WAVEHDR  WaveInHdr;
	//StartRecord();
	pFormat.wFormatTag = WAVE_FORMAT_PCM;     // simple, uncompressed format
	pFormat.nChannels = 1;                    //  1=mono, 2=stereo
	pFormat.nSamplesPerSec = sampleRate;      // 44100
	pFormat.nAvgBytesPerSec = sampleRate * 2;   // = nSamplesPerSec * n.Channels * wBitsPerSample/8
	pFormat.nBlockAlign = 2;                  // = n.Channels * wBitsPerSample/8
	pFormat.wBitsPerSample = 16;              //  16 for high quality, 8 for telephone-grade
	pFormat.cbSize = 0;
	// Specify recording parameters
	waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
	
	WaveInHdr.lpData = input_data; // actual data to be played
	// Set up and prepare header for input
	//WaveInHdr.lpData = (LPSTR)waveIn;
	WaveInHdr.dwBufferLength = NUMPTS * 2;
	WaveInHdr.dwBytesRecorded = 0;
	WaveInHdr.dwUser = 0L;
	WaveInHdr.dwFlags = 0L;
	WaveInHdr.dwLoops = 0L;
	waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
	printf("playing...%d\n",file_id);
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &pFormat, 0, 0, WAVE_FORMAT_DIRECT);
	waveOutWrite(hWaveOut, &WaveInHdr, sizeof(WaveInHdr)); // Playing the data
	Sleep(tp * 1000); //Sleep for as long as there was recorded
	waveInClose(hWaveIn);
	waveOutClose(hWaveOut);
}

void PlayRecord(LPSTR  input_data);
void *readandplay()
{
	FILE * fp;
	long lSize;
	LPSTR  buffer;
	size_t result;
	int sz;
	//int file_id = 0;
	eof_reached2 = 0;
	while(eof_reached2 == 0)
	{
		sem_wait(&play_start); // wating for the availability of file to be played
		if(file_id %2 == 0)
		{
			fp = fopen("record0.raw", "rb");
			printf("read file 0\n");
		}
		else if(file_id %2 == 1)
		{
			fp = fopen("record1.raw", "rb");
			printf("read file 1\n");
		}

		file_id++;
		if (fp == NULL) { fputs("File error", stderr); exit(1); }
		// obtain file size:
		fseek(fp, 0, SEEK_END);
		lSize = ftell(fp);
		rewind(fp);
		sz = sizeof(char)*lSize;
		//printf("Size of buffer is %d\n\n", sz);
		// allocate memory to contain the whole file:
		buffer = (LPSTR)malloc(sz);
		if (buffer == NULL) { fputs("Memory error", stderr); exit(2); }
		// copy the file into the buffer:
		result = fread(buffer, 1, lSize, fp);
		if (result != lSize) { fputs("Reading error", stderr); exit(3); }
		/* the whole file is now loaded in the memory buffer. */
		// terminate
		PlayRecord(buffer); // playing the file
//		sem_post(&write_start); //Allowing the function to write a new file
		fclose(fp);
		free(buffer);
	}
	
}

void *melp_rec(void)
{
	int length, frame,count, melp_file_count;
	int eof_reached = 0;
    //int num_frames = 0;
    
	//short speech_in[11];
    short speech_out[FRM];
	
	unsigned char enc_speech[11];

    FILE *fp_in, *fp_out;
	melpe_i();
	melp_file_count = 0;

    printf("\n1.2 kb/s Proposed Federal Standard MELP speech coder by Jawad\n");
    printf("  C simulation, version 1.0\n\n");
	while (1) 
	{
		sem_wait(&melp_start);

		melp_file_count = melp_file_count + 1;
		if(melp_file_count % 2 == 1) 
		{
			fp_in = fopen("out1.bit" ,"rb");
			fp_out = fopen("record0.raw","wb");
		}
		else 
		{
			fp_in = fopen("out2.bit" ,"rb");
			fp_out = fopen("record1.raw","wb");
		}

		eof_reached = 0;
	
		while (eof_reached == 0) 
		{
			length = fread(&enc_speech,sizeof(char),11,fp_in);

			melpe_s(speech_out, enc_speech);

			fwrite(speech_out,sizeof(short),FRM,fp_out);
		
			///if (length < FRAME) 
			if (length < 11)
			{
				eof_reached = 1;
				printf("eof reached \n");
			}
		}

		fclose(fp_in);
		fclose(fp_out);
		eof_reached = 0;

		sem_post(&play_start);
	}
	pthread_exit(NULL);
}

void *decryptor()
{
	unsigned char buff_aes1[16];
	unsigned char buff_aes2[16];
	unsigned char buff_aes3[16];
	unsigned char buff_aes4[16];
	unsigned char buff_aes5[16];
	unsigned char buff_aes6[16];
	unsigned char buff_aes7[16];
	unsigned char buff_aes8[16];
	unsigned char buff_aes9[16];
	unsigned char buff_aes10[16];
	unsigned char buff_aes11[16];
	
	unsigned char aes_dec[176], w_char1, w_char2, w_char3, w_char4;
	
	int temp, temp2;
	int index = 0, b;
	unsigned int filter = 32768,temp_r[9], temp_36extra[9]= {0};

	int s=0;
	int j =0;
	unsigned char key[32];
	int count_filewrite = 0;
	char out_name1[80] = "out1.bit"; 
	char out_name2[80] = "out2.bit";
	FILE *fp;
	
	unsigned char local_h2_buffer[176];
	
	int count = 0;
    DWORD byteswritten = 0, bytesread = 0;
	aes256_context ctx;
	unsigned char i;
	fp = fopen(out_name1, "wb");
	for (i = 0; i < sizeof(key);i++) key[i] = i;

	while(1)
	{
		sem_wait(&decryptor_start);
		memcpy(local_h2_buffer,h2_buffer,176);
		sem_post(&reciever_start);

		for(j=0;j<sizeof(buff_aes1);j++) 
		{
			buff_aes1[j] = local_h2_buffer[j];
			buff_aes2[j] = local_h2_buffer[j+16];
			buff_aes3[j] = local_h2_buffer[j+32];
			buff_aes4[j] = local_h2_buffer[j+48];
			buff_aes5[j] = local_h2_buffer[j+64];
			buff_aes6[j] = local_h2_buffer[j+80];
			buff_aes7[j] = local_h2_buffer[j+96];
			buff_aes8[j] = local_h2_buffer[j+112];
			buff_aes9[j] = local_h2_buffer[j+128];
			buff_aes10[j] = local_h2_buffer[j+144];
			buff_aes11[j] = local_h2_buffer[j+160];
		}

		aes256_init(&ctx, key);
	    aes256_decrypt_ecb(&ctx, buff_aes1);
	    aes256_decrypt_ecb(&ctx, buff_aes2);
	    aes256_decrypt_ecb(&ctx, buff_aes3);
	    aes256_decrypt_ecb(&ctx, buff_aes4);
	    aes256_decrypt_ecb(&ctx, buff_aes5);
	    aes256_decrypt_ecb(&ctx, buff_aes6);
	    aes256_decrypt_ecb(&ctx, buff_aes7);
	    aes256_decrypt_ecb(&ctx, buff_aes8);
	    aes256_decrypt_ecb(&ctx, buff_aes9);
	    aes256_decrypt_ecb(&ctx, buff_aes10);
	    aes256_decrypt_ecb(&ctx, buff_aes11);
	    aes256_done(&ctx);

	    for(j=0;j<sizeof(buff_aes1);j++) 
	    {
			local_h2_buffer[j] = buff_aes1[j];
			local_h2_buffer[j+16] = buff_aes2[j];
			local_h2_buffer[j+32] = buff_aes3[j];
			local_h2_buffer[j+48] = buff_aes4[j];
			local_h2_buffer[j+64] = buff_aes5[j];
			local_h2_buffer[j+80] = buff_aes6[j];
			local_h2_buffer[j+96] = buff_aes7[j];
			local_h2_buffer[j+112] = buff_aes8[j];
			local_h2_buffer[j+128] = buff_aes9[j];
			local_h2_buffer[j+144] = buff_aes10[j];
			local_h2_buffer[j+160] = buff_aes11[j];
		}

		fwrite((void *)local_h2_buffer,sizeof(char),176,fp);
		fwrite((void *)temp_36extra,sizeof(int),9,fp);


		count_filewrite = count_filewrite + 1; 
		fclose(fp);
		sem_post(&melp_start);

		if (count_filewrite%2 == 1)
			fp = fopen(out_name2, "wb");
		else
			fp = fopen(out_name1, "wb");
	}
	pthread_exit(NULL);
}

HANDLE GetSerialPort(char *p);
void *aes_receiver ()
{
	
	int temp, mytime;
	aes256_context ctx; 
    unsigned char key[32];
	unsigned char i;
	//COMMTIMEOUTS timeouts = {0};
	//timeouts.ReadIntervalTimeout = 50;
	//timeouts.ReadTotalTimeoutConstant = 50;
	//timeouts.ReadTotalTimeoutMultiplier = 10;

    for (i = 0; i < sizeof(key);i++) key[i] = i;

	ser_comm_rec(key, h1);
	pthread_exit(NULL);
}

int ser_comm_rec(unsigned char *key, HANDLE h1)
{	
	unsigned char aes_dec[176], w_char1, w_char2, w_char3, w_char4;
	
	int temp, temp2;
	unsigned char rx = {0};
	int bytes_read = 0;
	int index = 0, b;
	unsigned int filter = 32768,temp_r[9];
	int s=0;
	int count_filewrite = 0;

	int count = 0;
    DWORD byteswritten = 0, bytesread = 0;

	unsigned char i;

	while(1)
	{
		sem_wait(&reciever_start);
		//while (bytes_read < 176)
	//	{
			ReadFile(h1, h2_buffer, 176, &bytesread, NULL);
		//	if (bytesread == 1)
		//	{
		//	h2_buffer[bytes_read] = rx;
		//	bytes_read++;
			if (bytesread == 176)
				{
				//bytes_read = 0;
				printf("decryption number %d is complete\n", count);
				sem_post(&decryptor_start);
				}
		//	}
			else
			{
			printf("Break from while loop\n\n");
			//printf("value of bytes_read is %d\n",bytes_read);
			sem_post(&reciever_start);
			//exit(1);
			//break;
			}
	//	}

		//if (bytes_read == 176)
	//	{
     //      bytes_read = 0;
	//	   printf("decryption number %d is complete\n", count);
	//	   sem_post(&decryptor_start);
	//	}
	//	else 
	//		sem_post(&reciever_start);
		
	}
}


void main( /*int argc, char *argv[]*/ )
{
	
	int s_pressed = 0;
	char c1[] = {"COM6"};
	COMMTIMEOUTS timeouts = {0};
	pthread_t idp,idc,id_aes,idr;
	pthread_t id_ana;

	pthread_t id_aes_rec,id_melp;
	pthread_t play;
	pthread_t id_decryptor;

	pthread_t id_melp_1ana, id_melp_2ana, id_melp_3ana;

	sem_init(&mutx, NULL, 1);
	sem_init(&empty, NULL, 1);
	sem_init(&full, NULL, 0);
	sem_init(&last, NULL, 0);
	sem_init(&wait_read, NULL, 0);
	sem_init(&wait_melp, NULL, 0);
	sem_init(&rec_start, NULL, 1);
	sem_init(&id_analyzer, NULL, 0);
	/*==================receiver semaphores======================*/
	sem_init(&melp_start, NULL, 0);
	sem_init(&play_start, NULL, 0);
	sem_init(&reciever_start, NULL, 0);
	sem_init(&decryptor_start, NULL, 0);

	sem_init(&melp_1ana, NULL, 0);
	sem_init(&melp_2ana, NULL, 0);
	sem_init(&melp_3ana, NULL, 0);
	
	sem_init(&start_melp_1ana, NULL, 0);
	sem_init(&start_melp_2ana, NULL, 0);
	sem_init(&start_melp_3ana, NULL, 0);

	
	h1 = GetSerialPort(c1);	
	timeouts.ReadIntervalTimeout = 100;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.ReadTotalTimeoutMultiplier = 20;
	if(!SetCommTimeouts(h1, &timeouts)){
	printf("Error setting port state \n"); }
	
	printf("TX\n\n");
	printf("Press S to Start\n\n");
	printf("Press C to Stop\n\n");
	// Creating threads
	while(1)
	{
		//while(!kbhit());
		//printf("keystroke pressed\n");
		//key_hit = getch();
		//exit(1);
		if(kbhit())	key_hit = getch();
		if(key_hit == 's' && s_pressed == 0)
		{
		if(EscapeCommFunction(h1,SETRTS) == 0) 
		{
		printf ("Error Setting RTS\n");
		exit(1);}
				s_pressed = 1;
				if(start == 0)
				{
					start = 1;
					pthread_create(&idr, NULL, recorder, NULL);	
					//pthread_create(&idc, NULL, melp, NULL);
					pthread_create(&idp, NULL, producer, NULL);

					pthread_create(&id_melp_1ana, NULL, melp_analysis1, NULL);
					pthread_create(&id_melp_2ana, NULL, melp_analysis2, NULL);
					pthread_create(&id_melp_3ana, NULL, melp_analysis3, NULL);
					
					pthread_create(&id_aes, NULL, aes_serial, NULL);
					pthread_create(&id_ana, NULL, analyzer, NULL);

					pthread_create(&id_aes_rec, NULL, aes_receiver, NULL);
					pthread_create(&id_melp, NULL, melp_rec, NULL);
					pthread_create(&play, NULL, readandplay, NULL);
					pthread_create(&id_decryptor, NULL, decryptor, NULL);
				}
				else
				{
					sem_post(&rec_start);
					sem_wait(&reciever_start);
				}
		}
		else if(key_hit == 'c' && s_pressed == 1)
		//else if(key_hit == 'c')
		{
			//start = 0;
			s_pressed = 0;
			sem_wait(&rec_start);
			//last_file = 1;
			//while(flag == 0);
			
		
			sem_post(&reciever_start);
			// Waiting for threads to complete operation
			
		}
		else if(key_hit == 'e')// && start == 0)
		{
			break;
		}
		else
		{
			//printf("Press any valid key\n\n");
			if(s_pressed == 0)
			{
				printf("Press S to Start\n\n");
				printf("Press C to Stop\n\n");
				printf("Press C followed by E to Exit\n\n");
			}
			while(!kbhit());
		}
	}
	pthread_join(idr, NULL);
	pthread_join(idp, NULL);
	pthread_join(idc, NULL);
	pthread_join(id_aes, NULL);
	pthread_join(id_ana, NULL);

	pthread_exit(NULL);
}
