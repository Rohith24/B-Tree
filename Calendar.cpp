#include "stdafx.h"
#define bitvectorsize 100000
#define service_start_offset 100000
#define service_people_bit_vector_offset 100640
#define service_people_bit_vector_size 500
#define service_people_offset 101140
#define s_i_bitvector_start 140140
#define s_i_bitvector_size 550 //90+450
#define single_indirect_offset 140690
#define direct_offset 141050
struct service_people
{
	int id;
	char name[32];
	char role[32];
	char phone_no[10];
};

struct service
{
	int ID;
	char servicename[32];
	int service_people[5];
	int single_indirect;
	int double_indirect;
};

char calbitvector[20000];
int service_peoples_count = 0;

struct datebooked
{
	int userid;
	char date[11];
};

char services[10][20] =
{
	"Doctors",
	"House Keeping",
	"Baby Sitting",
	"Electricians",
	"Plumbers",
	"Teachers",
	"Beauty Parlour",
	"Cable Operator",
	"Courier",
	"Psychiatrists"
};
//void writecal()
//{
//	system("fsutil file createnew calendar.bin 104857600");
//	FILE* fcal = fopen("calendar.bin", "r+b");
//	fseek(fcal, 0, 0);
//	for (int i = 0; i < 10; i++)
//	{
//		struct service ser;
//		ser.ID = i + 1;
//		fwrite(&ser, sizeof(ser), 1, fcal);
//	}
//	fseek(fcal, 10000000, 0);
//	for (int i = 0; i < 1000000; i++)
//		calbitvector[i] = 0;
//	fwrite(&calbitvector, sizeof(calbitvector), 1, fcal);
//	fseek(fcal, 1000, 0);
//	for (int i = 0; i < 1000000; i++)
//	{
//		struct datebooked dt;
//		fwrite(&dt, sizeof(dt), 1, fcal);
//	}
//	fclose(fcal);
//}

void view_services(char *buff){
	for (int i = 0; i < 10; i++){
		strcat(buff, services[i]);
		strcat(buff, "\n");
	}
}

void print_service_people(FILE *f,struct service ser,char *buff){
	struct service_people ser_pep;
	int j = 1, count = 0;
	char buffer[1024] = "";
	int bytecount;
	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;

	for (int i = 0; i < 5; i++){
		if (ser.service_people[i] != NULL){
			fseek(f, ser.service_people[i], 0);
			//printf("%ld", ftell(f));
			fread(&ser_pep, sizeof(struct service_people), 1, f);
			sprintf(buffer, "\n%d:\t%s\t%s", j++, ser_pep.name, ser_pep.role);
			strcat(buff, buffer);
			count++;
		}
	}
	/*if (ser.single_indirect != 0){
		int direct_offset = 0;
		struct single_indirect_block sib;
		int single_indir_blok[5];
		fseek(f, single_indirect_start_offset, 0);
		fseek(f, cat.category_id * cat.size_of_indirect, 1);
		fread(&single_indir_blok, sizeof(int)* 9, 1, f);
		for (int i = 0; i < cat.size_of_indirect; i++){
			if (single_indir_blok[i] != 0){
				direct_offset = single_indir_blok[i];
				fseek(f, direct_offset, 0);
				struct single_indirect_block sib;
				fread(&sib, sizeof(sib), 1, f);
				for (int i = 0; i < 50; i++){
					if (sib.directblocks[i] == NULL){
						fseek(f, sib.directblocks[i], 0);
						//printf("%ld", ftell(f));
						fread(&msg, sizeof(struct Message_inode), 1, f);
						sprintf(buffer, "\n%d:\nUser%d says:\t%s", j++, msg.userid, msg.msgtext);
						strcat(buff, buffer);
						count++;
					}
				}
				strcat(buff, "\n\tNext->");
				strcat(buff, "\nEnter choice(n-Next,e-exit:");
				if ((bytecount = send(*csock, buff, 1024, 0)) == SOCKET_ERROR){
					fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
					free(csock);
					return -1;
				}
				memset(recvbuf, 0, recvbuf_len);
				if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
					fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
					free(csock);
					return -1;
				}
			}
		}
				//scanf("%c", &choice);
	}
	return count;*/
}

int get_free_space_for_people(FILE *f){
	char bitvector[service_people_bit_vector_size];
	fseek(f, service_people_bit_vector_offset, 0);
	fread(&bitvector, service_people_bit_vector_size*sizeof(char), 1, f);
	int freespace;
	for (int i = 0; i < service_people_bit_vector_size; i++){
		if (bitvector[i] == 0)
		{
			bitvector[i] = 1;
			freespace = service_people_offset + (i*sizeof(struct service_people));
			break;
		}
	}
	fseek(f, service_people_bit_vector_offset, 0);
	fwrite(&bitvector, service_people_bit_vector_size*sizeof(char), 1, f);
	fflush(f);
	return freespace;
}

int get_free_space_direct_people(FILE *f){
	char bitvector[s_i_bitvector_size];
	fseek(f, s_i_bitvector_start, 0);
	fread(&bitvector, s_i_bitvector_size, 1, f);
	int freespace;
	for (int i = 0; i < s_i_bitvector_size; i++){
		if (bitvector[i] == 0){
			bitvector[i] = 1;
			freespace = direct_offset + (i*sizeof(struct single_indirect_block));
			break;
		}
	}
	fseek(f, s_i_bitvector_start, 0);
	fwrite(&bitvector, s_i_bitvector_size, 1, f);
	fflush(f);
	return freespace;
}

void add_service_people(FILE *f, struct service *ser, int *csock){
	char buff[1024];
	int bytecount;
	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;
	struct service_people ser_pep;
	ser_pep.id = ser->ID;
	sprintf(buff, "\nEnter Name,Role,Phone Number:(seperated by ',')");
	if ((bytecount = send(*csock, buff, 1024, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
		free(csock);
		return;
	}
	memset(recvbuf, 0, recvbuf_len);
	if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
		fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
		free(csock);
		return;
	}
	//printf("\nEnter Msg text:");
	//fflush(stdin);
	//scanf("%32[^,],%32[^,],%10[^\n]", ser_pep.name, ser_pep.role, ser_pep.phone_no);
	sscanf(recvbuf, "%32[^,],%32[^,],%10[^\n]", ser_pep.name, ser_pep.role, ser_pep.phone_no);
	fwrite(&ser_pep, sizeof(struct service_people), 1, f);
	fflush(f);
}

void menu_services(FILE *f, struct service ser,int offset, int *csock){
	char buff[1024];
	int bytecount;
	char recvbuf[1024];
	int recvbuf_len = 1024;
	int recv_byte_cnt;
	while (1)
	{
		int option, count = 0;
		sprintf(buff, "\n1-View Service People\n2-Add New one");
		if ((bytecount = send(*csock, buff, 1024, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		memset(recvbuf, 0, recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		option = atoi(recvbuf);//scanf("%d", &option);
		switch (option)
		{
		case 1:
			return;
		case 2:
			for (int i = 0; i < 5; i++){
				if (ser.service_people[i] == NULL){
					int space = get_free_space_for_people(f);
					ser.service_people[i] = space;
					fseek(f, offset, 0);
					fwrite(&ser, sizeof(struct service), 1, f);
					fseek(f, space, 0);
					add_service_people(f, &ser, csock);
					count++;
					break;
				}
			}
			if (count == 0){
				int dir_offset = 0;
				struct single_indirect_block sib;
				int single_indir_blok[9];
				fseek(f, single_indirect_offset, 0);
				fseek(f, ser.ID * 9, 1);
				if (ser.service_people == NULL){
					fread(&single_indir_blok, sizeof(int)* 9, 1, f);
					single_indir_blok[0] = get_free_space_direct_people(f);
					dir_offset = single_indir_blok[0];
				}
				else
				{
					fread(&single_indir_blok, sizeof(int)* 9, 1, f);
					for (int i = 0; i < 9; i++){
						if (single_indir_blok[i] == 0){
							single_indir_blok[i] = get_free_space_direct_people(f);
							dir_offset = single_indir_blok[i];
							break;
						}
					}
				}
				fseek(f, dir_offset, 0);
				//struct single_indirect_block sib;
				fread(&sib, sizeof(sib), 1, f);
				for (int i = 0; i < 5; i++){
					if (sib.directblocks[i] == NULL){
						int space = get_free_space_for_people(f);
						sib.directblocks[i] = space;
						fseek(f, dir_offset, 0);
						fwrite(&sib, sizeof(struct single_indirect_block), 1, f);
						fseek(f, space, 0);
						add_service_people(f, &ser, csock);
						count++;
						break;
					}
				}
				fseek(f, offset, 0);
				fwrite(&ser, sizeof(struct service), 1, f);
				fflush(f);
			}
			//print_service_people(f, ser, buff);
			break;
		default:
			break;
		}
	}
}

void calendarstore(int *csock, int current_user_id){
	while (1)
	{
		system("cls");
		char buff[1024];
		int bytecount;
		char recvbuf[1024];
		int recvbuf_len = 1024;
		int recv_byte_cnt;
		int err = system("fsutil file createnew calendar.bin 104857600");
		FILE* f = fopen("calendar.bin", "r+b");
		if (err == 0){
			fseek(f, service_start_offset, 0);
			for (int i = 0; i < 10; i++){
				struct service ser;
				memset(&ser, 0, sizeof(ser));
				ser.ID = i + 1;
				strcpy(ser.servicename, services[i]);
				for (int j = 0; j < 5; j++){
					ser.service_people[j] = NULL;
				}
				ser.single_indirect = 0;
				fwrite(&ser, sizeof(struct service), 1, f);
			}
		}

		sprintf(buff, "The Services are:\n");
		view_services(buff);
		strcat(buff, "\nSelect Services:");
		if ((bytecount = send(*csock, buff, 1024, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		int option;
		memset(recvbuf, 0, recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		option = atoi(recvbuf);//scanf("%d", &option);
		int offset = service_start_offset + (sizeof(struct service)*(option-1));
		fseek(f, offset, 0);
		struct service ser;
		fread(&ser, sizeof(struct service), 1, f);
		memset(&buff, 0, 1024);
		menu_services(f, ser,offset, csock);
		print_service_people(f, ser, buff);
		if ((bytecount = send(*csock, buff, 1024, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error sending data %d\n", WSAGetLastError());
			free(csock);
			return;
		}
		memset(recvbuf, 0, recvbuf_len);
		if ((recv_byte_cnt = recv(*csock, recvbuf, recvbuf_len, 0)) == SOCKET_ERROR){
			fprintf(stderr, "Error receiving data %d\n", WSAGetLastError());
			free(csock);
			return;
		}

	}
}