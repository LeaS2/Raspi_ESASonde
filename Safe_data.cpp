#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/timeb.h>
#include <time.h>
#include <iostream>
#include "net_com.h"
#include <curses.h>


#define PORT     7	// Ethernet port 
#define MAXLINE 1024
#define BUFLEN 512	// Max length of buffer


using namespace std;
		

// Data package send in one frame
struct sensor_data
{
	uint counter;					// !! Muss noch auf Client Seite hinzugefügt werden!! Zählt Anzahl verschickter Datenpakete
    uint8_t id;
	uint32_t timestamp;
	float sensor1;
	float sensor2;
	float sensor3;
	float sensor4;
	float sensor5;
	float sensor1_raw;
	float sensor2_raw;
	float sensor3_raw;
	float sensor4_raw;
	float sensor5_raw;
	int32_t sensor6;
	int32_t sensor7;
	float temp1;
	float temp2;
	float temp3;
	float temp4;
	float temp5;
	int32_t temp6;
	int32_t temp7;
	float accel_x;
	float accel_y;
	float accel_z;
	float gyro_x;
	float gyro_y;
	float gyro_z;
};

// Sleep Funktion für Linux System - angegeben in Nanosek.
/* void mysleep_ms(int milisec)				 
{
    struct timespec res;
    res.tv_sec = milisec/1000;
    res.tv_nsec = (milisec*1000000) % 1000000000;
    clock_nanosleep(CLOCK_MONOTONIC, 0, &res, NULL);
}*/ 


// Read Sensordata and writes data in CSV file
void readValues(Net_com* net, int counter)
{
	struct sensor_data rx_data;						
	int temp_timestamp;                     // Hilfsvariable zur Berechnung der Latenz zw. zwei Datenpaketen
	char buffer[50];                        // buffer to store file name
	
	initscr(); 								// Hilfsfunktionen um Programm zu beenden 
	cbreak();								
	nodelay(stdscr, TRUE);								
	
	while(getch() <= 0)						//  hier: while status des Button sich nicht verändert  
	{
		FILE *file_temp;                                  // create file pointer
   		sprintf(buffer, "%d_Messung.csv", counter);       // create file name with counter included
   
		// checks if file name is already used
		if(access(buffer, F_OK) == 0)                      
		{
			printf("File mit diesem Namen existiert bereits.");
			exit(1);
		}

		file_temp = (fopen(buffer, "w+"));

		// checks if file was created successfully
		if(file_temp == NULL)                              
		{
			printf("File konnte nicht erstellt werden.");
			exit(1);
		}

		// Column header for CSV file
		fprintf(file_temp,"Timestamp, ID, Latency, Pressure 1, Pressure 2, Pressure 3\n");    
 
		int r = net->net_com_receive(&rx_data, sizeof(struct sensor_data));

		if(r > 0)			// if server receives data r > 0
		{
			temp_timestamp = rx_data.timestamp - temp_timestamp;			// caluclates latency = difference between data packages 
			fprintf(file_temp,"\n %d, %d, %d, %.2f, %.2f, %.2f \n", rx_data.timestamp, rx_data.id, temp_timestamp, rx_data.sensor1, rx_data.sensor2, rx_data.sensor3);
			
			// print in console 
			printf("\n %d, %d, %d, %.2f, %.2f, %.2f \n", rx_data.timestamp, rx_data.id, temp_timestamp, rx_data.sensor1, rx_data.sensor2, rx_data.sensor3);
			temp_timestamp = rx_data.timestamp;								// reset temp_timestamp to timestemp of recent data package
			
			// Programm kurz pausieren
			fflush(stdout);				// flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
      		usleep(20);					// in Millisekunden 
		}

		fclose(file_temp);
	}
}





int main(void)
{
	struct sockaddr_in si_me, si_other, server;			// Wird hier Socket initialisiert?
	clock_t start, stop;								// wofür? 
	int s, s2, i, slen = sizeof(si_other) , recv_len;	// Wofür?

	int counter = 1;									// Anzahl der Messungen  	

	char* ip = "192.168.0.2";
	int port = 69;
	char* source = "Sensorboard.bin";				// Wofür?
	char* destination = "Sensorboard.bin";
	Net_com net(7,"192.168.0.5","192.168.0.3"); 	// Port, Server address, Cient address - net = Datenübertragung 
	

	char input;
	net.net_com_connect();

	while(true)
	{
		 
		cout << "Choose:" << endl;
		cout << "1: read values" << endl;

		input = getchar();
		switch(input)
		{
			case '1':
				readValues(&net, counter);
				counter++;
				break;

			default:
				cout << "error!" << endl;
				break;
		}
	}
	return 0;
}
