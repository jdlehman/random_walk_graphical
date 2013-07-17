//Jonathan Lehman
//April 19, 2011

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <signal.h>
# include "child.h"

//declare functions
int randDegree();
void process(char*);
void nearEdge(FILE*, double, double, int);
void drawLine(FILE*, double, double, double, double);

//global vars
struct timespec t, t2;
int avgLifeOn = 1;
int soundOn = 1;
double timeRatio = 1.0;
double pixelCordX;
double pixelCordY;
double pixelPerInch;

void pipehandler(){
	exit(0);
}

//main where program runs
int main(int argc, char **argv)
{
	signal(SIGPIPE, pipehandler);
	char *invalChar1;
	char *invalChar2;

	//check that there are 3 arguments
	if(argc != 3)
	{
		fprintf(stderr, "There is/are %d argument(s) (not including the file location). There should be 2 integer arguments, board size in inches and number of beetles.\n", argc - 1);
		exit(1);
	}

	long arg1;
	long arg2;
	//perform conversion
	//and check that there is no overflow
	if(((arg1 = strtol(argv[1], &invalChar1, 10)) >= INT_MAX) || ((arg2 = strtol(argv[2], &invalChar2, 10)) >= INT_MAX))
	{
		fprintf(stderr, "Overflow. The arguments: board size, %s, and the number of beetles, %s, must be valid integers less than %d\n", argv[1], argv[2], INT_MAX);
		exit(1);
	}

	//check that arguments are valid integers > 0 also checks underflow error and that there are no characters in the arguments
	if(!(arg1 > 0 && arg2 > 0) || (*invalChar1 || *invalChar2))
	{
		fprintf(stderr, "Both arguments must be valid integers greater than 0. The arguments: board size, %s, and the number of beetles, %s, are not both valid integers greater than 0.\n", argv[1], argv[2]);
		exit(1);
	}

	int boardSize = strtol(argv[1], NULL, 10);
	int numBeetles = strtol(argv[2], NULL, 10);
	double startPoint = (double) boardSize / 2;
	double locX;
	double locY;
	double prevLocX;
	double prevLocY;
	int degree;
	int dead;

	double elapsedSec = 0.0;
	double totSec = 0.0;

	//called to seed random by using time in seconds
	//makes rand() generate truly random numbers
	//kearns does not want to seed
	//srand(time(NULL));

	t.tv_sec = 1;
	t.tv_nsec = 0;
	pixelPerInch = (double) 300.0 / boardSize;


	FILE *read_from, *write_to;
	int childpid;
	char result[6];

	childpid = start_child("wish", &read_from, &write_to);
	fprintf(write_to, "source beetle.tcl\n");//initialize

	//get file descriptor numbers
	int readNum;
	int writeNum;

	if((readNum = fileno(read_from)) < 0)
	{
		fprintf(stderr, "/nError getting file number of read_from\n");
		exit(1);
	}

	if((writeNum = fileno(write_to)) < 0)
	{
		fprintf(stderr, "/nError getting file number of write_to\n");
		exit(1);
	}

	int hits;
	fd_set mask;
	struct timeval timeout;

	//set up for select statement
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	fprintf(write_to, ".c create text 245 4 -anchor n -text \"Board Size: %d\"\n", boardSize);

	//iterate until all beetles die
	int i;
	for(i = 0; i < numBeetles; i++)
	{

		//reset variables
		locX = startPoint;
		locY = startPoint;
		pixelCordX = 250;
		pixelCordY = 250;
		elapsedSec = 0;
		dead = 0;

		//place beetle at center to start, tagged as theBeetle
		fprintf(write_to, ".c create oval 255 255 245 245 -fill blue -tags theBeetle\n");
		fprintf(write_to, ".c itemconfigure theSquare -fill white\n");

		while(dead != 1)
		{

			//set prev X and Y coordinates
			prevLocX = locX;
			prevLocY = locY;

			FD_ZERO(&mask);//these MUST be inside of loop or it will fail
			FD_SET(readNum, &mask);
			//check if tcl is sending back any data
			if((hits = select(readNum+1, &mask, (fd_set *)0, (fd_set *)0, &timeout)) < 0)
			{
				  perror("beetle:select");
				  exit(1);
			}


			if (hits > 0 && ((FD_ISSET(readNum, &mask))))
			{
				fgets(result, 5, read_from);
				//fprintf(stdout, "read b%sb\n", result);
				process(result);
			}


			//write the current beetle number
			fprintf(write_to, ".c itemconfigure beetleNumText -text \"Beetle Num: %d\"\n", i + 1);

			//get random degree
			degree = randDegree();

			locX += cos(degree / 180.0 * M_PI);
			locY += sin(degree / 180.0 * M_PI);

			elapsedSec += 1.0;// * timeRatio;

			if(nanosleep(&t, &t2) < 0)
			{
			  fprintf(stderr, "\nbeetle: nanosleep failed.\n");
			}

			//write average life text or current beetle lifetime
			if(avgLifeOn)
			{
				fprintf(write_to, ".c itemconfigure avgLifeText -text \"Average Life: %.1f\"\n", (totSec + elapsedSec) / (i + 1));
			}
			else
			{
				fprintf(write_to, ".c itemconfigure avgLifeText -text \"Current Beetle Lifetime: %.1f\"\n", elapsedSec);
			}

			//check if beetle is within an inch of the edge
			nearEdge(write_to, locX, locY, boardSize);
			//draw line
			drawLine(write_to, locX, locY, prevLocX, prevLocY);

			//check if bug died
			if(locX < 0 || locX > boardSize || locY < 0 || locY > boardSize)
			{

				if(soundOn)
				{
					fprintf(write_to, ".c itemconfigure theSquare -fill red4\n");
					fprintf(write_to, "bell\n");
				}
				sleep(1);//as specified by kearns before board cleared
				if(i != numBeetles - 1)
				{
					//delete beetle and lines from canvas
					fprintf(write_to, ".c delete lines\n");
					fprintf(write_to, ".c delete theBeetle\n");
				}
				//set bug to dead and add elapsed seconds to total seconds
				dead = 1;
				totSec += elapsedSec;
			}
			else
			{


				//bug passes out for 1*ratio second
				elapsedSec += 1.0; //* timeRatio;
				if(nanosleep(&t, &t2) < 0)
				{
				  fprintf(stderr, "\nbeetle: nanosleep failed.\n");
				}

				//write average life text or current beetle lifetime
				if(avgLifeOn)
				{
					fprintf(write_to, ".c itemconfigure avgLifeText -text \"Average Life: %.1f\"\n", (totSec + elapsedSec) / (i + 1));
				}
				else
				{
					fprintf(write_to, ".c itemconfigure avgLifeText -text \"Current Beetle Lifetime: %.1f\"\n", elapsedSec);
				}
			}
		}

	}

	//print results
	double avgSec = (double) totSec / (double)numBeetles;
	//fprintf(stdout, "%d by %d square, %d beetle(s), mean beetle lifetime is %.1f\n", boardSize, boardSize, numBeetles, avgSec);

	//finish with wish, leaves window up but disables buttons except exit
	fprintf(write_to, "done\n");
	return 0;
}

//generates a random degree [0,360]
int randDegree()
{
	int n;
	n = random() % 361;
	return n;
}

//process data received from tcl script
void process(char *result)
{
	char val = result[0];
	result++;
	strcat(result, "\0");
	double restVal;
	//perform conversion
	restVal = strtod(result, NULL);

	switch(val)
	{
	case 'v':
		timeRatio = restVal;
		//fprintf(stdout, "time ratio: %f\n", timeRatio);
		int seconds = floor(1.0 * timeRatio);
		int milliseconds = ((double)timeRatio - (double)seconds) * 1000;
		//fprintf(stdout, "seconds %d\n", seconds);
		//fprintf(stdout, "milliseconds %d\n", milliseconds);
		t.tv_sec = seconds;
		t.tv_nsec = milliseconds * 1000000;
		break;
	case 'a':
		avgLifeOn = restVal;
		break;
	case 's':
		soundOn = restVal;
		break;
	}
}

//check if beetle is within an inch of edge, turn red, beep if sound on
void nearEdge(FILE* write_to, double x, double y, int squareSize)
{
	//fprintf(stdout, "y %f x %f size %d\n", y, x, squareSize);
	if(x <= 1 || y <= 1 || y >= (squareSize - 1) || x >= (squareSize - 1))
	{
		fprintf(write_to, ".c itemconfigure theBeetle -fill red\n");
	}
	else
	{
		fprintf(write_to, ".c itemconfigure theBeetle -fill blue\n");
	}
}

void drawLine(FILE* write_to, double locX, double locY, double prevLocX, double prevLocY)
{
	double deltaX = locX - prevLocX;
	double deltaY = -1 * (locY - prevLocY); //Y inverted to account for inversion of coordinates in TK coordinate system

	//draw line
	fprintf(write_to, ".c create line %f %f %f %f -tags lines\n", pixelCordX, pixelCordY, pixelCordX + (deltaX * pixelPerInch), pixelCordY + (deltaY * pixelPerInch));
	pixelCordX += (deltaX * pixelPerInch);
	pixelCordY += (deltaY * pixelPerInch);

	//move beetle
	fprintf(write_to, ".c coords theBeetle %f %f %f %f \n", pixelCordX - 5, pixelCordY - 5, pixelCordX + 5, pixelCordY + 5);


}

