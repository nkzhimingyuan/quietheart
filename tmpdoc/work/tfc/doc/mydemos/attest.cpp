#include <iostream>
using namespace std;
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <pthread.h>
#define FALSE -1
#define TRUE 0
int fd;
int nquit;

int speed_arr[] = { B115200, B38400, B19200, B4800, B2400, B1200, B300 };
int name_arr[] = { 115200, 38400, 19200, 4800, 2400, 1200, 300 };

void set_speed(int fd, int speed)
{
	int i;
	int status;
	struct termios Opt;
	tcgetattr(fd, &Opt);
	for(i = 0; i<sizeof(speed_arr)/sizeof(int); i++){
		if (speed==name_arr[i]){
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0){
				perror("tcsetattr fd");
			    return;
			}
			tcflush(fd, TCIOFLUSH);
		}
	}
}

int set_Parity( int fd, int databits, int stopbits, int parity)
{
    struct termios opt;
    if( tcgetattr(fd, &opt)  !=  0)
    {
  	perror("SetupSerial 1");
  	return(FALSE);
    }
    opt.c_cflag &= ~CSIZE;
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG ); 
	opt.c_oflag &= ~OPOST;
    switch (databits)
    {
 	case 5:
  		opt.c_cflag |= CS5;
  		break;
        case 6:
  		opt.c_cflag |= CS6;
  		break;
        case 7:
  		opt.c_cflag |= CS7;
  		break;
  	case 8:
		opt.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return (FALSE);
    }
    switch (parity)
    {
        case 'n':
        case 'N':
		opt.c_cflag &= ~PARENB;   
		opt.c_iflag &= ~INPCK;     
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= (PARODD | PARENB);  /* parity checking */ 
		opt.c_iflag |= INPCK;      /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= PARENB;     /* Enable parity */
		opt.c_cflag &= ~PARODD;   /*  */  
		opt.c_iflag |= INPCK;       /* Disnable parity checking */
        case 'S':
	case 's':  
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return (FALSE);
    }
       
    switch (stopbits)
    {
    case 1:
  		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return (FALSE);
	}

    if (parity != 'n')
     opt.c_iflag |= INPCK;
	 tcflush(fd, TCIFLUSH); 
     opt.c_cc[VTIME] =100; //  mseconds
     opt.c_cc[VMIN] = 0;
 
    if (tcsetattr(fd, TCSANOW, &opt) != 0)
    {
  	    perror("SetupSerial 3");
    	return (FALSE);
    }
    return (TRUE);
}
int		
OpenDev( char *Dev)
{ int fd = open( Dev, O_RDWR );
  if (-1 == fd)
  { perror("Can't Open Serial Port");
    return -1;
  }
  else 
  { printf("Open OK");
    return fd;} 
}

void *rdThr(void *argc)
{
    char buffer[10];
    while(nquit)
    {
        read( fd, buffer, sizeof(buffer) );
	cout<<buffer<<endl;
	usleep(500);
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	
	int nread;
	char dev[32];
	int i;
	pthread_t Read;

    strcpy(dev, argv[1]);
	fd = OpenDev(dev);
	set_speed(fd, 115200);
	if (set_Parity(fd, 8, 1, 'N') == FALSE)
	{printf("Set Parity Error\n");
	 exit (0);
	}
	else printf("Set Parity OK\n");
    char str[10]; 
    //char buffer[10];
        
        nquit = 1;
	pthread_create( &Read, NULL, rdThr, NULL );
    
    
	while (nquit)
	{   
	        //cout<<"please input AT..."<<endl;
        cout<<">>>"<<flush;
		cin>>str;
		//strcpy(str,"ATDT2\r\n");
		//cout<<"write "<<str<<endl;
		if( !strcmp(str,"break") ) 
		{
		    nquit = 0;
		    break;
		    
		}
		if( write( fd, strcat(str,"\r\n"), sizeof(str)+2 ) < 0 ) cout<<"error"<<endl;
		//cout<<"a"<<endl;
		usleep(5000);
		//nquit = 0;
		//break;
        
	}
	pthread_join(Read,NULL);
    close(fd);
}
