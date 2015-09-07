/*  
*   Byte-oriented AES-256 implementation.
*   All lookup tables replaced with 'on the fly' calculations. 
*
*   Copyright (c) 2007 Ilya O. Levin, http://www.literatecode.com
*
*   Permission to use, copy, modify, and distribute this software for any
*   purpose with or without fee is hereby granted, provided that the above
*   copyright notice and this permission notice appear in all copies.
*
*   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
*   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
*   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
*   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
*   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
*   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <time.h>
#include "aes256.h"
#define DUMP(s, i, buf, sz)  {printf(s);                   \
                              for (i = 0; i < (sz);i++)    \
                                  printf("%02x ", buf[i]); \
                              printf("\n");}

//======================================================
// Get serial port
//======================================================

 HANDLE GetSerialPort(char *p)
{
	HANDLE hSerial;
	DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
    dcbSerialParams.BaudRate=CBR_2400;
    dcbSerialParams.ByteSize=8;
    dcbSerialParams.StopBits=ONESTOPBIT;
    dcbSerialParams.Parity=NOPARITY;
	 
    hSerial = CreateFile(p,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         0,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0);
    SetCommState(hSerial, &dcbSerialParams);
    return hSerial;
}

//=============================================
// serial transmitter
//=============================================
 int ser_comm(unsigned char *h1_buffer, HANDLE h1)
{
    DWORD byteswritten = 0;	 
	if (!WriteFile(h1, h1_buffer, 176, &byteswritten, NULL))
	{
		fprintf(stderr, "Error sending bytes serially\n");
        //CloseHandle(h1);
	}
	else{}
	//printf("%d bytes successfully sent serially\n",36);
return 0;
}


 void aes_transmitter (unsigned char bytes_to_send[], int bytes_size, HANDLE h1)
{
	int j;
	int temp1 = 16;
	
	int tempp1 = 16;
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
	/*unsigned char buff_aes12[16];
	unsigned char buff_aes13[16];
	unsigned char buff_aes14[16];
	unsigned char buff_aes15[16];
	unsigned char buff_aes16[16];
	unsigned char buff_aes17[16];
	unsigned char buff_aes18[16];
	unsigned char buff_aes19[16];
	unsigned char buff_aes20[16];
	unsigned char buff_aes21[16];
	unsigned char buff_aes22[16];
	unsigned char buff_aes23[16];
	unsigned char buff_aes24[16];
	unsigned char buff_aes25[16];
	unsigned char buff_aes26[16];
	unsigned char buff_aes27[16];*/

	unsigned char buff_enc[176];
	aes256_context ctx; 
    uint8_t key[32];
	uint8_t i;
	//HANDLE h1;
	//char c1[] = {"COM3"};
	int first_time;
	int temp, mytime, second_time;
	clock_t t1, t2, t;
	t1 = clock();
	first_time = time(NULL);
	for(j=0;j<sizeof(buff_aes1);j++) {
		buff_aes1[j] = bytes_to_send[j];
		buff_aes2[j] = bytes_to_send[j+16];
		buff_aes3[j] = bytes_to_send[j+32];
		buff_aes4[j] = bytes_to_send[j+48];
		buff_aes5[j] = bytes_to_send[j+64];
		buff_aes6[j] = bytes_to_send[j+80];
		buff_aes7[j] = bytes_to_send[j+96];
		buff_aes8[j] = bytes_to_send[j+112];
		buff_aes9[j] = bytes_to_send[j+128];
		buff_aes10[j] = bytes_to_send[j+144];
		buff_aes11[j] = bytes_to_send[j+160];
		/*buff_aes12[j] = bytes_to_send[j+176];
		buff_aes13[j] = bytes_to_send[j+192];
		buff_aes14[j] = bytes_to_send[j+208];
		buff_aes15[j] = bytes_to_send[j+224];
		buff_aes16[j] = bytes_to_send[j+240];
		buff_aes17[j] = bytes_to_send[j+256];
		buff_aes18[j] = bytes_to_send[j+272];
		buff_aes19[j] = bytes_to_send[j+288];
		buff_aes20[j] = bytes_to_send[j+304];
		buff_aes21[j] = bytes_to_send[j+320];
		buff_aes22[j] = bytes_to_send[j+336];
		buff_aes23[j] = bytes_to_send[j+352];
		buff_aes24[j] = bytes_to_send[j+368];
		buff_aes25[j] = bytes_to_send[j+384];
		buff_aes26[j] = bytes_to_send[j+400];
		buff_aes27[j] = bytes_to_send[j+416];
		temp1 = temp1 + 1;*/
	}
	//h1 = GetSerialPort(c1);	
	if (h1 == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Error opening serial port\n");
		exit(1);
	}   
	else {}
		//printf("serial port opened successfully\n");
	  /* put a test vector */
    for (i = 0; i < sizeof(key);i++) key[i] = i;	
	
//     DUMP("plain txt: ", i, bytes_to_send, bytes_size);
  //  printf("---\n");

   aes256_init(&ctx, key);
   aes256_encrypt_ecb(&ctx, buff_aes1);
   aes256_encrypt_ecb(&ctx, buff_aes2);
   aes256_encrypt_ecb(&ctx, buff_aes3);
   aes256_encrypt_ecb(&ctx, buff_aes4);
   aes256_encrypt_ecb(&ctx, buff_aes5);
   aes256_encrypt_ecb(&ctx, buff_aes6);
   aes256_encrypt_ecb(&ctx, buff_aes7);
   aes256_encrypt_ecb(&ctx, buff_aes8);
   aes256_encrypt_ecb(&ctx, buff_aes9);
   aes256_encrypt_ecb(&ctx, buff_aes10);
   aes256_encrypt_ecb(&ctx, buff_aes11);
   /*aes256_encrypt_ecb(&ctx, buff_aes12);
   aes256_encrypt_ecb(&ctx, buff_aes13);
   aes256_encrypt_ecb(&ctx, buff_aes14);
   aes256_encrypt_ecb(&ctx, buff_aes15);
   aes256_encrypt_ecb(&ctx, buff_aes16);
   aes256_encrypt_ecb(&ctx, buff_aes17);
   aes256_encrypt_ecb(&ctx, buff_aes18);
   aes256_encrypt_ecb(&ctx, buff_aes19);
   aes256_encrypt_ecb(&ctx, buff_aes20);
   aes256_encrypt_ecb(&ctx, buff_aes21);
   aes256_encrypt_ecb(&ctx, buff_aes22);
   aes256_encrypt_ecb(&ctx, buff_aes23);
   aes256_encrypt_ecb(&ctx, buff_aes24);
   aes256_encrypt_ecb(&ctx, buff_aes25);
   aes256_encrypt_ecb(&ctx, buff_aes26);
   aes256_encrypt_ecb(&ctx, buff_aes27);*/
   //DUMP("encrypted text: ", i, bytes_to_send, bytes_size);
    aes256_done(&ctx);
	
	//delay();
	for(j=0;j<sizeof(buff_aes1);j++) {
		buff_enc[j] = buff_aes1[j];
		buff_enc[j+16] = buff_aes2[j];
		buff_enc[j+32] = buff_aes3[j];
		buff_enc[j+48] = buff_aes4[j];
		buff_enc[j+64] = buff_aes5[j];
		buff_enc[j+80] = buff_aes6[j];
		buff_enc[j+96] = buff_aes7[j];
		buff_enc[j+112] = buff_aes8[j];
		buff_enc[j+128] = buff_aes9[j];
		buff_enc[j+144] = buff_aes10[j];
		buff_enc[j+160] = buff_aes11[j];
		/*buff_enc[j+176] = buff_aes12[j];
		buff_enc[j+192] = buff_aes13[j];
		buff_enc[j+208] = buff_aes14[j];
		buff_enc[j+224] = buff_aes15[j];
		buff_enc[j+240] = buff_aes16[j];
		buff_enc[j+256] = buff_aes17[j];
		buff_enc[j+272] = buff_aes18[j];
		buff_enc[j+288] = buff_aes19[j];
		buff_enc[j+304] = buff_aes20[j];
		buff_enc[j+320] = buff_aes21[j];
		buff_enc[j+336] = buff_aes22[j];
		buff_enc[j+352] = buff_aes23[j];
		buff_enc[j+368] = buff_aes24[j];
		buff_enc[j+384] = buff_aes25[j];
		buff_enc[j+400] = buff_aes26[j];
		buff_enc[j+416] = buff_aes27[j];
		tempp1 = tempp1 + 1;*/
	}
		ser_comm(buff_enc, h1);
		//ser_comm(bytes_to_send, h1);
	//printf("\n");	

	t2 = clock();
	second_time = time(NULL);
	t = t2 - t1;
	mytime = second_time - first_time;
//	printf ("Total execution time %f seconds.\n",((float)t)/CLOCKS_PER_SEC);
//	printf ("Program took me %d cycles to execute.\n",t);
//	printf("Actual time taken is %d seconds.\n", mytime);
} /* main */
