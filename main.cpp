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
#include "sensor_config.h"
#include "diag_sensorboard.h"
//#include <wiringPi.h>

#define DATA_PORT 7 // Ethernet port
#define DIAG_PORT 8
#define MAXLINE 1024
#define BUFLEN 512 // Max length of buffer
#define AMOUNT_OFFSETS 500.0 // Anzahl Durchläufe zur Berechnung des Offset

using namespace std;

// Data package send in one frame
struct sensor_data
{
	uint32_t counter; // counter variable zur Überprüfung, ob alle Datenpakete in richtiger Reihenfolge ankommen
	uint8_t id;
	uint32_t timestamp; // Systemzeit des Microcontrollers
	float sensor1;
	float sensor2;
	float sensor3;
	float sensor4;
	float sensor5;
	int32_t sensor6;
	int32_t sensor7;
	float temp1;
	float temp2;
	float temp3;
	float temp4;
	float temp5;
	int32_t temp6;
	int32_t temp7;
};

// offset for differencial pressure sensors 
double offset_p1 = 0;
double offset_p2 = 0;
double offset_p3 = 0;
double offset_p4 = 0;
double offset_p5 = 0;


void calculateOffsets(Net_com &net)
{
	net.net_com_connect();

	struct sensor_data rx_data;

	for (int i = 0; i < AMOUNT_OFFSETS; i++)
	{
		// waiting for connection to sensorboard
		int rec_values = 0;
		do
		{
			rec_values = net.net_com_receive(&rx_data, sizeof(struct sensor_data));
			usleep(10);
		} while (rec_values == 0);


		offset_p1 = offset_p1 + rx_data.sensor1;
		offset_p2 = offset_p2 + rx_data.sensor2;
		offset_p3 = offset_p3 + rx_data.sensor3;
		offset_p4 = offset_p4 + rx_data.sensor4;
		offset_p5 = offset_p5 + rx_data.sensor5;

		// Pause programm
		printf("%.2f, %.2f, %.2f, %.2f, %.2f\n", offset_p1, offset_p2, offset_p3, offset_p4, offset_p5);

	}

	offset_p1 = offset_p1 / (double)AMOUNT_OFFSETS;
	offset_p2 = offset_p2 / (double)AMOUNT_OFFSETS;
	offset_p3 = offset_p3 / (double)AMOUNT_OFFSETS;
	offset_p4 = offset_p4 / (double)AMOUNT_OFFSETS;
	offset_p5 = offset_p5 / (double)AMOUNT_OFFSETS;
	printf("\n Offsets %.2f; %.2f; %.2f; %.2f; %.2f; \n", offset_p1, offset_p2, offset_p3, offset_p4, offset_p5);

	net.net_com_close();

	printf("Offsets ready!\n");
}

// Sensor calibration 
void KalibValues(Net_com &net, int counter)
{
	net.net_com_connect();

	struct sensor_data rx_data;
	int latency;	 // Hilfsvariable zur Berechnung der Latenz zw. zwei Datenpaketen
	char buffer[50]; // buffer to store file name
	int druck = 0;
	int SPS = 0; 

	// Date & Time
	time_t timer;
	char buffer_date[26];
	char buffer_time[26];
	struct tm *tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer_date, 26, "Date: %d.%m.%Y", tm_info);	
	strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

	printf("Bisheriger Druck: %i\n", druck);
	printf("Neuen Druck eingeben: ");
	scanf("%d", &druck);
	printf("Gesetzter Druck: %i\n", druck);

	printf("SPS eingeben: ");
	scanf("%d", &SPS);
	printf("Gesetzte SPS: %i\n", SPS);

	FILE *file;										 		
	sprintf(buffer, "%d_%d_Kalibrierung.csv", counter, SPS); // create file name with counter/SPS included

	// checks if file name is already used
	if (access(buffer, F_OK) == 0)
	{
		printf("File mit diesem Namen existiert bereits.\n");
		exit(1);
	}

	file = (fopen(buffer, "w+"));

	// checks if file was created successfully
	if (file == NULL)
	{
		printf("File konnte nicht erstellt werden.\n");
		exit(1);
	}
	else
	{
		// File header 
		fprintf(file, "%s; %s \n", buffer_date, buffer_time);
		fprintf(file, "Druck %i\n", druck);
		fprintf(file, "Offset 1: %.2f; Offset 2: %.2f; Offset 3: %.2f; Offset 4: %.2f; Offset 5: %.2f\n", offset_p1, offset_p2, offset_p3, offset_p4, offset_p5);
		fprintf(file, "Counter; Timestamp; ID; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		printf("Datei wurde erfolgreich erstellt. \n");
	}

	printf("Übertragung gestartet.");

	for (int i = 0; i < 150; i++)
	{
		int rec_values = 0;
		do
		{
			rec_values = net.net_com_receive(&rx_data, sizeof(struct sensor_data));
			usleep(10);
		} while (rec_values == 0);

		// write data in file
		latency = rx_data.timestamp - latency; // caluclates latency = difference between data packages

		rx_data.sensor1 = rx_data.sensor1 - offset_p1;
		rx_data.sensor2 = rx_data.sensor2 - offset_p2;
		rx_data.sensor3 = rx_data.sensor3 - offset_p3;
		rx_data.sensor4 = rx_data.sensor4 - offset_p4;
		rx_data.sensor5 = rx_data.sensor5 - offset_p5;

		fprintf(file, "\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, latency, rx_data.sensor1, rx_data.sensor2,
				rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		// print in console
		printf("\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, latency, rx_data.sensor1, rx_data.sensor2,
			   rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);
		latency = rx_data.timestamp; // reset temp_timestamp to timestemp of recent data package

		fflush(stdout); // flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
	}

	fclose(file);
	net.net_com_close();

}

// Read sensordata and writes data in CSV file
void readValues(Net_com &net, int counter, float temp_A, float temp_S)
{
	struct sensor_data rx_data;
	int latency;	  // Hilfsvariable zur Berechnung der Latenz zw. zwei Datenpaketen
	char buffer1[50]; // buffer to store file name
	char buffer2[50]; // buffer to store file name

	// Date & Time
	time_t timer;
	char buffer_date[26];
	char buffer_time[26];
	struct tm *tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer_date, 26, "Date: %d.%m.%Y", tm_info);
	strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

	FILE *file_temp;								  // create file pointer
	FILE *file_kalib;								  // create file pointer
	sprintf(buffer1, "%d_Messung.csv", counter);	  // create file name with counter included
	sprintf(buffer2, "%d_MessungKalib.csv", counter); // create file name with counter included

	// checks if file name is already used
	if (access(buffer1, F_OK) == 0)
	{
		printf("File mit diesem Namen existiert bereits.\n");
		exit(1);
	}

	file_temp = (fopen(buffer1, "w+"));
	file_kalib = (fopen(buffer2, "w+"));

	// checks if file was created successfully
	if (file_temp == NULL || file_kalib == NULL)
	{
		printf("File konnte nicht erstellt werden.\n");
		exit(1);
	}
	else
	{
		// Header for CSV file: Anstell- & Schiebewinkel; column header //
		fprintf(file_temp, "%s; %s \n", buffer_date, buffer_time);
		fprintf(file_kalib, "%s; %s \n", buffer_date, buffer_time);
		fprintf(file_temp, "Anstellwinkel: %f - Schiebewinkel %f\n", temp_A, temp_S);
		fprintf(file_kalib, "Anstellwinkel: %f - Schiebewinkel %f\n", temp_A, temp_S);
		fprintf(file_temp, "Counter; Timestamp; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		fprintf(file_kalib, "Counter; Timestamp; Latency; Pressure 1; Pressure 2; Pressure 3; Pressure 4; Pressure 5; Pressure 6; Pressure 7; Temperature 1; Temperature 2; Temperature 3; Temperature 4; Temperature 5; Temperature 6; Temperature 7\n");
		printf("Datei wurde erfolgreich erstellt. \n");
	}

	printf("Übertragung gestartet.\n");

	for (int i = 0; i < 500; i++)
	{
		int rec_values = 0;
		do
		{
			rec_values = net.net_com_receive(&rx_data, sizeof(struct sensor_data));
			usleep(10);
		} while (rec_values == 0);

		// write data in file
		latency = rx_data.timestamp - latency; // caluclates latency = difference between data packages
		fprintf(file_temp, "\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, latency, rx_data.sensor1, rx_data.sensor2,
				rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		rx_data.sensor1 = rx_data.sensor1 - offset_p1;
		rx_data.sensor2 = rx_data.sensor2 - offset_p2;
		rx_data.sensor3 = rx_data.sensor3 - offset_p3;
		rx_data.sensor4 = rx_data.sensor4 - offset_p4;
		rx_data.sensor5 = rx_data.sensor5 - offset_p5;

		fprintf(file_kalib, "\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, latency, rx_data.sensor1, rx_data.sensor2,
				rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);

		// print in console
		printf("\n %i; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i; %.2f; %.2f; %.2f; %.2f; %.2f; %i; %i \n", rx_data.counter, rx_data.timestamp, latency, rx_data.sensor1, rx_data.sensor2,
			   rx_data.sensor3, rx_data.sensor4, rx_data.sensor5, rx_data.sensor6, rx_data.sensor7, rx_data.temp1, rx_data.temp2, rx_data.temp3, rx_data.temp4, rx_data.temp5, rx_data.temp6, rx_data.temp7);
		latency = rx_data.timestamp; // reset temp_timestamp to timestemp of recent data package

		// Pause programm
		fflush(stdout); // flushed Outputstream bevor System schläft - notwendig vor allem wenn Daten auf Konsole ausgegeben werden
	}

	fclose(file_temp);
	fclose(file_kalib);
	net.net_com_close();

	printf("Daten wurden erforlgreich gespeichert.\n");
}


/*
 * send diag data to sensorboard
 */
bool sendDiagData(Net_com &diag, diag_data* send_data)
{
	struct diag_data recv_data;

	diag.net_com_connect();
	uint8_t send_counter = 0;
	do
	{
	    diag.net_com_sendto(send_data, sizeof(struct sensor_config));
        diag.net_com_receive(&recv_data, sizeof(struct sensor_config));
        if(send_counter > 10u)
        {
        	diag.net_com_close();
        	return false;
        }
        send_counter++;
	} while((int)recv_data.id != (SET_SENSOR_CONFIG  + CONTROL_WORD));

	diag.net_com_close();

	return true;
}

/*
 *
 */
void config_sensor(Net_com &diag)
{
	struct sensor_config sensor_config;
	struct diag_data send_data;

	sensor_config.datarate = N_DR_90_SPS;
	sensor_config.mode = NORMAL_MODE;
	sensor_config.delay = 12; //12 ms

	send_data.id = SET_SENSOR_CONFIG;
	std::memcpy(send_data.data, &sensor_config, sizeof(struct sensor_config));
	if(sendDiagData(diag, &send_data))
	{
		cout << "config erfolgreich geschrieben" << endl;
	}
	else
	{
		cout << "config error!" << endl;
	}
}


/*
 * main
 */
int main( int argc, char* argv[] )
{
	Net_com net(DATA_PORT, "192.168.0.5", "192.168.0.3"); // Port, Server address, Cient address - net = Datenübertragung
	Net_com net_diag(DIAG_PORT, "192.168.0.5", "192.168.0.3"); // Port, Server address, Cient address - net = Datenübertragung

	float Anstellwinkel = 0; // von -18° bis 18°
	float Schiebewinkel = 0;
	char Schieb;
	int counter = 0; // Anzahl der Messungen
	int counter_kalib = 0;

    if(strcmp(argv[0], "run") == 0)
    {
    	config_sensor(net_diag);
    }

	while (true)
	{
		// Menü
		cout << "Wähle:" << endl;
		cout << "1: Offsets berechnen." << endl;
		cout << "2: Neuen Anstellwinkel eingeben und Messung starten." << endl;
		cout << "3: Neuen Druck eingeben und Kalibrierung starten." << endl;
		cout << "4: Programm beenden." << endl;

		// Terminates programm if 2 is choosen
		char input;
		scanf(" %c", &input);
		if (input != '1' && input != '2' && input != '3' && input != '4')
		{
			printf("Falsche Eingabe.\n");
			continue;
		}
		else if (input == '1')
		{
			calculateOffsets(net);
			continue;
		}
		else if (input == '3')
		{
			counter_kalib++;
			KalibValues(net, counter_kalib);
			continue;
		}
		else if (input == '4')
		{
			break;
		}

		// Sets Anstellwinkel 
		printf("Bisheriger Anstellwinkel: %.2f\n", Anstellwinkel);
		printf("Neuen Anstellwinkel eingeben: ");
		scanf("%f", &Anstellwinkel);
		printf("Gesetzter Anstellwinkel: %.2f\n", Anstellwinkel);
		fflush(stdout); 
		sleep(1);

		// reads sensor data for all Anstellwinkel for the set Schiebewinkel
		while (true) // von -6° bis 15°
		{
			printf("Nächste Messung? Ja = j | Beenden = n");
			scanf("%c", &input);
		
			if (input != 'j' && input != 'n')
			{
				printf("Falsche Eingabe.\n");
				continue;
			}
			else if (input == 'n')
			{
				break;
			}
			else
			{
				printf("Bisheriger Schiebewinkel: %.2f\n", Schiebewinkel);
				printf("Neuen Schiebewinkel eingeben: ");
				scanf("%f", &Schiebewinkel);
				counter++;
				printf("%i Messung. \n", counter);
				readValues(net, counter, Anstellwinkel, Schiebewinkel);

				fflush(stdout);
			}
		}
	}

	return 0;
}
