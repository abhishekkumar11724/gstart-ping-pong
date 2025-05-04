/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/
// #include <bits/stdc++.h>
#include <iostream>
#include "raylib.h"
#include <string>
#include "network.h"

#include "resource_dir.h" // utility header for SearchAndSetResourceDir
using namespace std;

// char * player1Score = "0";
// char * player2Score = "0";

int player1Score = 0;
int player2Score = 0;

class Ball
{
public:
	int pos_x, pos_y, radius, speed_x, speed_y;
	void draw()
	{
		DrawCircle(pos_x, pos_y, radius, WHITE);
	}

	void update()
	{
		pos_x += speed_x, pos_y += speed_y;

		if (pos_y + radius >= GetScreenHeight() || pos_y - radius <= 0)
		{
			speed_y *= -1;
		}

		if (pos_x + radius >= GetScreenWidth())
		{
			pos_x = GetScreenWidth() / 2, pos_y = GetScreenHeight() / 2;
			player1Score++;
		}
		else if (pos_x - radius <= 0)
		{
			// speed_x *= -1;
			pos_x = GetScreenWidth() / 2, pos_y = GetScreenHeight() / 2;
			player2Score++;
		}
	}
};

class Paddle
{
public:
	int pos_x, pos_y, speed_y, width, height;

	void draw()
	{
		DrawRectangle(pos_x, pos_y, width, height, WHITE);
	}
	void update(char ch)
	{
		if (ch == 'l' && IsKeyDown(87) && pos_y >= 0)
		{
			pos_y -= speed_y;
		}

		if (ch == 'l' && IsKeyDown(83) && pos_y + height <= GetScreenHeight())
		{
			pos_y += speed_y;
		}

		if (ch == 'r' && IsKeyDown(73) && pos_y >= 0)
		{
			pos_y -= speed_y;
		}

		if (ch == 'r' && IsKeyDown(75) && pos_y + height <= GetScreenHeight())
		{
			pos_y += speed_y;
		}
	}
};

class CpuPaddle : public Paddle
{
public:
	void update(int ball_y)
	{
		if (pos_y + height / 2 > ball_y)
		{
			pos_y -= speed_y;
		}
		if (pos_y + height / 2 <= ball_y)
		{
			pos_y += speed_y;
		}
	}
};

int main()
{
	cout << "game is starting" << endl;
	const int screen_width = 1200, screen_height = 800;
	InitWindow(screen_width, screen_height, "Gstar");
	SetTargetFPS(60);

	Network net;
	bool amHost = 1;/* ask user or detect via CLI flag */;
	if (amHost)
	{
		net.initHost(3333);
	}
	else
	{
		net.initClient("192.168.1.42", 3333);
		// send an initial empty packet so host learns our addr
		net.sendPacket({0 /*input*/, 0, 0, 0, 0, 0, 0});
	}

	Ball ball;
	ball.radius = 20, ball.pos_x = screen_width / 2, ball.pos_y = screen_height / 2, ball.speed_x = 10, ball.speed_y = 5;

	Paddle paddleLeft;
	paddleLeft.pos_x = 10, paddleLeft.pos_y = screen_height / 2 - 50, paddleLeft.speed_y = 10, paddleLeft.width = 20, paddleLeft.height = 100;

	CpuPaddle paddleRight;
	paddleRight.pos_x = screen_width - 30, paddleRight.pos_y = screen_height / 2 - 50, paddleRight.speed_y = 10, paddleRight.width = 20, paddleRight.height = 100;
	while (!WindowShouldClose())
	{
		// 1) Gather local input
		int8_t myInput = 0;
		if (IsKeyDown(KEY_W)) myInput = -1;
		else if (IsKeyDown(KEY_S)) myInput = +1;

		// declare clientInput here so it's visible below
		int8_t clientInput = 0;
		Packet pkt{};

		if (net.isHost)
		{
			// receive client input (non-blocking)
			if (net.recvPacket(pkt) && pkt.type == 0)
			{
				clientInput = pkt.input;   // just assign, don't redeclare
			}
		}
		else
		{
			// send our input
			pkt = {0, myInput, 0, 0, 0, 0, 0};
			net.sendPacket(pkt);
		}

		// 3) Always update local paddles+ball
		//    Host uses clientInput & myInput.
		//    Client uses only myInput for its paddle; ball is teleport-updated below.
		if (net.isHost)
		{
			// host updates both paddles & ball & scores
			ball.update();
			paddleLeft.update(myInput);
			paddleRight.update(clientInput);
		}
		else
		{
			// client moves only its paddle locally for responsiveness
			paddleLeft.update(myInput);
		}

		// 4) State-sync
		if (net.isHost)
		{
			// send authoritative state back to client
			Packet statePkt;
			statePkt.type = 1;
			statePkt.ballX = ball.pos_x;
			statePkt.ballY = ball.pos_y;
			statePkt.paddleY = amHost ? paddleRight.pos_y : paddleLeft.pos_y;
			statePkt.score1 = player1Score;
			statePkt.score2 = player2Score;
			net.sendPacket(statePkt);
		}
		else
		{
			// client tries to read the latest state
			if (net.recvPacket(pkt) && pkt.type == 1)
			{
				ball.pos_x = pkt.ballX;
				ball.pos_y = pkt.ballY;
				// update remote paddle & scores
				paddleRight.pos_y = pkt.paddleY;
				player1Score = pkt.score1;
				player2Score = pkt.score2;
			}
		}

		ClearBackground(BLACK);
		BeginDrawing();

		DrawText((to_string(player1Score)).c_str(), GetScreenWidth() / 4, 0, 20, WHITE);
		DrawText((to_string(player2Score)).c_str(), (GetScreenWidth() / 4) * 3, 0, 20, WHITE);

		ball.draw();
		paddleLeft.draw();
		paddleRight.draw();
		DrawLine(screen_width / 2, 0, screen_width / 2, screen_height, WHITE);
		if (CheckCollisionCircleRec(Vector2{(float)ball.pos_x, (float)ball.pos_y},
									ball.radius, Rectangle{(float)paddleLeft.pos_x, (float)paddleLeft.pos_y, (float)paddleLeft.width, (float)paddleLeft.height}))
		{
			ball.speed_x *= -1;
		}
		if (CheckCollisionCircleRec(Vector2{(float)ball.pos_x, (float)ball.pos_y},
									ball.radius, Rectangle{(float)paddleRight.pos_x, (float)paddleRight.pos_y, (float)paddleRight.width, (float)paddleRight.height}))
		{
			ball.speed_x *= -1;
		}

		EndDrawing();
	}

	CloseWindow();
	return 0;
}
