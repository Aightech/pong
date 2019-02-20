#include <iostream>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <time.h>
#include "netapi.hpp"

#define NB_PLAYERS 2
#define NB_DIM 2
#define WIDTH 100
#define HEIGHT 30
#define L 5

typedef struct _Pong
{
	int size[NB_DIM];
	int pos[NB_PLAYERS][NB_DIM];
	char map[HEIGHT][WIDTH];
	int ball[NB_DIM];
	int score[NB_DIM];
	int c;
	float dir[2];
	float ballf[2];
	float angleCoef[2];
	clock_t time;
	int speed;
} Pong;


//clar the whole map
int clear(Pong &p)
{
	for(int i = 0 ; i < HEIGHT ; i++)
		{
			for(int j = 0 ; j < WIDTH ; j++)
				{
					p.map[i][j]=' ';
				}
		}
       
}

//init all the parameters
int init(Pong &p)
{
	p.size[0] = HEIGHT;
	p.size[1] = WIDTH;
	for(int player = 0; player < NB_PLAYERS; player++)
		{
			p.pos[player][0] = HEIGHT/2-L/2;
			p.pos[player][1] =  (WIDTH-1)*player;
			//std::cout << p.pos[player][0] << std::endl;
		}
	clear(p);

	for(int i=0; i< NB_DIM;i++)
		{
			p.ball[i] = p.size[i]/2;
			p.ballf[i] = p.ball[i]; 
			p.dir[i]=-0.6;
			p.angleCoef[i]=i;
		}
	p.angleCoef[0]=0.5;
	p.angleCoef[1]=0.5;
	p.time = clock();
	p.speed = 5000;
	
}


//redraw the map and the bordure in the terminal
int draw(Pong &p)
{
	initscr();
	cbreak();
	noecho();
	timeout(50);
	endwin();
	
	p.c = getch();
	std::cout << std::endl;
	std::cout << '+';
	for(int j = 0 ; j < WIDTH ; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;
	for(int i = 0 ; i < HEIGHT ; i++)
		{
			std::cout << '|' << ' ';
			for(int j = 0 ; j < WIDTH ; j++)
				{
					std::cout << p.map[i][j];
				}
			std::cout << ' ' << '|' << std::endl;
		}
	std::cout << '+';
	for(int j = 0 ; j < WIDTH ; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;
	std::cout << std::endl;
	
}

int update(Pong &p)
{
	double dt = p.speed*(double)(clock() - p.time) / (double)CLOCKS_PER_SEC;
	p.time = clock();
	std::cout << dt<< std::endl;
	//make it boing against wall
	for(int i = 0 ; i< NB_DIM; i++)
		{
			float npos = p.ballf[i] + p.dir[i]*p.angleCoef[i]*dt;
			if(npos > 0 && npos < p.size[i])
				p.ballf[i] = npos;
			else
				p.dir[i] = -p.dir[i];
			p.ball[i] = p.ballf[i];
		}


	//test if the ball hit a goal 
	for(int pl = 0 ; pl < NB_PLAYERS; pl++)
		{
			if(p.ball[1]==(p.size[1]-1)*pl)
				{
					if(p.ball[0] < p.pos[pl][0] || p.ball[0] > p.pos[pl][0] + L )
						std::cout << "goal"<< std::endl;
					else//if the player hit it then change the trajectory
						{
							p.ballf[1] += -(2*pl-1);
							p.ball[1] += -(2*pl-1);
							p.dir[1] = -p.dir[1];
							float coef =  ((p.ball[0] - p.pos[pl][0])/(float)(L-1)-0.5)*2;
							p.angleCoef[0] += 0.1*coef;
							p.angleCoef[0] = (p.angleCoef[0] < 0)?0:(p.angleCoef[0]>1)?1:p.angleCoef[0];
							p.angleCoef[1] = 1 - p.angleCoef[0];
						}
			
				}
		}
	
	
	//if the player pressed a key to move
	if(p.c == 'z' && p.pos[0][0] > 0)
		p.pos[0][0]--;
	if(p.c == 's' && p.pos[0][0] < HEIGHT - L)
		p.pos[0][0]++;

	//redraw the rackets in the map
	int j;
	for(int pl = 0 ; pl < NB_PLAYERS; pl++)
		{
			
			for(int i = p.pos[pl][0] ; i < p.pos[pl][0] + L; i++)
				{
					j = p.pos[pl][1];
					//std::cout <<i << " " <<  j << std::endl;
					p.map[i][j] = '#';
				}
		}

	//draw the ball in the map
	p.map[p.ball[0]][p.ball[1]] = 'O';
}


int main(int argc, char *argv[])
{
	Pong pong;
	NetAPI api;
	char enter[50];
	int choice=0;
	api.verbose();
	api.setConnectable();
	api.setConnectionPhrase((char*)"ok");


	std::cout << "######## ######### #######" << std::endl;
	std::cout << "######## PONG GAME #######" << std::endl;
	std::cout << std::endl;

	std::cout << "LAN GAME:  Please enter the IP of the adversaire: " << std::endl;
	std::cout << "IP address : ";
	
	cin >> enter;
        
	std::cout << "STARTING THE RECEIVER ... " << std::endl;
	api.startReceiver(2000,(char*)"TCP");
	int n;
        while((n=api.getReceiverBuffer(enter))>-1)
		{
			api.sendToAddress(2000,(char *)enter,(char*)"M0",(char*)"tcp");
		}
	while((n=api.getReceiverBuffer(enter))>-1)
		{
			cout << "buffer n°"<< n<< ": "<<enter<< endl;
		}
	
	// //waitSec(2,true);
	// if(argc > 1)
	// 	{
	// 		char enter[50];
	// 		api.connectToServer(2000,(char *)argv[1]);
	// 	}
	// else
	// 	while(api.getClientAddr().size() < 1 ){}

	// api.sendToClient(0,(char*)"Mhey",(char*)"tcp");
	// api.sendToClient(0,(char*)"Mhoyy",(char*)"tcp");
	// api.sendToClient(0,(char*)"Mhoazda",(char*)"tcp");
	// api.clearSendingThread();
	// cin>>enter;

	// int n;
	// cout << "start" << endl;
	// while((n=api.getReceiverBuffer(enter))>-1)
	// {
	// 	cout << "buffer n°"<< n<< ": "<<enter<< endl;
	// }
	// init(pong);
	// while(1)
	// 	{
	// 		clear(pong);
	// 		update(pong);
	// 		draw(pong);	
	// 	}
	return 0;
}
