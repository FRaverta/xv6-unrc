#include "param.h"
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"
#include "syscall.h"
#include "traps.h"
#include "memlayout.h"
#include "rtc.h"


#define err_msg "Use date as follow:\n   _date (ask for current date)\n   _date MM DD hh mm (set month as MM,day of month as DD, hours as hh and minutes as mm)\n"

//convert value in decimal representation to binary representation
static
uchar
to_binary(uchar value)
{
	return (((value & 0xF0) >> 4) *10) + (value & 0x0F);
}

//Obtain a day name from a day number in decimal version
static
char* 
get_day(uchar day_of_week)
{
	switch(day_of_week){		
		case 1: return "Sunday";
		case 2: return "Monday";
		case 3: return "Tuesday";
		case 4: return "Wednesday";
		case 5: return "Thursday";
		case 6: return "Friday";
		case 7: return "Saturday";
		default: return "Error";
	}
}

//Obtain a month name from a day number in decimal version
static
char* 
get_month(uchar month)
{
	month= to_binary(month);
	switch(month){
		case 1: return "January";
		case 2: return "February";
		case 3: return "March";
		case 4: return "April";
		case 5: return "May";
		case 6: return "June";
		case 7: return "July";
		case 8: return "Agoust";
		case 9: return "September";
		case 10: return "October";
		case 11: return "November";
		case 12: return "December";
		default: return "Error";
	}
}

//check if a date is valid
static
int 
is_valid(struct time* date){
	uchar month  = to_binary(date->month);
	uchar day_of_month  = to_binary(date->day_of_month);
	uchar hours  = to_binary(date->hours);
	uchar minutes  = to_binary(date->minutes);
	uchar year = to_binary(date->year);

	if(month<0 || month>12)
		return 0;
	if(day_of_month < 0)
		return 0;
	if(hours<0 || hours>24)
		return 0;
	if(minutes<0 || minutes>60)
		return 0;
	switch(month){
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:
				if(day_of_month>31)
					return 0;
				break;
		case 4:
		case 6:
		case 9:
		case 11:
				if(day_of_month>30)
					return 0;
				break;
		case 2:
				{
					int days_on_f=28;
					
					if ((year%4 == 0 && year % 100 != 0) || year % 400 == 0)
						days_on_f=29;
					if(day_of_month > days_on_f)
						return 0;
					break;
				}
	}
	return 1;
}

//auxiliary function for parse number from char to int in decimal version
static
uchar
twotoint(char* entry)
{
	uchar result=0;
	char current[2] = {0,0};

	current[0] = *entry;
	result = (uchar) atoi(current);
	result = result << 4;
	current[0] = *(entry+1);
	result = result | (uchar) atoi(current);
	
	return result;
}

//parse a string like MMDDhhmm into struct time.
static
int
parse_time(char* entry,struct time* time)
{	
	if (strlen(entry) != 8)
		return -1;

	time->month= twotoint(entry);
	time->day_of_month=twotoint(entry+2);
	time->hours=twotoint(entry+4);
	time->minutes=twotoint(entry+6);

	return 0;
}

/*
Main method for program date. use it like: 
	_ "date" for consult current date
	_ "date MMDDhhmm for set month(MM), day(DD), hh (hours) and minute(mm)"
*/
int 
main(int argc,char *argv[])
{
	int fd;
	struct time time;

	if(argc == 1){
		fd=open("rtc",O_RDWR);
		read(fd,&time,1);
		close(fd);
		printf(1,"%s %s %d %d:%d:%d\n",get_day(time.day_of_week), get_month(time.month), to_binary(time.day_of_month),to_binary(time.hours),to_binary(time.minutes),to_binary(time.seconds));
	}else{ 
			if(argc == 2){
				fd=open("rtc",O_RDWR);
				read(fd,&time,1);																
				if (parse_time(argv[1],&time) == 0){												
					if(is_valid(&time))
						write(fd,&time,1);
					else
						printf(1,"The date isn't valid.\n");
					close(fd);						
				}else					
					printf(1,err_msg);
			}else
				printf(1,err_msg);				
	}
	exit();
}

