#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define MAX_STR_LEN       32
#define MIN_LOGIN_STR_LEN 4
#define MIN_PASS_STR_LEN  12

typedef unsigned char byte;

const unsigned log_bias  = 0xfec0135a;
const unsigned pass_bias = 0xdeadbeef;

const uint64_t byte_masks_64b[] = { 0xFF00000000000000,
				    0x00FF000000000000,
				    0x0000FF0000000000,
                                    0x000000FF00000000,
				    0x00000000FF000000,
				    0x0000000000FF0000,
				    0x000000000000FF00,
				    0x00000000000000FF };

//
byte randxy(byte x, byte y)
{ return (rand() % (y - x)) + x; }

//
byte is_digit(char c)
{ return (c >= '0' && c <= '9'); }

//
byte is_low_alpha(char c)
{ return (c >= 'a' && c <= 'f'); }

//
byte is_upper_alpha(char c)
{ return (c >= 'A' && c <= 'F'); }

//
void check_str_len(char *str, unsigned len)
{
  if (strlen(str) < len)
    printf("Error, incorrect length.\n"), exit(-1);
}

//
byte get_byte(uint64_t val, unsigned pos)
{ return (val & byte_masks_64b[pos]) >> (64 - ((pos + 1) << 3)); }

//Builds a hex string  
void set_last_byte(byte b, uint64_t *val)
{
  unsigned tmp = b;

  *val = ((*val) << 8) | b;
}

//
uint64_t str_to_hex(char *str)
{
  byte b = 0;
  uint64_t val = 0x0000000000000000;
  
  for (int i = 0; i < strlen(str); i += 2)
    {
      b = (is_digit(str[i])) ? str[i] - '0' : (is_low_alpha(str[i])) ? str[i] - 'W' : str[i] - '7';
      b = (b << 4) | ((is_digit(str[i + 1])) ? str[i + 1] - '0' : (is_low_alpha(str[i + 1])) ? str[i + 1] - 'W' : str[i + 1] - '7');
      
      set_last_byte(b, &val);
    }

  return val;
}

//
unsigned get_str_checksum(char *str, unsigned n)
{
  unsigned sum = 0;

  for (int i = 0; i < n; i++)
    sum += str[i];
  
  return sum;
}

//
int main(int argc, char **argv)
{
  srand(time(NULL)); //Generate a seed based on time. 
  
  byte login[MAX_STR_LEN];
  byte password[MAX_STR_LEN];
  
  printf("Login: ");
  scanf("%s", login);

  printf("Password: ");
  scanf("%s", password);
  
  check_str_len(login, MIN_LOGIN_STR_LEN);
  check_str_len(password, MIN_PASS_STR_LEN);

  byte b;
  unsigned log_chunk;
  uint64_t pass_hex = str_to_hex(password);
  unsigned log_checksum = get_str_checksum(login, strlen(login));
  
  log_chunk = login[0]; //First character
  log_chunk = (log_chunk << 8) | login[1]; //Second character
  log_chunk = (log_chunk << 8) | login[strlen(login) - 2]; //Before last character
  log_chunk = (log_chunk << 8) | login[strlen(login) - 1]; //Last character

  uint64_t hex_pass = 0;
  unsigned pass_magicnumber = 0xadde;
  unsigned log_magicnumber = log_chunk ^ log_checksum ^ log_bias;
  
  for (int i = 0; i < 8; i++)
    {
      b = get_byte(pass_hex, i);
      
      if (b == 0 && (i == 0 || i == 1)) //If left bytes are null, do nothing. Size limit of 6 bytes!
	;
      else
	pass_magicnumber = b ^ ((pass_magicnumber << 5) + 1) ^ pass_bias;
    }
  
  printf("Login    check : 0x%04x\n", log_magicnumber);  
  printf("Password check : 0x%04x\n", pass_magicnumber);

  if (!(log_magicnumber ^ pass_magicnumber))
    printf("Check success :)\n");
  else
    printf("Check fail :(\n");
  
  return 0;
}
