#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "FlareMap.h"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;

SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 projectionMatrix = glm::mat4(1.0f);
vector<float> vertexData;
vector<float> texCoordData;
float TILE_SIZE = 16.0;
int SPRITE_COUNT_X = 16;
int SPRITE_COUNT_Y = 16;

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / TILE_SIZE);
    *gridY = (int)(worldY / -TILE_SIZE);
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    //tiles
    program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glm::mat4 player_modelMatrix = glm::mat4(1.0f);
    glm::mat4 player_viewMatrix = glm::mat4(1.0f);
    glUseProgram(program.programID);
    

    //
    FlareMap Map;
    Map.Load("New_Tile_Game.txt"); // I don't know why this won't open
    int player_pos = 0;
    for(int i = 0; i < Map.entities.size(); i++){
        if(Map.entities[i].type == "Player"){
            player_pos = i;
        }
    }
   
    for(int y=0; y < Map.mapHeight; y++) {
        for(int x=0; x < Map.mapWidth; x++) {
            float u = (float)(((int)Map.mapData[y][x]) % SPRITE_COUNT_X) / (float) SPRITE_COUNT_X;
            float v = (float)(((int)Map.mapData[y][x]) / SPRITE_COUNT_X) / (float) SPRITE_COUNT_Y;
            float spriteWidth = 1.0f/(float)SPRITE_COUNT_X;
            float spriteHeight = 1.0f/(float)SPRITE_COUNT_Y;
            vertexData.insert(vertexData.end(), {
                TILE_SIZE * x, -TILE_SIZE * y,
                TILE_SIZE * x, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                TILE_SIZE * x, -TILE_SIZE * y,
                (TILE_SIZE * x)+TILE_SIZE, (-TILE_SIZE * y)-TILE_SIZE,
                (TILE_SIZE * x)+TILE_SIZE, -TILE_SIZE * y
            });
            texCoordData.insert(texCoordData.end(), {
                u, v,
                u, v+(spriteHeight),
                u+spriteWidth, v+(spriteHeight),
                u, v,
                u+spriteWidth, v+(spriteHeight),
                u+spriteWidth, v
            });
        }
    }
    
    float lastFrameTicks = 0.0f;
    float player_x = 0.0f;
    float player_y = 0.0f;
    float direction_x = 1.0f;
    float direction_y = 0.0f;
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            float ticks = (float)SDL_GetTicks()/1000.0f;
            float elapased = ticks - lastFrameTicks;
            lastFrameTicks = ticks;
            
            const Uint8 *keys = SDL_GetKeyboardState(NULL);

            //Map Drawing
            for(int y=0; y < Map.mapHeight; y++) {
                for(int x=0; x < Map.mapWidth; x++) {
                    if(Map.mapData[y][x] != 0) {
                        program.SetModelMatrix(modelMatrix);
                        program.SetProjectionMatrix(projectionMatrix);
                        program.SetViewMatrix(viewMatrix);
                        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false,0, &vertexData);
                        glEnableVertexAttribArray(program.positionAttribute);
                        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, &texCoordData);
                        glEnableVertexAttribArray(program.texCoordAttribute);
                        glDrawArrays(GL_TRIANGLES, 0, 6);
                        glDisableVertexAttribArray(program.positionAttribute);
                        glDisableVertexAttribArray(program.texCoordAttribute);
                        
                    }
                }
            }
            
            //Player drawing
            
            if (keys[SDL_SCANCODE_RIGHT] && player_x <= 1.777){
                direction_x = 1.0;
            }
            if (keys[SDL_SCANCODE_LEFT] && player_x >= -1.777){
                direction_x = -1.0;
            }
            player_x +=  elapased * Map.entities[player_pos].x * direction_x;
            player_y += elapased * Map.entities[player_pos].y * direction_y;
            
            player_modelMatrix = glm::mat4(1.0f);
            program.SetModelMatrix(player_modelMatrix);
            player_modelMatrix = glm::translate(player_modelMatrix, glm::vec3(player_y, player_x,0.0f));
            player_modelMatrix = glm::scale(player_modelMatrix, glm::vec3(0.05f, 0.30f, 1.0f));
            program.SetProjectionMatrix(projectionMatrix);
            program.SetViewMatrix(player_viewMatrix);
            float vertices_right[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
            glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false,0, vertices_right);
            glEnableVertexAttribArray(program.positionAttribute);
            float texCoords_right[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_right);
            glEnableVertexAttribArray(program.texCoordAttribute);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(program.positionAttribute);
            glDisableVertexAttribArray(program.texCoordAttribute);

            
            
        
            
        }
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
