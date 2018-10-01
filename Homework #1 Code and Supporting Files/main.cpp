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


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//Pink Alien Admiring Spinning coin which is on a pedestal

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}


int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Spining Coin Scene", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
  
    //background
    
    ShaderProgram program_background;
program_background.Load(RESOURCE_FOLDER"vertex_textured.glsl",RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint backgroundTexture = LoadTexture(RESOURCE_FOLDER"backgrounds.png");
    glm::mat4 projectionMatrix_back = glm::mat4(1.0f);
    projectionMatrix_back = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix_back = glm::mat4(1.0f);
    glm::mat4 viewMatrix_back = glm::mat4(1.0f);
    glUseProgram(program_background.programID);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    
    
    //alien
    
    ShaderProgram program_alien;
    program_alien.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint alienTexture = LoadTexture(RESOURCE_FOLDER"alienPink.png");
    glm::mat4 projectionMatrix_alien = glm::mat4(1.0f);
    projectionMatrix_alien = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix_alien = glm::mat4(1.0f);
    modelMatrix_alien = glm::translate(modelMatrix_alien, glm::vec3(-1.25f, -0.25f, 1.0f));
    modelMatrix_alien = glm::scale(modelMatrix_alien, glm::vec3(0.5f, 0.5f, 1.0f));
    glm::mat4 viewMatrix_alien = glm::mat4(1.0f);
    glUseProgram(program_alien.programID);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // Box/Pedestal
    
    ShaderProgram program_box;
    program_box.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    glm::mat4 projectionMatrix_box = glm::mat4(1.0f);
    projectionMatrix_box = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix_box = glm::mat4(1.0f);
    modelMatrix_box = glm::translate(modelMatrix_box, glm::vec3(-0.25f, -0.25f, 1.0f));
    modelMatrix_box = glm::scale(modelMatrix_box, glm::vec3(0.5f, 0.5f, 1.0f));
    glm::mat4 viewMatrix_box = glm::mat4(1.0f);
    glUseProgram(program_box.programID);
    
    // spinning coin
    
    ShaderProgram program_coin;
    program_coin.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    GLuint coinTexture = LoadTexture(RESOURCE_FOLDER"coinGold.png");
    glm::mat4 projectionMatrix_coin = glm::mat4(1.0f);
    projectionMatrix_coin = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
    glm::mat4 modelMatrix_coin = glm::mat4(1.0f);
    modelMatrix_coin = glm::translate(modelMatrix_coin, glm::vec3(-0.25f, 0.20f, 1.0f));
    modelMatrix_coin = glm::scale(modelMatrix_coin, glm::vec3(0.5f, 0.5f, 1.0f));
    float lastFrameTicks = 0.0f;
    
    glm::mat4 viewMatrix_coin = glm::mat4(1.0f);
    glUseProgram(program_coin.programID);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  
    
    
    
    SDL_Event event;
    bool done = false;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }

        //background
            glClear(GL_COLOR_BUFFER_BIT);

            program_background.SetModelMatrix(modelMatrix_back);
            program_background.SetProjectionMatrix(projectionMatrix_back);
            program_background.SetViewMatrix(viewMatrix_back);
            glBindTexture(GL_TEXTURE_2D, backgroundTexture);
            float vertices_two[] = {-1.777, -1.0, 1.777, -1.0, 1.777, 1.0,  -1.777, -1.0, 1.777, 1.0, -1.777, 1.0};
            glVertexAttribPointer(program_background.positionAttribute, 2, GL_FLOAT, false, 0, vertices_two);
            glEnableVertexAttribArray(program_background.positionAttribute);
            float texCoords_two[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(program_background.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_two);
            glEnableVertexAttribArray(program_background.texCoordAttribute);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(program_background.positionAttribute);
            glDisableVertexAttribArray(program_background.texCoordAttribute);

        
        //alien
        
       

        program_alien.SetModelMatrix(modelMatrix_alien);
        program_alien.SetProjectionMatrix(projectionMatrix_alien);
        program_alien.SetViewMatrix(viewMatrix_alien);
        glBindTexture(GL_TEXTURE_2D, alienTexture);
        float vertices_alien[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_alien.positionAttribute, 2, GL_FLOAT, false, 0, vertices_alien);
        glEnableVertexAttribArray(program_alien.positionAttribute);
        float texCoords_alien[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program_alien.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_alien);
        glEnableVertexAttribArray(program_alien.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program_alien.positionAttribute);
        glDisableVertexAttribArray(program_alien.texCoordAttribute);

        
        //box
        
        program_box.SetModelMatrix(modelMatrix_box);
        program_box.SetProjectionMatrix(projectionMatrix_box);
        program_box.SetViewMatrix(viewMatrix_box);
        float vertices_box[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_box.positionAttribute, 2, GL_FLOAT, false, 0, vertices_box);
        glEnableVertexAttribArray(program_box.positionAttribute);
        float texCoords_box[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program_box.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_box);
        glEnableVertexAttribArray(program_box.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program_box.positionAttribute);
        glDisableVertexAttribArray(program_box.texCoordAttribute);
        
        //spinning coin !!!!
        
        program_coin.SetModelMatrix(modelMatrix_coin);
        program_coin.SetProjectionMatrix(projectionMatrix_coin);
        program_coin.SetViewMatrix(viewMatrix_coin);
        glBindTexture(GL_TEXTURE_2D, coinTexture);
        float vertices_coin[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(program_coin.positionAttribute, 2, GL_FLOAT, false, 0, vertices_coin);
        glEnableVertexAttribArray(program_coin.positionAttribute);
        float texCoords_coin[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(program_coin.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords_coin);
        glEnableVertexAttribArray(program_coin.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program_coin.positionAttribute);
        glDisableVertexAttribArray(program_coin.texCoordAttribute);
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        float angle = 180.0f * (3.1415926f / 180.0f);
        angle *= elapsed;
        modelMatrix_coin= glm::rotate(modelMatrix_coin, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        
        
        SDL_GL_SwapWindow(displayWindow);
        
    
    }
    SDL_Quit();
    return 0;
    
}


