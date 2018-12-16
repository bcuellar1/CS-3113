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
#include <vector>

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;

class Entity; //forward declaration

//Global Variables
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
SDL_Event event;
ShaderProgram untexturedShader;
ShaderProgram texturedShader;
vector <Entity> foreign_objects;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
GLuint spriteSheet;
GLuint fontTexture;
GLuint background;
GLuint sheetSpriteTexture;
bool done = false;
float lastFrameTicks = 0.0f;
float elapsed = 0.0f;
int sprite_count_x = 16;
int sprite_count_y = 8;


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

void DrawText(ShaderProgram &program, int fontTexture, string text, float x, float y, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    vector<float> vertexData;
    vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program.programID);
    program.SetModelMatrix(textModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, int(text.size()) * 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}


class TitleScreen{
public:
    void Render() {
        DrawText(texturedShader, fontTexture, "SpaceTravelers by Bryan Cuellar", -1.12,0,0.05,.01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void ProcessEvents(){
        //        while (SDL_PollEvent(&event)) {
        //        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
        //            done = true;
        //        }
        ////        else if (event.type == SDL_KEYDOWN){
        ////            if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
        ////                mode = STATE_GAME_LEVEL;
        ////                }
        ////            }
        //        }
    }
    void CleanUp(){
    }
};

class Background{
public:
    void changeScore(int points){
        score += points;
    }
    void run(){
        glClear(GL_COLOR_BUFFER_BIT);
        float vertexData[] = {-1.777, -1.0, 1.777, -1.0, 1.777, 1.0,  -1.777, -1.0, 1.777, 1.0, -1.777, 1.0};
        float texCoordData[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        projectionMatrix = glm::ortho(-1.777f, 1.777f, -1.0f, 1.0f, -1.0f, 1.0f);
        texturedShader.SetProjectionMatrix(projectionMatrix);
        texturedShader.SetModelMatrix(modelMatrix);
        texturedShader.SetViewMatrix(viewMatrix);
        glUseProgram(texturedShader.programID);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glVertexAttribPointer(texturedShader.positionAttribute, 2, GL_FLOAT, false, 0, vertexData);
        glEnableVertexAttribArray(texturedShader.positionAttribute);
        glVertexAttribPointer(texturedShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData);
        glEnableVertexAttribArray(texturedShader.texCoordAttribute);
        glBindTexture(GL_TEXTURE_2D, background);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(texturedShader.positionAttribute);
        glDisableVertexAttribArray(texturedShader.texCoordAttribute);
        DrawText(texturedShader, fontTexture, "Score: " + to_string(this->score), -1.70, 0.9, 0.05,0.01);
    }
private:
    int score = 0;
};

class Game{
public:
    class Background backdrop;
    void Setup(){
        //Sound setup
        Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
        Mix_Chunk *enemyShooting;
        Mix_Chunk *playerShooting;
        Mix_Chunk *explosion;
        Mix_Music *music;
        music = Mix_LoadMUS("03 Space Love Attack.mp3");
        playerShooting = Mix_LoadWAV("PlayerShooting.wav");
        enemyShooting = Mix_LoadWAV("enemyshooting.wav");
        explosion = Mix_LoadWAV("explosion.wav");
        //plays background continously
        Mix_PlayMusic(music, -1);
        //
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
        glewInit();
#endif
        glViewport(0, 0, 1280, 720);
        projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        texturedShader.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        untexturedShader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
        untexturedShader.SetViewMatrix(viewMatrix);
        untexturedShader.SetProjectionMatrix(projectionMatrix);
        untexturedShader.SetModelMatrix(modelMatrix);
        texturedShader.SetViewMatrix(viewMatrix);
        texturedShader.SetProjectionMatrix(projectionMatrix);
        texturedShader.SetModelMatrix(modelMatrix);
        fontTexture = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
        background = LoadTexture(RESOURCE_FOLDER"starry_background.png");
        sheetSpriteTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
    }
    void ProcessEvents(){
        
    }
    void Update(int elapased){
        
    }
    void CleanUp(){
        
    }
    void Render(){
        backdrop.run();
    }
    
    
    
};

struct SheetSprite{
    SheetSprite(int textureID = 0, float u = 0, float v = 0, float width = 0, float height = 0, float size = 0){
        this->textureID = textureID;
        this->u = u;
        this->v = v;
        this->width = width;
        this->height = height;
        this->size = size;
    }
    void Draw(ShaderProgram &program, float x, float y){
        glBindTexture(GL_TEXTURE_2D, textureID);
        GLfloat texCoords[] = {u, v+height, u+width, v, u, v, u+width, v, u, v+height, u+width, v+height};
        float aspect = width/height;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 1.0f));
        program.SetModelMatrix(modelMatrix);
        float vertices[] = {-0.5f * size * aspect, -0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, 0.5f * size,
            0.5f * size * aspect, 0.5f * size,
            -0.5f * size * aspect, -0.5f * size ,
            0.5f * size * aspect, -0.5f * size};
        glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(program.positionAttribute);
        glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(program.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(program.positionAttribute);
        glDisableVertexAttribArray(program.texCoordAttribute);
    }
    int textureID;
    float u;
    float v;
    float width;
    float height;
    float size;
};

class Entity{
public:
    Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r, float g, float b, float u, float v , int textureID, float size);
    void Draw(ShaderProgram &program, float elapsed){
        
    }
    void update(float elapsed);
    bool collision(Entity &other);
    glm::vec2 position;
    SheetSprite sprite;
    glm::vec2 velocity;
    bool collidedBottom = false;
    float width;
    float height;
    float rotation;
    int textureID;
    float r;
    float g;
    float b;
};



TitleScreen title;
Game SpaceTraveler;
Background backdrop;

void GameSetup(){
    SpaceTraveler.Setup();
}

void ProcessEvents(){
    switch(mode){
        case STATE_MAIN_MENU:
            title.ProcessEvents();
            break;
        case STATE_GAME_LEVEL:
            SpaceTraveler.ProcessEvents();
            break;
    }
}
void Update(float elapsedUpdate = elapsed){
    switch(mode){
        case STATE_MAIN_MENU:
            title.Update();
            break;
        case STATE_GAME_LEVEL:
            SpaceTraveler.Update(elapsedUpdate);
            break;
    }
}
void Render(){
    switch(mode){
        case STATE_MAIN_MENU:
            title.Render();
            break;
        case STATE_GAME_LEVEL:
            SpaceTraveler.Render();
            break;
    }
}
void Cleanup(){
    switch(mode){
        case STATE_MAIN_MENU:
            title.CleanUp();
            break;
        case STATE_GAME_LEVEL:
            SpaceTraveler.CleanUp();
            break;
    }
}


int main(int argc, char *argv[])
{
    GameSetup();
    //    SheetSprite player = SheetSprite(sheetSpriteTexture, 325.0f/1024.0f, 0.0f/1024.0f, 98.0f/1024.0f, 75.0f/1024.0f, .2f);
    //    SheetSprite enemy_black = SheetSprite(sheetSpriteTexture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    //    SheetSprite enemy_blue = SheetSprite(sheetSpriteTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    //    SheetSprite enemy_green = SheetSprite(sheetSpriteTexture, 425.0f/1024.0f, 552.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    //    SheetSprite bullet = SheetSprite(sheetSpriteTexture, 849.0f/1024.0f, 364.0f/1024.0f, 9.0f/1024.0f, 57.0f/1024.0f, .2f);
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            //            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            //                done = true;
            //            }
            //            else if (event.type == SDL_KEYDOWN){
            //                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
            //                    mode = STATE_GAME_LEVEL;
            //                    lastFrameTicks = (float)SDL_GetTicks()/1000.0f;
            //                }
            //            }
            ProcessEvents();
            Update();
            Render();
            SDL_GL_SwapWindow(displayWindow);
        }
        //Cleanup();
        SDL_Quit();
        return 0;
    }
    
}
