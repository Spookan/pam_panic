/*
FILENAME :     pam_panic_pw.c
DESCRIPTION :  Generates and saves password for pam_panic
AUTHOR :       Bandie, some Author of the glibc manpage
DATE :         2018-03-27T02:34:08+02:00
LICENSE :      GNU-GPLv3
*/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <crypt.h>
#include "config.h"


int writePasswords(char pw[][99]){

  FILE *f = fopen(PPASSFILE, "w");
  if(f == NULL){
    fprintf(stderr, "ERROR opening file!\n");
    return 2;
  }

  fprintf(f, "%s\n%s\n", pw[0], pw[1]);

  fclose(f);

  chmod(PPASSFILE, 0644);

  return 0;
}


int main(void){

  time_t t;
  srand((unsigned) time(&t));
  unsigned long seed[2];
  char salt[] = "$6$........";
  const char *const seedchars =
    "./0123456789ABCDEFGHIJKLMNOPQRST"
    "UVWXYZabcdefghijklmnopqrstuvwxyz";
  char *password[2];
  char *pwvrf[2];
  
  char pw[2][99];
  char pwv[2][99];

  char *prompt[4] = {"Key password: ",  "Panic password: ", "Retype key password: ","Retype panic password: "};

  int i;

  if(getuid() != 0){
    printf("Please run this program under root. Write access to %s is mandatory.\n", PPASSFILE);    return 1;
  }
  
  for(int j=0; j<2; j++){ 

    seed[0] = time(&t);
    seed[1] = rand() ^ (seed[0] >> 14 & 0x30000);
    
    /* Turn it into printable characters from ‘seedchars’. */
    for (i = 0; i < 8; i++)
      salt[3+i] = seedchars[(seed[i/5] >> (i%5)*6) & 0x3f];

    /* Read in the user’s password and encrypt it. */
    password[j] = crypt(getpass(prompt[j]), salt);
  
    strcpy(pw[j], password[j]);

    for(int k=0; k<3; k++){
      pwvrf[j] = crypt(getpass(prompt[j+2]), pw[j]);
      strcpy(pwv[j], pwvrf[j]);
      int ok = strcmp(pw[j], pwv[j]) == 0;
      if(!ok){
        if(k==2){
          printf("Didn't work. Bye.\n");
          return 1;
        }else
          printf("Password didn't match. Try again.\n");
      }else
        break;
    }
  
  }

  /* Save the results. */
  
  return writePasswords(pw);
}
