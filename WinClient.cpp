#include <winsock2.h>
#include <StdAfx.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <conio.h>

char services[11][20] =
{
	"",
	"Doctors",
	"Psychiatrists",
	"HouseKeeping",
	"BabySitting",
	"Electricians",
	"Plumbers",
	"s5",
	"s6",
	"s5",
	"s6"
};

int determineFirstDayOfYear(int year)
{
	int d1, d2, d3;
	d1 = (year - 1.) / 4.0;
	d2 = (year - 1.) / 100.;
	d3 = (year - 1.) / 400.;
	return (year + d1 - d2 + d3) % 7;
}
int daysInMonth[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
char months[13][20] =
{
	"",
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};
int leap = 0;
char suffixOfDate[10][3] =
{
	"th",
	"st",
	"nd",
	"rd",
	"th",
	"th",
	"th",
	"th",
	"th",
	"th"
};

char day[7][10] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

void checkLeapYr(int y)
{
	if (y % 400 == 0 || y % 4 == 0)
		leap = 1;
}

void printCalenderofMonth(int m, int y, int d)
{
	checkLeapYr(y);
	if (leap == 1)
		daysInMonth[2] = 29;
	int fd1 = determineFirstDayOfYear(y);
	int dim = daysInMonth[m];
	int fd = fd1, l;

	for (l = 1; l <= m - 1; l++)
	{
		fd += daysInMonth[l];
	}

	fd = ((fd + d - 2) % 7);
	int i = 0, j = 0, k = 1, flag = 0;
	printf("%s  %d%s  %s  %d", day[fd], d, ((d / 10 == 1) ? suffixOfDate[0] : suffixOfDate[d % 10]), months[m], y);
	printf(" %s %d\n", months[m], y);
	printf(" SUN MON TUE WED THU FRI SAT\n");
	for (i = 1; i <= 6; i++)
	{
		if (i == 1)
		{
			while (j<(fd)* 4)
			{
				printf(" ");
				j++;
			}
			for (; j<28; (j += 4), k++)
			{
				printf("%4d", k);

			}
		}
		else
		{
			for (j = 0; j<28; j += 4, k++)
			{
				if (k>dim) break;
				printf("%4d", k);
				if (k == dim)
				{
					flag = 1;
					break;
				}
			}

		}
		printf("\n");
		if (k - 1 == dim || flag)
		{
			//printf(31);
			break;
		}
	}
	printf("%d", fd1);
}

int c1 = 0, c2 = 0, c3 = 0, c4 = 0, opt1, opt2;
char* showmenu()
{
	char* buffer = (char*)malloc(1024 * sizeof(char));
	int i = 1;
	if (c1 == 0)
	{
		printf("Our services\n");
		for (; i < 11; i++)
			printf("%s\n", services[i]);
		printf("Please select one\n");
		scanf("%d", &opt1);
		buffer[0] = 1 + '0';
		buffer[1] = opt1 + '0';
		buffer[2] = '\0';
		c1 = 1;
		return buffer;
	}
	printf("Available:\n\n99.Add %s", services[1]);
	scanf("%d", &opt2);
	buffer[0] = 2 + '0';
	if (opt2 == 99)
	{
		buffer[1] = 9 + '0';
		buffer[2] = 9 + '0';
		buffer[3] = opt1 + '0';
		buffer[4] = '\0';
		char buff[16];
		printf("Enter name\n");
		scanf("%s", buff);
		strcat(buffer, buff);
		strcat(buffer, "#");
		printf("Enter role\n");
		scanf("%s", buff);
		strcat(buffer, buff);
		strcat(buffer, "#");
		printf("Enter phone number\n");
		scanf("%s", buff);
		strcat(buffer, buff);
		return buffer;
	}
	else
	{
		buffer[1] = opt1 + '0';
		buffer[2] = opt2 + '0';
		buffer[3] = '\0';
		return buffer;
	}

	if (opt2 != 99)
	{
		printf("Enter date to Schedule an appointment (i/p example : 21/6/2016)\n");
		char date[16];
		scanf("%d", date);
		buffer[0] = 3 + '0';
		buffer[1] = opt1 + '0';
		buffer[2] = opt2 + '0';
		buffer[3] = '\0';
		strcat(buffer, date);
		return buffer;
	}
}


int getsocket()
{
	int hsock;
	int * p_int;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if (hsock == -1){
		printf("Error initializing socket %d\n", WSAGetLastError());
		return -1;
	}

	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if ((setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1) ||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1)){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		return -1;
	}
	free(p_int);

	return hsock;
}



void socket_client()
{

	//The port and address you want to connect to
	int host_port = 1101;
	char* host_name = "127.0.0.1";

	//Initialize socket support WINDOWS ONLY!
	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0 || (LOBYTE(wsaData.wVersion) != 2 ||
		HIBYTE(wsaData.wVersion) != 2)) {
		fprintf(stderr, "Could not find sock dll %d\n", WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set any options

	//Connect to the server
	struct sockaddr_in my_addr;

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(host_port);

	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = inet_addr(host_name);

	//if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR ){
	//	fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
	//	goto FINISH;
	//}

	//Now lets do the client related stuff
	char *buffer = (char*)malloc(sizeof(char) * 1024);
	int buffer_len = 1024;
	int bytecount;
	int c;

	while (true) {

		int hsock = getsocket();
		//add error checking on hsock...
		if (connect(hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR){
			fprintf(stderr, "Error connecting socket %d\n", WSAGetLastError());
			goto FINISH;
		}

		memset(buffer, '\0', buffer_len);
		buffer = showmenu();
		if (buffer == "")
			buffer = "*";
		if ((bytecount = send(hsock, buffer, strlen(buffer), 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			goto FINISH;
		}
		printf("Sent bytes %d\n", bytecount);

		if ((bytecount = recv(hsock, buffer, buffer_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			goto FINISH;
		}
		//printf("Recieved bytes %d\nReceived string \"%s\"\n", bytecount, buffer);
		printf("%s\n", buffer);

		closesocket(hsock);
	}

	//closesocket(hsock);
FINISH:
	;
}