/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

#include "raylib.h"

#include "resource_dir.h"	// utility header for SearchAndSetResourceDir

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 800
#define MAX_TERRAIN 10

// wabbit constants
#define MAX_X_VELOCITY 5.0f
#define MAX_Y_VELOCITY 5.0f
#define GRAVITY 0.5f

typedef struct Wabbit
{
	Texture texture;
	Vector2 position;
	Rectangle bounds;
	float rotation;
	float scale;
	Color tint;
	float alpha;
	Vector2 velocity;
}Wabbit;

typedef struct StaticTerrain{
	Rectangle bounds;
	Texture2D texture;
	bool hasTexture;
	// Vector2 position;
	float rotation;
	float scale;
	Color tint;
	float alpha;
}StaticTerrain;

bool isColliding(Rectangle bounds1, Rectangle bounds2){
	// check if the two rectangles are colliding
	if (bounds1.x < bounds2.x + bounds2.width &&
		bounds1.x + bounds1.width > bounds2.x &&
		bounds1.y < bounds2.y + bounds2.height &&
		bounds1.y + bounds1.height > bounds2.y)
	{
		return true;
	}
	return false;
}

// static terrain

StaticTerrain CreateStaticTerrain(Rectangle bounds, float rotation, float scale, Color tint, float alpha)
{
	StaticTerrain terrain;
	// by default have no texture
	terrain.hasTexture = false;
	terrain.texture = (Texture2D){0};
	terrain.bounds = bounds;
	terrain.rotation = rotation;
	terrain.scale = scale;
	terrain.tint = tint;
	terrain.alpha = alpha;
	return terrain;
}

void InitStaticTerrain(StaticTerrain terrain[], int nrOfTerrain){
	for (int i = 0; i < nrOfTerrain; i++)
	{
		terrain[i].texture = LoadTexture("wabbit_alpha.png");
		// terrain[i].position = (Vector2){ 0, 0 };
		terrain[i].rotation = 0.0f;
		terrain[i].scale = 1.0f;
		terrain[i].tint = WHITE;
		terrain[i].alpha = 1.0f;
	}	
}

int DestroyStaticTerrain(StaticTerrain * terrain)
{
	// unload the texture so it can be cleaned up
	UnloadTexture(terrain->texture);
	return 0;
}

// draw the terrain with a texture if one is defined, otherwise draw a rectangle
void DrawStaticTerrain(StaticTerrain * terrain)
{
	if (terrain->hasTexture)
	{
		DrawTexturePro(
			terrain->texture, 
			(Rectangle){ 0, 0, terrain->texture.width, terrain->texture.height }, 
			terrain->bounds, 
			(Vector2){ 0, 0 }, 
			terrain->rotation, 
			terrain->tint);
	}
	else
		DrawRectangleRec(terrain->bounds, terrain->tint);
}


// WABBIT

Wabbit CreateWabbit(const char * texture_path, Vector2 position, float rotation, float scale, Color tint, float alpha, Vector2 velocity)
{
	Wabbit wabbit;
	wabbit.texture = LoadTexture(texture_path);
	wabbit.position = position;
	wabbit.rotation = rotation;
	wabbit.scale = scale;
	wabbit.tint = tint;
	wabbit.alpha = alpha;
	wabbit.velocity = velocity;

	// set the bounds of the wabbit
	wabbit.bounds.x = wabbit.position.x;
	wabbit.bounds.y = wabbit.position.y;
	wabbit.bounds.width = wabbit.texture.width * wabbit.scale;
	wabbit.bounds.height = wabbit.texture.height * wabbit.scale;

	return wabbit;
}

void DestroyWabbit(Wabbit * wabbit)
{
	// unload the texture so it can be cleaned up
	UnloadTexture(wabbit->texture);
}

void DrawWabbit(Wabbit * wabbit)
{
	DrawTextureEx(wabbit->texture, wabbit->position, wabbit->rotation, wabbit->scale, wabbit->tint);
}


void ApplyGravity(Wabbit * wabbit)
{
	int ypos = wabbit->position.y + wabbit->texture.height * wabbit->scale;
	DrawText(TextFormat("ypos: %d", ypos), WINDOW_WIDTH - 400, 300, 20, WHITE);
	if(wabbit->position.y >= (WINDOW_HEIGHT - wabbit->texture.height * wabbit->scale))
	{
		wabbit->position.y = (WINDOW_HEIGHT - wabbit->texture.height * wabbit->scale);
		wabbit->velocity.y = 0.0f;
	}
	else{
		// apply gravity to the wabbit
		wabbit->velocity.y += 0.5f;
	}
}

void HandleInput(Wabbit * wabbit)
{
	// update the wabbit's velocity based on user input
	// goes into the next update loop
	if (IsKeyDown(KEY_RIGHT))
		{
			if(wabbit->velocity.x <= 0)
				wabbit->velocity.x = 1.0f;

			wabbit->velocity.x *= 1.2f;
			
			// cap the top speed
			if(wabbit->velocity.x > MAX_X_VELOCITY)
				wabbit->velocity.x = MAX_X_VELOCITY;
			// if moving below top speed apply braking force
		}
	else if (IsKeyDown(KEY_LEFT)) 
	{
		if(wabbit->velocity.x >= 0)
			wabbit->velocity.x = -1.0f;
		wabbit->velocity.x *= 1.2f;
			
		// cap the top speed
		if (wabbit->velocity.x < -MAX_X_VELOCITY)
			wabbit->velocity.x = -MAX_X_VELOCITY;
	}
	else
	{
		// apply a braking force to the wabbit's velocity
		if (wabbit->velocity.x > 0)
		{
			wabbit->velocity.x -= 0.5f;
			if(wabbit->velocity.x < 0)
				wabbit->velocity.x = 0.0f;
		}
		else if (wabbit->velocity.x < 0)
		{
			wabbit->velocity.x += 0.5f;
			if(wabbit->velocity.x > 0)
				wabbit->velocity.x = 0.0f;
		}
	}
	static bool jumping = false;
	if (IsKeyDown(KEY_SPACE)) 
	{
		// jump
		wabbit->velocity.y -= 5.0f;
		wabbit->velocity.y = max(wabbit->velocity.y, -10.0f);
		jumping = true;
	}
	
	// adorable hopping so it stays	
	if (IsKeyDown(KEY_UP))
	{
		wabbit->position.y -= 5.0f;
	}

}

void Update(Wabbit * wabbit, StaticTerrain * terrain)
{
	// update the wabbit's position based on its velocity
	wabbit->position.x += wabbit->velocity.x;
	wabbit->position.y += wabbit->velocity.y;
	
	// apply gravity to the wabbit
	wabbit->bounds.x = wabbit->position.x;
	wabbit->bounds.y = wabbit->position.y;


	// check for collision with the terrain
	if (isColliding(wabbit->bounds, terrain->bounds))
	{
		DrawText(TextFormat("Collision detected"), WINDOW_WIDTH - 400, 350, 20, WHITE);
		// reset the wabbit's position to before the update
		wabbit->position.x -= wabbit->velocity.x;
		wabbit->position.y -= wabbit->velocity.y;
		wabbit->bounds.x = wabbit->position.x;

		// wabbit->position.y = terrain->bounds.y - wabbit->texture.height * wabbit->scale;
		wabbit->velocity.y = 0.0f;
		wabbit->bounds.y = wabbit->position.y;
	}
	else
		ApplyGravity(wabbit);
}



// This is the main function
int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Raylib");

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	Wabbit wabbit = CreateWabbit("wabbit_alpha.png", (Vector2){0,0}, 0.0f, 2.0f, WHITE, 1.0f, (Vector2){0,0});

	// create the terrain
	StaticTerrain sTerrain1 = CreateStaticTerrain((Rectangle){WINDOW_WIDTH/2-400, WINDOW_HEIGHT-200, 400, 200}, 0.0f, 1.0f, GREEN, 1.0f);
	
	// start the clock
	SetTargetFPS(60);	// Set the target frames-per-second

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{
		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		// Update
		// check user inputs
		HandleInput(&wabbit);

		// update the wabbit's state
		Update(&wabbit, &sTerrain1);

		// draw some text using the default font
		DrawText("Hello Raylib", 200,200,20, WHITE);
		// draw some text on the right side of the screen
		DrawText("Arrow keys to move left and right, SPACE to jump", 200, 250, 20, WHITE);

		// draw the wabbit's position and velocity on the right side of the screen
		DrawText(TextFormat("Wabbit position: (%.2f, %.2f)", wabbit.position.x, wabbit.position.y), WINDOW_WIDTH - 400, 200, 20, WHITE);
		DrawText(TextFormat("Wabbit velocity: (%.2f, %.2f)", wabbit.velocity.x, wabbit.velocity.y), WINDOW_WIDTH - 400, 250, 20, WHITE);

		// draw the wabbit
		DrawWabbit(&wabbit);
		// draw the terrain
		DrawStaticTerrain(&sTerrain1);
		
		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	DestroyWabbit(&wabbit);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
