#include "types.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "file.h"
#include "x86.h"
#include "user/rtc.h"
#include "spinlock.h"




#define RTC_PORT(x)		(unsigned short)(0x70 + (x))
#define RTC_SECONDS	0
#define RTC_MINUTES	2
#define RTC_HOURS	4
#define RTC_WEEKDAY 6
#define RTC_MONTHDAY 7
#define RTC_MONTH 8
#define RTC_YEAR 9

struct spinlock lock;


static 
uchar
get_rtc(uchar addr)
{
    acquire(&lock);
    uchar val;
    outb(RTC_PORT(0),addr);
    val=inb(RTC_PORT(1));
    release(&lock);
    return val;

}

static 
void
set_rtc(uchar addr,uchar value)
{
    acquire(&lock);
    outb(RTC_PORT(0),addr);
    outb(RTC_PORT(1),value);
    release(&lock);

}

//write rtc fields on decimal version
int
rtcwrite(struct inode *ip, char *buf, int n)
{
  //cast char *buf to struct time
  struct time d = *((struct time*)buf);

  /* set RTC */
  set_rtc(RTC_SECONDS,d.seconds);
  set_rtc(RTC_MINUTES,d.minutes);
  set_rtc(RTC_HOURS,d.hours);
  set_rtc(RTC_MONTHDAY,d.day_of_month);
  set_rtc(RTC_MONTH,d.month);
  set_rtc(RTC_YEAR,d.year);

  return 1;
}

//read rtc fields on decimal version
int
rtcread(struct inode *ip, char *dst, int n)
{
  struct time d;

  /* Get the current time from RTC */
  d.seconds = get_rtc(RTC_SECONDS);
  d.minutes = get_rtc(RTC_MINUTES);
  d.hours   = get_rtc(RTC_HOURS);
  d.day_of_week  = get_rtc(RTC_WEEKDAY);
  d.day_of_month = get_rtc(RTC_MONTHDAY);
  d.month = get_rtc(RTC_MONTH);
  d.year = get_rtc(RTC_YEAR);

  *((struct time*)dst) = d; 

  return 0;
}

//Init rtc clock
void
rtcinit(void)
{
  initlock(&lock, "rtc");
  devsw[RTC].write = rtcwrite;
  devsw[RTC].read = rtcread;
}






