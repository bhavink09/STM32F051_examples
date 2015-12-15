#include "stm32f0xx_conf.h"

#include "at_cmd.h"
#include "usart.h"
#include "delay.h"
#include "string.h"

#define AT_BUFFER_SIZE 128
#define AT_NEWLINE "\r"
#define AT_CMD "AT"

uint8_t at_buffer[AT_BUFFER_SIZE];

int8_t at_send_dummy()
{
  usart2_puts("\r\n\r\n");
  return AT_SUCCESS;
}

int8_t at_direct_send(const char* data)
{
  usart2_flush_queue();
  usart2_puts(data);
  return AT_SUCCESS;
}

int8_t at_check_ok(uint32_t timeout)
{
  uint32_t cnt = 0;
  char rcv[2];
  
  while(cnt < timeout)
  {
    rcv[0] = rcv[1];
    rcv[1] = usart2_getch();
    if(rcv[0] == 'O' && rcv[1] == 'K')
    {
      return AT_SUCCESS;
    }
    ++cnt;
  }
  return AT_ERROR;
}

// Waits until you get a \r\n\r\n
int8_t at_wait_till_double_newline(uint32_t max_size)
{
  char tmp_buf[4];
  uint8_t is_match;
  uint32_t cnt;
  uint32_t i;
  
  while(!is_match && cnt < max_size)
  {
    tmp_buf[cnt % sizeof(tmp_buf)] = usart2_getch();    
    i = (cnt + 1) % sizeof(tmp_buf);    
    ++cnt;
    if(tmp_buf[i] != '\r') continue;
    
    i = (i + 1) % sizeof(tmp_buf);
    if(tmp_buf[i] != '\n') continue;
    
    i = (i + 1) % sizeof(tmp_buf);
    if(tmp_buf[i] != '\r') continue;
    
    i = (i + 1) % sizeof(tmp_buf);
    if(tmp_buf[i] != '\n') continue;        
    
    is_match = 1;
  }
  return AT_SUCCESS;
}

int8_t at_send_receive(char* send_buf, char* rcv_buf, uint32_t rcv_buf_size)
{
  uint32_t cnt = 0;
  int32_t send_size = strlen(send_buf);
    
  usart2_flush_queue();
  usart2_puts(AT_CMD);
  usart2_puts(send_buf);
  usart2_puts(AT_NEWLINE);
    
  cnt = 0;
  while(cnt < rcv_buf_size)
  {
    rcv_buf[cnt] = usart2_getch();    
    if(cnt >= 1)
    {
      if(!strncmp(&(rcv_buf[cnt - 1]), "OK", 2))
      {
        return AT_SUCCESS;
      }      
    }
    
    if(cnt >= 4)
    {
      if(!strncmp(&(rcv_buf[cnt - 4]), "ERROR", 5))
      {
        return AT_ERROR;
      }      
    }
    
    if(cnt >= 3)
    {
      if(!strncmp(&rcv_buf[cnt - 3], "FAIL", 4))
      {
        return AT_ERROR;
      }
    }
    ++cnt;
  }
  return AT_ERROR;
}
