/*
1.function:
usage for openssl aes.
gcc main.c -lcrypto

2.makefile:
CC=gcc
CFLAGS=-Wall -g -O2
LIBS=-lcrypto

all: aes

aes: aes.cc
	$(CC) $(CFLAGS) aes.cc -o $@ $(LIBS)

clean:
	@rm -f aes

3.refer to:
http://www.lovelucy.info/openssl-aes-encryption.html
http://www.inconteam.com/software-development/41-encryption/55-aes-test-vectors#aes-cbc-128

*/
#include <stdio.h>
#include <string.h>
//#include <openssl/ssl.h>
//#include <openssl/evp.h>
#include <openssl/aes.h>

typedef unsigned char byte;
#define LEN 16
static char hexes[16] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'
};

static void bytesToHexString(const byte *input, size_t length, char *output);

int main(int argc, char *argv[])
{
	//SSL_load_error_strings();
	//SSL_library_init();
	AES_KEY aes;
	byte key[AES_BLOCK_SIZE];        // AES_BLOCK_SIZE = 16
	byte iv[AES_BLOCK_SIZE];        // init vector
	byte* input_data;
	byte* encrypt_data;
	byte* decrypt_data;
	char hexString[(LEN << 1 ) + 1] = {0};
	unsigned int i;

	/************test encrypt*************/
	// set the input string
	input_data = (unsigned char*)malloc(LEN);
	if (input_data == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for input_data\n");
		goto end;
	}
	//6bc1bee22e409f96e93d7e117393172a
	input_data[0]=0x6b;
	input_data[1]=0xc1;
	input_data[2]=0xbe;
	input_data[3]=0xe2;
	input_data[4]=0x2e;
	input_data[5]=0x40;
	input_data[6]=0x9f;
	input_data[7]=0x96;
	input_data[8]=0xe9;
	input_data[9]=0x3d;
	input_data[10]=0x7e;
	input_data[11]=0x11;
	input_data[12]=0x73;
	input_data[13]=0x93;
	input_data[14]=0x17;
	input_data[15]=0x2a;

	// Generate AES 128-bit key
	//2b7e151628aed2a6abf7158809cf4f3c
	key[0] = 0x2b;
	key[1] = 0x7e;
	key[2] = 0x15;
	key[3] = 0x16;
	key[4] = 0x28;
	key[5] = 0xae;
	key[6] = 0xd2;
	key[7] = 0xa6;
	key[8] = 0xab;
	key[9] = 0xf7;
	key[10] = 0x15;
	key[11] = 0x88;
	key[12] = 0x09;
	key[13] = 0xcf;
	key[14] = 0x4f;
	key[15] = 0x3c;

	if (AES_set_encrypt_key(key, 128, &aes) < 0)
	{
		fprintf(stderr, "Unable to set encryption key in AES\n");
		goto end_mallocinput;
	}

	/*Generate AES 128-bit iv*/
	//000102030405060708090A0B0C0D0E0F
	iv[0]=0x00;
	iv[1]=0x01;
	iv[2]=0x02;
	iv[3]=0x03;
	iv[4]=0x04;
	iv[5]=0x05;
	iv[6]=0x06;
	iv[7]=0x07;
	iv[8]=0x08;
	iv[9]=0x09;
	iv[10]=0x0A;
	iv[11]=0x0B;
	iv[12]=0x0C;
	iv[13]=0x0D;
	iv[14]=0x0E;
	iv[15]=0x0F;

	// alloc encrypt_data
	encrypt_data = (unsigned char*)malloc(LEN);    
	if (encrypt_data == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for encrypt_data\n");
		goto end_mallocinput;
	}

	// encrypt (iv will change)
	AES_cbc_encrypt(input_data, encrypt_data, LEN, &aes, iv, AES_ENCRYPT);//the iv changes to the encrypt_data also after here.

	/************test decrypt*************/
	// alloc decrypt_data
	decrypt_data = (unsigned char*)malloc(LEN);
	if (decrypt_data == NULL)
	{
		fprintf(stderr, "Unable to allocate memory for decrypt_data\n");
		goto end_mallocencrypt;
	}

	/*Generate AES 128-bit iv again*/
	//000102030405060708090A0B0C0D0E0F
	iv[0]=0x00;
	iv[1]=0x01;
	iv[2]=0x02;
	iv[3]=0x03;
	iv[4]=0x04;
	iv[5]=0x05;
	iv[6]=0x06;
	iv[7]=0x07;
	iv[8]=0x08;
	iv[9]=0x09;
	iv[10]=0x0A;
	iv[11]=0x0B;
	iv[12]=0x0C;
	iv[13]=0x0D;
	iv[14]=0x0E;
	iv[15]=0x0F;

	if (AES_set_decrypt_key(key, 128, &aes) < 0)
	{
		fprintf(stderr, "Unable to set decryption key in AES\n");
		goto end_mallocencrypt;
	}

	// decrypt
	AES_cbc_encrypt(encrypt_data, decrypt_data, LEN, &aes, iv, AES_DECRYPT);//the iv changes to the encrypt_data also after here.

	bytesToHexString(input_data, LEN, hexString);
	printf("input_data:%s, should be:%s\n", hexString, "6bc1bee22e409f96e93d7e117393172a");
	memset(hexString, 0, sizeof(hexString));

	bytesToHexString(key, LEN, hexString);
	printf("key:%s, should be:%s\n", hexString, "2b7e151628aed2a6abf7158809cf4f3c");
	memset(hexString, 0, sizeof(hexString));

	bytesToHexString(encrypt_data, LEN, hexString);
	printf("encrypted string:%s, should be:%s\n", hexString, "7649abac8119b246cee98e9b12e9197d");
	memset(hexString, 0, sizeof(hexString));

	bytesToHexString(decrypt_data, LEN, hexString);
	printf("decrypted string:%s, should be:%s\n", hexString, "6bc1bee22e409f96e93d7e117393172a");
	memset(hexString, 0, sizeof(hexString));

end_mallocdecrypt:
	free(decrypt_data);

end_mallocencrypt:
	free(encrypt_data);

end_mallocinput:
	free(input_data);

end:
	return 0;
}

static void bytesToHexString(const byte *input, size_t length, char *output)
{
	int i;
	//fprintf(stderr, "when convert!\n");
	//dbg("length:%d\n", length);
	for(i = 0; i < length; i++) 
	{
		byte t = input[i];
		byte a = t / 16;
		byte b = t % 16;
		//fprintf(stderr, "i:%d, a:%d, hexa:%c; b:%d, hexb:%c\n",i, a,hexes[a], b,hexes[b]);
		strncat(output, &hexes[a], 1);
		strncat(output, &hexes[b], 1);
	}
}
