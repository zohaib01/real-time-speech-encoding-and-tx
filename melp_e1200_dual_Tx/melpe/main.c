#include<stdio.h>
#include<stdlib.h>
#include "melpe.h"
//#include "melpe.c"

#define FRAME 540
//#define CHSIZE 9

//char in_name[80]="input.raw", out_name[80]="out.raw", out_bit[80]="out_bit.bit";

void test_melpe(void)

{
	int length, frame,count;
	int eof_reached = 0;
    int num_frames = 0;
    short speech_in[FRAME];
    short speech_out[FRAME];
	unsigned char enc_speech[11];
    //static struct melp_param melp_par;      /* melp parameters */
  //  unsigned int chbuf[CHSIZE];
    FILE *fp_in, *fp_out, *fp_out_raw;

	melpe_i();
    /* Print user message */
    printf("\n2.4 kb/s Proposed Federal Standard MELP speech coder\n");
    printf("  C simulation, version 1.2\n\n");


    /* Open input, output, and parameter files */
   /* if (( fp_in = fopen(in_name,"rb")) == NULL ) {
	printf("  ERROR: cannot read file %s.\n",in_name);
	exit(1);
    }

	if (( fp_out = fopen(out_bit,"wb")) == NULL ) {
	printf("  ERROR: cannot write file %s.\n",out_name);
	exit(1);
    }

    if (( fp_out_raw = fopen(out_name,"wb")) == NULL ) {
	printf("  ERROR: cannot write file %s.\n",out_name);
	exit(1);
    }
	count = 0;
	while (eof_reached == 0) 
	{
	    count = count + 1;
		//read input speech 

		length = fread(&speech_in,sizeof(short),FRAME,fp_in);
		printf("length number %d is : %d\n",count, length);
		
		melpe_a(enc_speech, speech_in);
		melpe_s(speech_out, enc_speech);

		fwrite(enc_speech,sizeof(char),11,fp_out);

		fwrite(speech_out,sizeof(short),FRAME,fp_out_raw);
		
		if (length < FRAME) 
		{
			eof_reached = 1;
			printf("eof reached \n");
	    }
	}
	count  = 0;
	fclose(fp_in);
	fclose(fp_out);
	fclose(fp_out_raw);
	printf("files are closed\n");*/
}