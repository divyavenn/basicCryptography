#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/aes.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
 
#include <arpa/inet.h>  
#include <string.h>
#define PORT 12000

RSA *createRSA(unsigned char *key, int public){
	RSA *rsa = NULL;
  	BIO *keybio;
  	keybio = BIO_new_mem_buf(key, -1);
  	if (keybio == NULL){
		printf("Failed to create key BIO");
    		return 0;
  	}
  	if (public) rsa = PEM_read_bio_RSA_PUBKEY(
					keybio, 
					&rsa, 
					NULL, 
					NULL);
  	else rsa = PEM_read_bio_RSAPrivateKey(
					keybio, 
					&rsa, 
					NULL, 
					NULL);
  	if (rsa == NULL)printf("Failed to create RSA");
  	return rsa;
}


int main(int argc , char *argv[]){
	
	struct sockaddr_in addr;
	int sk;
	//create socket
	sk = socket(AF_INET,SOCK_STREAM,0);
	if (sk == -1){
		printf("Could not create socket");
		return -1;
	}
	//set to local IP
	if(inet_pton(AF_INET, 
			"127.0.0.1", 
			&addr.sin_addr)<=0){ 
        	printf("\nInvalid address\n"); 
        	return -1; 
    	} 
	//set TCP mode
	addr.sin_family = AF_INET;
	//set port #
	addr.sin_port = htons(PORT);
	
	//attempt connection
    	if (connect(sk,
		(struct sockaddr*)(&addr),	
		sizeof(addr)) < 0) { 
        		printf("\nConnection Failed \n"); 
        		return -1; 
    	}
	else printf("Connected! \n");

	//Recieve Public key size
	int32_t pKeyLen;
	char* data = (char*)&pKeyLen;
	if (read(sk, data, 4) < 0) printf("Reception failed \n");
	printf("%i \n", pKeyLen);
	
	//receive public key
	char pKey[pKeyLen];
	data = pKey;
	if (read(sk, data, pKeyLen) < 0) printf("Reception failed \n");
	else{
		for(int i = 0; i<pKeyLen; i++){
			printf("%c", pKey[i]);
		}
	}
	
		
	int32_t AESkeyLen = 16;
	//unsigned char AESkey[16] = {'b', '1', 'c', 
	//			's', 'f', 'g', 
	//			'f', 'f', 'g', 
	//			'h', '3', '5', 
	//			'5', '5', '4', 'f'};
	unsigned char AESkey[16] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
				0x07, 0x08, 0x09, 0x10, 0x11, 0x12, 0x13,
				0x14, 0x15, 0x16};

	char en[8192];
	unsigned char* toEncrypt = AESkey;
	unsigned char* encrypted = en;

	//encrypt our AES key using server's public key
	RSA *rsa = createRSA(pKey, 1);
	int padding = RSA_PKCS1_PADDING;
	int length = RSA_public_encrypt(16,
			toEncrypt,
			encrypted, 
		  	rsa,
			padding);
	

	//send length of encrypted AES key 
	if (send(sk, &length, sizeof(length), 0) < 0){
		printf("Failed to send encrypt key length \n");
	}


	//send encrupted AES key
	if (send(sk, encrypted, length, 0) < 0){
		printf("Failed to send encrypted key \n");
	}
	


	//recieved AES encrypted secret message
	int n = 16;
 	char buf[n];	
	data = buf;
	if (read(sk, data, n) < 0) printf("Reception failed \n");
	printf("Encrypted sercret message I get \n");
	for (int i = 0; i<n; i++){
		printf("%02x", buf[i]);
	}
	//decrypt secret message
	char de[n];
	unsigned char* toDecrypt = buf;
	unsigned char* decrypted = de;	
	AES_KEY *expanded;
	expanded = (AES_KEY *)malloc(sizeof(AES_KEY));
	AES_set_decrypt_key(AESkey, 128, expanded);
	AES_decrypt(toDecrypt, decrypted, expanded);
	printf("Decrypted secrete message \n");
	for (int i = 0; i<n; i++){
		printf("%02x \n", de[i]);
	}
	
	//Translate from ASCII to char
	for (int i = 0; i<n; i++){
		
	}	
		
	return 0;

}
