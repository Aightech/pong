#include <iostream>
#include <unistd.h>
#include <curses.h>
#include <string.h>
#include <time.h>
#include "netapi.hpp"

#define NB_PLAYERS 2
#define NB_DIM 2
#define WIDTH 50
#define HEIGHT 16
#define L 5

typedef struct _Pong
{
	int size[NB_DIM];
	int pos[NB_PLAYERS][NB_DIM];
	char map[HEIGHT][WIDTH];
	int ball[NB_DIM];
	int score[NB_PLAYERS];
	int c;
	float dir[2];
	float ballf[2];
	float angleCoef[2];
	clock_t time;
	int speed;
	bool count[NB_PLAYERS];
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
int init(Pong &p,int first_dir)
{
	p.size[0] = HEIGHT;
	p.size[1] = WIDTH;
	for(int player = 0; player < NB_PLAYERS; player++)
		{
			p.pos[player][0] = HEIGHT/2-L/2;
			p.pos[player][1] =  (WIDTH-1)*player;
			//std::cout << p.pos[player][0] << std::endl;
			p.score[player]=0;
		}
	clear(p);

	for(int i=0; i< NB_DIM;i++)
		{
			p.ball[i] = p.size[i]/2;
			p.ballf[i] = p.ball[i]; 
			p.dir[i]=-0.6;
			p.angleCoef[i]=i;
		}
	int dir = first_dir/abs(first_dir);
	p.angleCoef[0]=0.5;
	p.angleCoef[1]=0.5*dir;
	p.time = clock();
	p.speed = 5000;
	
	
}


//redraw the map and the bordure in the terminal
int draw(Pong &p)
{
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
	
	initscr();
	cbreak();
	noecho();
	timeout(50);
	endwin();
	
	p.c = getch();
	//if the player pressed a key to move
	if(p.c == 'z' && p.pos[0][0] > 0)
		p.pos[0][0]--;
	if(p.c == 's' && p.pos[0][0] < HEIGHT - L)
		p.pos[0][0]++;


	if(p.c == 'p')
		init(p,1);

	
	std::cout << std::endl;
	std::cout << '+';
	for(int j = 0 ; j < WIDTH+2 ; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;
	std::cout << '+';
	for(int j = 0 ; j < WIDTH+2 ; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;

	

	std::cout << "+ YOU: " << p.score[0];
	
	for(int j = 0 ; j < (WIDTH-16)/2 ; j++)
		std::cout << ' ';	
	std::cout << "PONG";
	for(int j = 0 ; j < (WIDTH-16)/2 ; j++)
		std::cout << ' ';
	std::cout << "ADD: " << p.score[1] << " +"<<std::endl;


	
	std::cout << '+';
	for(int j = 0 ; j < WIDTH +2; j++)
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
	for(int j = 0 ; j < WIDTH +2; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;

	std::cout << "+" ;
	
	for(int j = 0 ; j < (WIDTH-19)/2 ; j++)
		std::cout << '-';	
	std::cout << " press \'p\' to restart ";
	for(int j = 0 ; j < (WIDTH-19)/2 ; j++)
		std::cout << '-';
	std::cout << "+"<<std::endl;

	std::cout << '+';
	for(int j = 0 ; j < WIDTH +2; j++)
		std::cout << '-';
	std::cout << '+'<<std::endl;
	std::cout << std::endl;

	
	
}

int update(Pong &p)
{
	double dt = p.speed*(double)(clock() - p.time) / (double)CLOCKS_PER_SEC;
	p.time = clock();
	//std::cout << dt<< std::endl;
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
					if((p.ball[0] < p.pos[pl][0] || p.ball[0] > p.pos[pl][0] + L )&& !p.count[pl])
						{
							p.score[(pl+1)%2]++;
							p.count[pl]=true;
						}
					 
					
					else//if the player hit it then change the trajectory
						{
							p.count[pl]=false;
							p.ballf[1] += -(2*pl-1);
							p.ball[1] += -(2*pl-1);
							p.dir[1] = -p.dir[1];
							float coef =  ((p.ball[0] - p.pos[pl][0])/(float)(L-1)-0.5)*2;
							p.angleCoef[0] += 0.1*coef;
							p.angleCoef[0] = (p.angleCoef[0] < 0)?0:(p.angleCoef[0]>1)?1:p.angleCoef[0];
							p.angleCoef[1] = 1 - p.angleCoef[0];
						}
			
				}
			else
				p.count[pl]=false;
		}
	
	
	

	
}


int main(int argc, char *argv[])
{
	Pong pong;
	// NetAPI api;
	// char enter[50];
	// char update_msg[50];
	// char ip_add[50];
	// int choice=0;
	// //api.verbose();
	// api.setConnectable();
	// api.setConnectionPhrase((char*)"ok");


	// std::cout << "######## ######### #######" << std::endl;
	// std::cout << "######## PONG GAME #######" << std::endl;
	// std::cout << std::endl;

	// std::cout << "LAN GAME:  Please enter the IP of the adversaire: " << std::endl;
	// std::cout << "IP address : 192.168.0.";
	
	// cin >> ip_add;
	// if(strlen(ip_add)<4)
	// 	sprintf(ip_add,"192.168.0.%d",atoi(ip_add));
        
	// std::cout << "STARTING THE RECEIVER ... " << std::endl;
	// api.startReceiver(2000,(char*)"TCP");
	// int n,tic=0;
	// int master = 0;
	// sprintf(update_msg,"M%d",master);
	// if(api.sendToAddress(2000,(char *)ip_add,(char*)update_msg,(char*)"tcp")==1)
	// 	master = 1;
	// else
	// 	{
	// 		while((n=api.getReceiverBuffer(enter))<0)
	// 			{
	// 				char str[] = "o        \0";
	// 				for(int i=0;i<10;i++) str[i]= (i==abs((tic%10 - (tic/10)%2*9)%10))?'o':' ';
	// 				std::cout << "[PONG] waiting player (" << ip_add << ") ... [" << str << "]" <<"\xd"<<std::flush;
	// 				tic++;
	// 			}
	// 	}

	// int val[4];
	// init(pong, (master)?1:-1);
	// while(1)
	// 	{
	// 		clear(pong);
	// 		while((n=api.getReceiverBuffer(enter))>-1)
	// 			for(int i = 0 ; i < 6; i++)
	// 				val[i] = atoi(strchr(enter,'a'+i)+1);	
				
			
	// 		pong.pos[1][0] = val[0];
	// 		if(!master)
	// 			{
					
	// 				pong.ball[0] = val[2];
	// 				pong.ball[1] = pong.size[1]-1 - val[3];
	// 			}
	// 		else
	// 			{
	// 				update(pong);
	// 			}
	// 		sprintf(update_msg,"Ma%db%dc%dd%d",pong.pos[0][0],pong.pos[1][0],pong.ball[0],pong.ball[1]);
	// 		api.sendToAddress(2000,(char *)ip_add,(char*)update_msg,(char*)"tcp");
	//  		draw(pong);

			
	// 	}
	init(pong,1);
	while(1)
		{
			clear(pong);
			update(pong);
			draw(pong);
		}
	

	return 0;
}
