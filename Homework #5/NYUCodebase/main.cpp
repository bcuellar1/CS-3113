#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL_mixer.h>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    //Sound setup
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    Mix_Chunk *rightSound;
    Mix_Chunk *leftSound;
    Mix_Music *music;
    music = Mix_LoadMUS("03 Space Love Attack.mp3");
    rightSound = Mix_LoadWAV("right_win.wav");
    leftSound = Mix_LoadWAV("left_win.wav");
    
    //plays background continously
    Mix_PlayMusic(music, -1);
    
    //left paddle set up
    ShaderProgram program;
    program.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix_left_paddle = glm::mat4(1.0f);
    glm::mat4 viewMatrix_left_paddle = glm::mat4(1.0f);
    glUseProgram(program.programID);
    
    float left_y_pos = 0.0f;
    
    //right paddle set up
    glm::mat4 modelMatrix_right_paddle = glm::mat4(1.0f);
    glm::mat4 viewMatrix_right_paddle = glm::mat4(1.0f);
    glUseProgram(program.programID);
    
    float right_y_pos = 0.0f;
    
    
    //box setup
    
    glm::mat4 modelMatrix_box = glm::mat4(1.0f);
    glm::mat4 viewMatrix_box = glm::mat4(1.0f);
    glUseProgram(program.programID);
    
    //all variables used
    float lastFrameTicks = 0.0f;
    float distance_to_travel = 1.0f;
    float direction_to_travel_x = 0.0f;
    float direction_to_travel_y = 0.0f;
    float box_pos_x = 0.0f;
    float box_pos_y = 0.0f;
    float direction_vector_x = cos(45);
    float direction_vector_y = sin(45);
    float win_right = 0.0f;
    float win_left = 0.0f;
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapased = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        
        
        //left Paddle "AI paddle"
        
        if (keys[SDL_SCANCODE_W] && left_y_pos <= .85){
            direction_to_travel_y = 1.0;
        } else if(keys[SDL_SCANCODE_S] && left_y_pos >= -.85){
            direction_to_travel_y = -1.0;
        }
        else{
            direction_to_travel_y = 0.0;
        }
        
        left_y_pos += elapased * distance_to_travel * direction_to_travel_y;
        
        modelMatrix_left_paddle = glm::mat4(1.0f);
        modelMatrix_left_paddle = glm::translate(modelMatrix_left_paddle, glm::vec3(-1.60f, left_y_pos, 1.0f));
        modelMatrix_left_paddle = glm::scale(modelMatrix_left_paddle, glm::vec3(0.05f, 0.30f, 1.0f));
        program.SetModelMatrix(modelMatrix_left_paddle);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix_left_paddle);
        float vertices_left[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false,0, vertices_left);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords_left[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_left);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //right paddle I.E Human Player
        
        if (keys[SDL_SCANCODE_UP] && right_y_pos <= .85){
            direction_to_travel_x = 1.0;
        } else if(keys[SDL_SCANCODE_DOWN] && right_y_pos >= -.85){
            direction_to_travel_x = -1.0;
        }
        else{
            direction_to_travel_x = 0.0;
        }
        
        
        right_y_pos += elapased * distance_to_travel * direction_to_travel_x;
        
        
        modelMatrix_right_paddle = glm::mat4(1.0f);
        modelMatrix_right_paddle = glm::translate(modelMatrix_right_paddle, glm::vec3(1.6f, right_y_pos,0.0f));
        modelMatrix_right_paddle = glm::scale(modelMatrix_right_paddle, glm::vec3(0.05f, 0.30f, 1.0f));
        program.SetModelMatrix(modelMatrix_right_paddle);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix_right_paddle);
        float vertices_right[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false,0, vertices_right);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords_right[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_right);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        //Code for Ball
        
        //checks if a "player" has won
        
        win_right = box_pos_x + .08/2;
        win_left = box_pos_x - .08/2;
        
        if(win_right >= 1.777){
            //flashes red
            glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            //reset box
            box_pos_x = 0.0f;
            box_pos_y = 0.0f;
            //play right winner sound
            Mix_PlayChannel(-1, rightSound, 0);
        } else if(win_left <= -1.777){
            //flashes blue
            glClearColor(0.0f, 0.0f, 1.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            //reset box
            box_pos_x = 0.0f;
            box_pos_y = 0.0f;
            //play left winner sound
            Mix_PlayChannel(-1, leftSound, 0);
        }
        
        box_pos_x += direction_vector_x * elapased * distance_to_travel;
        box_pos_y += direction_vector_y * elapased * distance_to_travel;
        
        //checks for top bottom collision
        float collision_top = box_pos_y + .08/2;
        float collision_bottom = box_pos_y - .08/2;
        if(collision_top >= 1){
            direction_vector_y *= -1;
        }else if(collision_bottom <= -1){
            direction_vector_y *= -1;
        }
        
        // checks right and left paddle collision
        
        float collision_right = box_pos_x + .08/2;
        float collision_left = box_pos_x - .08/2;
        if(collision_right >= 1.6 && (box_pos_y +.08/2) >= right_y_pos){
            direction_vector_x *= -1;
        }else if(collision_left <= -1.6 && (box_pos_y - .08/2) <= left_y_pos){
            direction_vector_x *= -1;
        }
        
        modelMatrix_box = glm::mat4(1.0f);
        modelMatrix_box = glm::translate(modelMatrix_box, glm::vec3(box_pos_x, box_pos_y, 1.0f));
        modelMatrix_box = glm::scale(modelMatrix_box, glm::vec3(0.08f, 0.08f, 1.0f));
        program.SetModelMatrix(modelMatrix_box);
        program.SetProjectionMatrix(projectionMatrix);
        program.SetViewMatrix(viewMatrix_box);
        float vertices_box[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices_box);
        glEnableVertexAttribArray(program.positionAttribute);
        float texCoords_box[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_box);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
        
        
        
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}

