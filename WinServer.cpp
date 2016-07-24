#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#define _CRT_SECURE_NO_WARNINGS
struct services
{
	int sID;
	int s_person[5], s_indirect, d_indirect;
};

int s_person_count = 0;

struct s_person
{
	int s_id;
	char s_name[16];
	char role[16];
	char phone_no[16];
	int dt[5], s_indirect, d_indirect;
};

struct date
{
	char dt[16];
	int user_id;
};

int calbitvector[2000000];

struct node
{
	char msg[128];
	int msg_id;
	node *next;
}*flist,*alist,*printid;

struct bufserv{
	
		int userId;
		int forumId;
		int msgId;
		int commentId;
		int choice;
		char *forumname;
		char msg[128];
}buf1;

bool flag=true;
int mid = 0;
int count1 =0;
char *Data[100];
int count=1;
int values[100];
void replyto_client(char *buf, int *csock);
void SocketHandler(void *);

void writecal()
{
	FILE* fcal = fopen("calendar.bin", "r+b");
	fseek(fcal, 0, SEEK_SET);
	for (int i = 0; i < 10; i++)
	{
		struct services s;
		s.s_person[0] = -1;
		s.s_person[1] = -1;
		s.s_person[2] = -1;
		s.s_person[3] = -1;
		s.s_person[4] = -1;
		s.s_indirect = -1;
		s.d_indirect = -1;
		s.sID = i + 1;
		fwrite(&s, sizeof(s), 1, fcal);
	}
	fseek(fcal, 10000000, SEEK_SET);
	for (int i = 0; i < 1000000; i++)
		calbitvector[i] = 0;
	fwrite(&calbitvector, sizeof(calbitvector), 1, fcal);
	fseek(fcal, 1000, SEEK_SET);
	for (int i = 0; i < 1000000; i++)
	{
		struct date dt;
		fwrite(&dt, sizeof(dt), 1, fcal);
	}
	//person struct -> 20000000
}

void socket_server() {
	writecal();
	//The port you want the server to listen on
	int host_port= 1101;

	unsigned short wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD( 2, 2 );
 	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 || ( LOBYTE( wsaData.wVersion ) != 2 ||
		    HIBYTE( wsaData.wVersion ) != 2 )) {
	    fprintf(stderr, "No sock dll %d\n",WSAGetLastError());
		goto FINISH;
	}

	//Initialize sockets and set options
	int *hsock = (int*)malloc(sizeof(int));
	int * p_int ;
	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if(hsock == -1){
		printf("Error initializing socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	p_int = (int*)malloc(sizeof(int));
	*p_int = 1;
	if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
		(setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
		printf("Error setting options %d\n", WSAGetLastError());
		free(p_int);
		goto FINISH;
	}
	free(p_int);

	//Bind and listen
	struct sockaddr_in my_addr;
	my_addr.sin_family = AF_INET ;
	my_addr.sin_port = htons(host_port);
	
	memset(&(my_addr.sin_zero), 0, 8);
	my_addr.sin_addr.s_addr = INADDR_ANY ;
	
	/* if you get error in bind 
	make sure nothing else is listening on that port */
	if( bind( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
		fprintf(stderr,"Error binding to socket %d\n",WSAGetLastError());
		goto FINISH;
	}
	if(listen( hsock, 10) == -1 ){
		fprintf(stderr, "Error listening %d\n",WSAGetLastError());
		goto FINISH;
	}
	
	//Now lets do the actual server stuff
	SocketHandler(hsock);
	/*int* csock;
	sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	
	while(true){
		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));
		
		if((*csock = accept( hsock, (SOCKADDR*)&sadr, &addr_size))!= INVALID_SOCKET ){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			SocketHandler((void*)csock);
		}
		else{
			fprintf(stderr, "Error accepting %d\n",WSAGetLastError());
		}
	}*/

FINISH:
;
}



void process_input(char *recvbuff, int recv_buf_cnt, int* csock) 
{
	FILE* fcal = fopen("calendar.bin", "r+b");
	char replybuff[1024]={'\0'};
	printf("%s",recvbuff);
	if (recvbuff[0] - '0' == 1)
	{
		struct services s;
		fseek(fcal, (recvbuff[1] - '0') * 32, SEEK_SET);
		fread(&s, sizeof(s), 1, fcal);
		int i = 0;
		while (s.s_person[i] != -1)
		{
			struct s_person sp;
			fseek(fcal, 20000000 + s.s_person[i]*80, SEEK_SET);
			strcat(replybuff, sp.s_name);
			strcat(replybuff, "\t");
			strcat(replybuff, sp.role);
			strcat(replybuff, "\t");
			strcat(replybuff, sp.phone_no);
			strcat(replybuff, "\n");
		}
	}
	else if (recvbuff[0] - '0' == 2)
	{
		if (recvbuff[1] == '9' && recvbuff[2] == '9')
		{
			struct s_person sp;
			int i = 4, j = 0 ;
			while (recvbuff[i] != '#')
			{
				sp.s_name[j++] = recvbuff[i];
				i++;
			}
			sp.s_name[j] = '\0';
			i++; j = 0;
			while (recvbuff[i] != '#')
			{
				sp.role[j++] = recvbuff[i];
				i++;
			}
			sp.role[j] = '\0';
			i++; j = 0;
			while (recvbuff[i] != '#')
			{
				sp.phone_no[j++] = recvbuff[i];
				i++;
			}
			sp.phone_no[j] = '\0';
			i = 0;
			while (i != 5)
				sp.dt[i++] = -1;
			fseek(fcal, 20000000 + s_person_count * 80, SEEK_SET);
			fwrite(&sp, sizeof(sp), 1, fcal);
			struct services s;
			fseek(fcal, (recvbuff[3] - '0') * 32, SEEK_SET);
			fread(&s, sizeof(s), 1, fcal);
			while (s.s_person[i] != -1)
				i++;
			s.s_person[i] = s_person_count;
			fseek(fcal, (recvbuff[3] - '0') * 32, SEEK_SET);
			fwrite(&s, sizeof(s), 1, fcal);
			replybuff[0] = '*';
			replybuff[1] = '\0';
		}
		else
		{
			int s = recvbuff[1] - '0';
			int sp = recvbuff[2] - '0';
			struct services ss;
			struct s_person spp;
			struct date d;
			fseek(fcal, s * 32, SEEK_SET);
			fread(&ss, sizeof(ss), 1, fcal);
			fseek(fcal, 20000000 + ss.s_person[sp] * 80, SEEK_SET);
			fread(&ss, sizeof(ss), 1, fcal);
			int i = 0;
			while (spp.dt[i] != -1)
				i++;
			int j = 0;
			fseek(fcal, 10000000 , SEEK_SET);
			fread(&calbitvector, sizeof(calbitvector), 1, fcal);
			while(calbitvector[j] != -1) j++;
			calbitvector[j] = 1;
			fwrite(&calbitvector, sizeof(calbitvector), 1, fcal);
			fseek(fcal, j * 1000, SEEK_SET);
			int k = 3;
			j = 0;
			while (recvbuff[k] != '/0')
			{
				d.dt[j++] = recvbuff[k];
				k++;
			}
			d.dt[j] = '\0';
			fwrite(&d, sizeof(d), 1, fcal);
		}
	}
	else if (recvbuff[0] - '0' == 3)
	{

	}
	else if (recvbuff[0] - '0' == 4)
	{

	}
	else if (recvbuff[0] - '0' == 5)
	{

	}
	else if (recvbuff[0] - '0' == 6)
	{

	}
	else if (recvbuff[0] - '0' == 8)
	{

	}
	else if (recvbuff[0] - '0' == 9)
	{

	}
	
	replyto_client(replybuff, csock);
	replybuff[0] = '\0';
}

void replyto_client(char *buf, int *csock) {
	int bytecount;
	if (buf[0] == '\0')
		buf = "*";
	if((bytecount = send(*csock, buf, strlen(buf), 0))==SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free (csock);
	}
	printf("replied to client: %s\n",buf);
}

struct reply
{
	int r[5];
};

struct reply rpl[600];

int bitvector[50000];

struct Msgs
{
	char msg[128];
	int uid;
};

struct Msgs msgs[50000];

struct Catg
{
	int catg1;
	char cat1[32];
	int catg2;
	char cat2[32];
	int catg3;
	char cat3[32];
	int catg4;
	char cat4[32];
	int catg5;
	char cat5[32];
};

struct user
{
	char username[32];
	int uid;
};

struct Offset
{
	int offset1[30];
	int offset2[30];
	int offset3[30];
	int offset4[30];
	int offset5[30];
};

void write(FILE* fp)
{

	int i = 0, j = 0, k;
	for (i = 0; i<20; i++)
	{
		fseek(fp, i * 40, SEEK_SET);
		struct user u;
		u.uid = -1;
		fwrite(&u, sizeof(struct user), 1, fp);
	}

	for (i = 0; i<20; i++)
	{
		fseek(fp, 1000 + i * 200, SEEK_SET);
		struct Catg c;
		c.catg1 = -1;
		c.catg2 = -1;
		c.catg3 = -1;
		c.catg4 = -1;
		c.catg5 = -1;
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}

	for (i = 0; i<20; i++)
	{
		fseek(fp, 10000 + i * 600, SEEK_SET);
		struct Offset o;
		for (j = 0; j<30; j++)
			o.offset1[j] = -1;
		for (j = 0; j<30; j++)
			o.offset2[j] = -1;
		for (j = 0; j<30; j++)
			o.offset3[j] = -1;
		for (j = 0; j<30; j++)
			o.offset4[j] = -1;
		for (j = 0; j<30; j++)
			o.offset5[j] = -1;
		fwrite(&o, sizeof(struct Offset), 1, fp);
	}

	for (i = 0; i<600; i++)
		for (j = 0; j<5; j++)
			rpl[i].r[j] = -1;

	fseek(fp, 26000, SEEK_SET);
	fwrite(&rpl, sizeof(rpl), 1, fp);

	fseek(fp, 40000, SEEK_SET);
	for (i = 0; i<50000; i++)
		bitvector[i] = -1;
	fwrite(&bitvector, sizeof(bitvector), 1, fp);

	fseek(fp, 300000, SEEK_SET);
	fwrite(&msgs, sizeof(msgs), 1, fp);
	fclose(fp);
}

void read(FILE* fp)
{

	int i = 0, j = 0, k;
	for (i = 0; i<20; i++)
	{
		fseek(fp, i * 40, SEEK_SET);
		struct user u;
		fread(&u, sizeof(struct user), 1, fp);
		printf("%d\n", u.uid);
	}

	for (i = 0; i<20; i++)
	{
		fseek(fp, 1000 + i * 200, SEEK_SET);
		struct Catg c;
		fread(&c, sizeof(struct Catg), 1, fp);
		printf("%d", c.catg1);

	}

	for (i = 0; i<20; i++)
	{
		fseek(fp, 10000 + i * 600, SEEK_SET);
		struct Offset o;
		fread(&o, sizeof(struct Offset), 1, fp);
		printf("%d", o.offset1[20]);
		printf("%d", o.offset2[20]);
		printf("%d", o.offset3[20]);
		printf("%d", o.offset4[20]);
		printf("%d", o.offset5[20]);
	}

	fclose(fp);
}


void addUser(FILE *fp)
{
	char usr[32];
	int i;
	printf("Enter a name\n");
	scanf("%s", usr);
	for (i = 0; i<20; i++)
	{
		fseek(fp, i * 40, SEEK_SET);
		struct user u;
		fread(&u, sizeof(struct user), 1, fp);
		if (u.uid == -1)
		{
			u.uid = i + 1;
			strcpy(u.username, usr);
			fseek(fp, i * 40, SEEK_SET);
			fwrite(&u, sizeof(struct user), 1, fp);
			break;
		}
	}
}

void addCatg(int uid, FILE* fp)
{
	printf("Enter category name\n");
	char ct[32];
	scanf("%s", ct);
	fseek(fp, 1000 + (uid - 1) * 200, SEEK_SET);
	struct Catg c;
	fread(&c, sizeof(struct Catg), 1, fp);
	if (c.catg1 == -1)
	{
		c.catg1 = 1;
		strcpy(c.cat1, ct);
		fseek(fp, 1000 + (uid - 1) * 200, SEEK_SET);
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}
	else if (c.catg2 == -1)
	{
		c.catg2 = 2;
		strcpy(c.cat2, ct);
		fseek(fp, 1000 + (uid - 1) * 200 + 36, SEEK_SET);
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}
	else if (c.catg3 == -1)
	{
		c.catg3 = 3;
		strcpy(c.cat3, ct);
		fseek(fp, 1000 + (uid - 1) * 200 + 36 * 2, SEEK_SET);
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}
	else if (c.catg4 == -1)
	{
		c.catg4 = 4;
		strcpy(c.cat4, ct);
		fseek(fp, 1000 + (uid - 1) * 200 + 36 * 3, SEEK_SET);
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}
	else
	{
		c.catg5 = 5;
		strcpy(c.cat5, ct);
		fseek(fp, 1000 + (uid - 1) * 200 + 36 * 4, SEEK_SET);
		fwrite(&c, sizeof(struct Catg), 1, fp);
	}
}

int checkEmptyIndex(int bitvector[], FILE* fp)
{
	int i, j;
	fseek(fp, 40000, SEEK_SET);
	for (i = 0; i<50000; i++)
	{
		fread(&j, sizeof(i), 1, fp);
		if (j == -1)
			return i;
	}
	return i;
}

int getOffsetIDreply(int uid, int cid, int mid, int idx, FILE* fp)
{
	int i, j = 0;
	fseek(fp, (26000 + (uid - 1) * 200 + (cid - 1) * 40 + (mid - 1) * 20), SEEK_SET);
	while (1)
	{
		fread(&i, sizeof(i), 1, fp);
		if (i == -1)
		{
			fseek(fp, (26000 + (uid - 1) * 200 + (cid - 1) * 40 + (mid - 1) * 20 + i * 4), SEEK_SET);
			fwrite(&idx, sizeof(idx), 1, fp);
			break;
		}
		j += 1;
	}
	return 1;
}

int getOffsetID(int uid, int cid, FILE* fp)
{
	int i, j = 0;
	fseek(fp, 10000 + (uid - 1) * 600 + (cid - 1) * 120, SEEK_SET);
	while (1)
	{
		fread(&i, sizeof(i), 1, fp);
		if (i == -1)
			return j;
		j += 1;
	}
}

void LinkMsgToCid(int uid, int cid, int ofs, int idx, FILE* fp)
{
	fseek(fp, 10000 + (uid - 1) * 600 + (cid - 1) * 120 + ofs * 4, SEEK_SET);
	fwrite(&idx, sizeof(idx), 1, fp);
}

void addMsg(int uid, int cid, FILE* fp)
{
	char buff[128];
	int k = 1;
	//gets(buff);
	scanf("%s", buff);
	int idx = checkEmptyIndex(bitvector, fp);
	fseek(fp, 300000 + (idx * 132), SEEK_SET);
	fwrite(&buff, sizeof(buff), 1, fp);
	fwrite(&uid, sizeof(uid), 1, fp);
	fseek(fp, 40000 + (idx * 4), SEEK_SET);
	fwrite(&k, sizeof(k), 1, fp);
	int offsetID = getOffsetID(uid, cid, fp);
	LinkMsgToCid(uid, cid, offsetID, idx, fp);
}

void addReply(int uid, int cid, int mid, FILE* fp)
{
	char buff[128];
	int k = 1;
	//gets(buff);
	scanf("%s", buff);
	int idx = checkEmptyIndex(bitvector, fp);
	fseek(fp, 300000 + (idx * 132), SEEK_SET);
	fwrite(&buff, sizeof(buff), 1, fp);
	fwrite(&uid, sizeof(uid), 1, fp);
	fseek(fp, 40000 + (idx * 4), SEEK_SET);
	fwrite(&k, sizeof(k), 1, fp);
	int offsetID = getOffsetIDreply(uid, cid, mid, idx, fp);
}

char* showUsers(FILE* fp)
{
	char *replybuff = (char*)malloc(sizeof(char) * 1024);
	replybuff[0] = 'c';
	replybuff[1] = '\0';
	int i, j;
	for (i = 0; i<20; i++)
	{
		fseek(fp, i * 40, SEEK_SET);
		struct user u;
		fread(&u, sizeof(struct user), 1, fp);
		if (u.uid == -1)
			break;
		strcat(replybuff, u.username);
	}
	strcat(replybuff, "45.Select a user\n55.Add a user\n65.Delete a user\n95.Exit\n");
	return replybuff;
}

char* showCatg(int uid, FILE* fp)
{
	char *replybuff = (char*)malloc(sizeof(char) * 1024);
	replybuff[0] = 'c';
	replybuff[1] = '\0';
	fseek(fp, 1000 + (uid - 1) * 200, SEEK_SET);
	struct Catg c;
	fread(&c, sizeof(struct Catg), 1, fp);
	if (c.catg1 != -1)
		strcat(replybuff, c.cat1);
	if (c.catg2 != -1)
		strcat(replybuff, c.cat2);
	if (c.catg3 != -1)
		strcat(replybuff, c.cat3);
	if (c.catg4 != -1)
		strcat(replybuff, c.cat4);
	if (c.catg5 != -1)
		strcat(replybuff, c.cat5);
	strcat(replybuff, "45.Select a category\n55.Add a category\n65.Delete a category\n95.Exit\n");
	return replybuff;
}

char* showmsgs(int uid, int cid, FILE* fp)
{
	char *replybuff = (char*)malloc(sizeof(char) * 1024);
	replybuff[0] = 'm';
	replybuff[1] = '\0';
	int idx, i = 0;
	while (1)
	{
		fseek(fp, 10000 + ((uid - 1) * 600) + ((cid - 1) * 120) + i * 4, SEEK_SET);
		fread(&idx, sizeof(idx), 1, fp);
		if (idx == -1)
			break;
		else if (idx == -2)
		{
			i++;
			continue;
		}
		else
		{
			char msg[128]; int id;
			fseek(fp, 300000 + (idx * 132), SEEK_SET);
			fread(msg, sizeof(msg), 1, fp);
			fread(&id, sizeof(id), 1, fp);
			char buff[2];
			buff[0] = (id + 1) - '0';
			buff[1] = '\0';
			strcat(replybuff, buff);
			strcat(replybuff, msg);
			i++;
		}
	}
	
	strcat(replybuff, "55.Add a message\n\n65.65.Reply a message\n75.Delete a message\n85.Show replies of a message\n95.Exit\n");
	return replybuff;
}

char* showReply(int uid, int cid, int mid, FILE* fp)
{
	char *replybuff = (char*)malloc(sizeof(char)*1024);
	replybuff[0] = 'r';
	replybuff[1] = '\0';
	int i = 0, idx;
	while (1)
	{
		fseek(fp, 26000 + (uid - 1) * 200 + (cid - 1) * 40 + (mid - 1) * 20 + i * 4, SEEK_SET);
		fread(&idx, sizeof(idx), 1, fp);
		if (idx == -1)
			break;
		else if (idx == -2)
		{
			i++;
			continue;
		}
		else
		{
			char msg[128]; int id;
			fseek(fp, 300000 + (idx * 132), SEEK_SET);
			fread(msg, sizeof(msg), 1, fp);
			fread(&id, sizeof(id), 1, fp);
			//printf("%d: %s\n", i + 1, msg);
			char buff[2];
			buff[0] = (id + 1) - '0';
			buff[1] = '\0';
			strcat(replybuff, buff);
			strcat(replybuff, msg);
			i++;
		}
	}
	//printf("55.Add a reply\n");
	//printf("65.Delete a reply\n");
	//printf("75.Exit\n");
	strcat(replybuff, "55.Add a reply\n65.Delete a reply\n75.Exit\n");
	return replybuff;
}

void deleteReply(int uid, int cid, int mid, int rid, FILE* fp)
{
	fseek(fp, (26000 + (uid - 1) * 200 + (cid - 1) * 40 + (mid - 1) * 20 + (rid - 1) * 4), SEEK_SET);
	int i = -2;
	fwrite(&i, sizeof(i), 1, fp);
}

void deleteMessage(int uid, int cid, int idx, FILE* fp)
{
	fseek(fp, (10000 + ((uid - 1) * 600) + ((cid - 1) * 120) + (idx - 1) * 4), SEEK_SET);
	int i = -2;
	fwrite(&i, sizeof(i), 1, fp);
}

void SocketHandler(void* lp){
	int* csock;
	struct sockaddr_in sadr;
	int	addr_size = sizeof(SOCKADDR);
	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;
	while (true){

		printf("waiting for a connection\n");
		csock = (int*)malloc(sizeof(int));

		if ((*csock = accept(hsock, (SOCKADDR*)&sadr, &addr_size)) != INVALID_SOCKET){
			//printf("Received connection from %s",inet_ntoa(sadr.sin_addr));
			{
				memset(recvbuf, 0, recvbuf_len);
				if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
					fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
					free(csock);
				}

				//printf("Received bytes %d\nReceived string \"%s\"\n", recv_byte_cnt, recvbuf);
				process_input(recvbuf, recv_byte_cnt, csock);
			}
		}
		else{
			fprintf(stderr, "Error accepting %d\n", WSAGetLastError());
		}
	}
}