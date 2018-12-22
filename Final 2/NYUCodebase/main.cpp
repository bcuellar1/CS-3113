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
#include <cstdlib>
#include <math.h>



#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
using namespace std;

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

class Entity; //forward declaration

//Global Variables
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
SDL_Event event;
ShaderProgram untexturedShader;
ShaderProgram texturedShader;
vector <Entity> foreign_objects;
vector <Entity> user;
vector <Entity> bullets;
vector <Entity> curr_enemy_enitities;
enum GameMode {STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_END_GAME};
GameMode mode = STATE_MAIN_MENU;
GLuint spriteSheet;
GLuint fontTexture;
GLuint background;
GLuint sheetSpriteTexture;
bool done = false;
float lastFrameTicks;
float ticks;
float elapsed;
float accumulator;
float alien_sensor;
int max_foreign_entities;
int bullet_pos;
int total_score;
Mix_Chunk *enemyShooting;
Mix_Chunk *playerShooting;
Mix_Chunk *explosion;
Mix_Music *music;


SDL_Window* displayWindow;

//used
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

//used
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



//used
class Background{
public:
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
        DrawText(texturedShader, fontTexture, "Score: " + to_string(total_score), -1.70, 0.9, 0.05,0.01);
    }
};

//used
class TitleScreen{
public:
    class Background backdrop;
    void Render() {
        DrawText(texturedShader, fontTexture, "SpaceTravelers by Bryan Cuellar", -1.12,0,0.05,.01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void End(){
        DrawText(texturedShader, fontTexture, "Game Over", -1.12,0,0.05,.01);
        DrawText(texturedShader, fontTexture, "Score: " + to_string(total_score), -1.12, -0.2, 0.05,0.01);
    }
};

//used
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
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, 1.0f));
        //need to check if spinning
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

//used
struct vec2 {
    vec2(float x, float y): x(x), y(y){}
    float x;
    float y;
};

//used
struct Entity{
    Entity(float x, float y, float velocity_x, float velocity_y, float width, float height ,float u = 0, float v = 0, int textureID = 0, float size = 0): position(x,y), velocity(velocity_x, velocity_y){
        this->width = width;
        this->height = height;
        this->sprite = SheetSprite(textureID, u, v ,width, height, size);
    }
    void Draw(ShaderProgram &program){
        sprite.Draw(program, position.x, position.y);
    }
    void update(float elapsed){
        position.x += velocity.x * elapsed;
        position.y += velocity.y * elapsed;
    }
    bool collision(Entity &other){
        float x_distance = abs(other.position.x-position.x)-((other.width+width));
        float y_distance = abs(other.position.y-position.y)-((other.height+height));
        if(x_distance < 0 && y_distance < 0){
            return true;
        }
        return false;
    }
    bool edge(){
        if(position.x - width/2 <= -1.777 || position.x + width/2 >= 1.777){
            return true;
        }
        else{
            return false;
        }
    }
    void swap(){
        position.x *= -1;
    }
    void added(){
        if(add == false){
            curr_enemy_enitities.push_back(*this);
            add = true;
        }
    }
    void alien(){
        isAlien = true;
    }
    void left(){
        int random = rand() % 2;
        if(random == 1){
            going_left = -1;
        }
        else{
            going_left = 0;
        }
    }
    void alien_edge(){
        float edge = (position.x + width/2) + 0.1;
        if(position.x > 0){
            if(edge >= 1.777 ){
                velocity.x *= -1;
            }
        }
        else{
            if(edge <= -1.777){
                velocity.x *= 1;
            }
        }
    }
    vec2 position;
    SheetSprite sprite;
    vec2 velocity;
    bool collidedRight = false;
    bool collidedLeft = false;
    bool pos_check = true;
    bool add = false;
    bool isAlien = false;
    int going_left = -1;
    bool used = false;
    bool user = true;
    bool alien_shot = true;
    bool out_of_range = false;
    int alien_pos = 0;
    int score = 10;
    float width;
    float height;
    float rotation;
    int textureID;
    float r;
    float g;
    float b;
};

//used
bool entity_check(int pos){
    float top_of_entity = 0.0;
    top_of_entity = curr_enemy_enitities[pos].position.y - (curr_enemy_enitities[pos].height/2);
    if(top_of_entity <= -2.5){
        return true;
    }
    else{
        return false;
    }
}

//used
void Randomdraw(){
    if(curr_enemy_enitities.size()+1 < max_foreign_entities){
        //picks random number between 0-9
        int random_num = 0;
        int pos_or_neg = 0;
        float random_x = 0.0;
        float out_of_range = 0.0;
        float offset = 0.0;
        random_num = rand() % 10;
        pos_or_neg = rand() % 2;
        //chooses random position -1.667 to 1.667 on x axis, y axis is set
        random_x = fmod(float(rand()), 1.667);;
        if(pos_or_neg == 0){
            random_x *= -1;
        }
        if(random_x > 0){
            out_of_range = random_x + (foreign_objects[random_num].width/2.0);
            if(out_of_range >= 1.777){
                offset = out_of_range - 1.777;
                random_x = random_x - (offset + (foreign_objects[random_num].width/2.0));
            }
        }
        if(random_x < 0){
            out_of_range = random_x - (foreign_objects[random_num].width *0.5);
            if(out_of_range <= -1.777){
                offset = out_of_range + 1.777;
                random_x = random_x + (offset - (foreign_objects[random_num].width *0.5));
            }
        }
        //draws enitity and checks if it needs to go left or right
        if(foreign_objects[random_num].going_left == -1 && foreign_objects[random_num].isAlien){
            foreign_objects[random_num].left();
        }
        foreign_objects[random_num].position.x = random_x;
        foreign_objects[random_num].Draw(texturedShader);
        foreign_objects[random_num].added();
    }
    else{
        for(int i =0; i < curr_enemy_enitities.size(); i++){
            //check if entity left screen
            if(i == 0 && entity_check(i)){
                if(curr_enemy_enitities[i].isAlien){
                    curr_enemy_enitities[i].going_left = -1;
                }
                curr_enemy_enitities.erase(curr_enemy_enitities.begin());
                continue;
            }
            else if((i+1 < curr_enemy_enitities.size()) && entity_check(i+1)){
                if(curr_enemy_enitities[i].isAlien){
                    curr_enemy_enitities[i].going_left = -1;
                }
                //remove
                Entity temp = curr_enemy_enitities[i+1];
                curr_enemy_enitities[i+1] = curr_enemy_enitities[curr_enemy_enitities.size() -1];
                curr_enemy_enitities[curr_enemy_enitities.size()-1] = temp;
                curr_enemy_enitities.pop_back();
            }
            else if( i == 9 && entity_check(i) ){
                if(curr_enemy_enitities[i].isAlien){
                    curr_enemy_enitities[i].going_left = -1;
                }
                curr_enemy_enitities.erase(curr_enemy_enitities.end());
                continue;
            }
            else{
                curr_enemy_enitities[i].Draw(texturedShader);
            }
        }
    }
}

//used
bool range(int pos){
    if(bullets[pos].used){
        if((bullets[pos].position.y + bullets[pos].width/2) >= 2.5 || (bullets[pos].position.y - bullets[pos].width/2) <= -2.5){
            bullets[pos].used = false;
            bullets[pos].position.x = 0.0;
            bullets[pos].position.y = -0.55;
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

//used
void Bulletdraw(){
    //include bool to see if bullet should be drawn
    for(int i = 0; i < bullets.size(); i++){
        if(bullets[i].used){
            if(bullets[i].user){
                bullets[i].Draw(texturedShader);
            }
            //have to be an alien
            else if(bullets[i].alien_shot){
                bullets[i].Draw(texturedShader);
            }
            else{
                continue;
            }
        }
    }
}

//used
void playSound(Mix_Chunk *sound){
    Mix_PlayChannel(-1, sound, 0);
}

void deleteEntity(Entity &Entity, int entity_pos, int vec){
    if(vec == 0){
        //cout << Entity.score << endl;
        total_score += Entity.score;
        curr_enemy_enitities[entity_pos].position.y = 1.5;
    }
}

//used
class Game{
public:
    class Background backdrop;
    void Setup(){
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
        lastFrameTicks = 0.0f;
        accumulator = 0.0f;
        ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
    }
    
    void Update(float elapsed){
        //update player
        glClear(GL_COLOR_BUFFER_BIT);
        //is something hitting player
        for(int i = 0; i < curr_enemy_enitities.size(); i++){
            if(user[0].collision(curr_enemy_enitities[i])){
                //screenShake();
                playSound(explosion);
                mode = STATE_END_GAME;
            }
        }
        //bullets hitting foreign_entities
        for(int i = 0; i < curr_enemy_enitities.size(); i++){
            for(int j = 0; j < bullets.size(); j++){
                if(bullets[i].user || bullets[i].alien_shot){
                        if(bullets[j].collision(curr_enemy_enitities[i])){
                            //screenShake();
                            playSound(explosion);
                            deleteEntity(curr_enemy_enitities[i], i, 0);
                        }
                    }
                }
        }

        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        //need to check if sprite is at end of screen
        if(keys[SDL_SCANCODE_LEFT]) {
            if(user[0].edge()){
                user[0].swap();
            }
            user[0].velocity.x = -2.0;
        }
        else if(keys[SDL_SCANCODE_RIGHT]){
            if(user[0].edge()){
                user[0].swap();
            }
            user[0].velocity.x = 2.0;
        }
        else if(keys[SDL_SCANCODE_F]){
            // add logic to see if the bullet should be used
            for(int i = 0; i < bullets.size(); i++){
                if(bullet_pos >= bullets.size()){
                    bullet_pos = 0;
                    continue;
                }
                else if(!bullets[bullet_pos].used){
                    bullets[bullet_pos].position.x = user[0].position.x;
                    bullets[bullet_pos].user = true;
                    bullets[bullet_pos].used = true;
                    bullets[bullet_pos].velocity.y = 1.0;
                    bullet_pos++;
                    playSound(playerShooting);
                    break;
                }
                else{
                    bullet_pos++;
                }
            }
        }
            
        else{
            user[0].velocity.x = 0;
        }
        //have entities go down
        for(int i = 0; i < curr_enemy_enitities.size(); i++){
            if(curr_enemy_enitities[i].isAlien){
                //determineShot(i);
                if(curr_enemy_enitities[i].going_left){
                    curr_enemy_enitities[i].velocity.x = -0.5;
                    if(curr_enemy_enitities[i].edge()){
                        curr_enemy_enitities[i].swap();
                    }
                }
                else{
                    curr_enemy_enitities[i].velocity.x = 0.5;
                    if(curr_enemy_enitities[i].edge()){
                        curr_enemy_enitities[i].swap();
                    }
                }
            }
            curr_enemy_enitities[i].velocity.y = -0.5;
            curr_enemy_enitities[i].update(elapsed);
        }
        for(int i = 0; i < bullets.size(); i++){
            if(bullets[i].user || bullets[i].alien_shot){
                //during update check to see if bullet is out of range
                if(!range(i)){
                    bullets[i].update(elapsed);
                }
            }
            else{
                continue;
            }
        }
        user[0].update(elapsed);
        //make entity move left/right or spin
        //make entity shoot
        
    }
    void Render(){
        glClear(GL_COLOR_BUFFER_BIT);
        backdrop.run();
        //drawplayer
        user[0].Draw(texturedShader);
        //draw entities
        //randomly draws entities
        Randomdraw();
        Bulletdraw();
    }
};

TitleScreen title;
Game SpaceTraveler;
Background backdrop;

void GameSetup(){
    SpaceTraveler.Setup();
}

void EntitySetup(){
    //Sound setup
    int bullet_pos = 0;
    int total_score = 0;
    float screenShakeSpeed = 50000.0;
    int screenShakeValue = 50000.0;
    float screenShakeIntensity = 50000.0;
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    music = Mix_LoadMUS("03 Space Love Attack.mp3");
    playerShooting = Mix_LoadWAV("PlayerShooting.wav");
    enemyShooting = Mix_LoadWAV("enemyshooting.wav");
    explosion = Mix_LoadWAV("explosion.wav");
    //plays background continouslyf
    Mix_PlayMusic(music, -1);
    max_foreign_entities = 5;
    SheetSprite spaceship = SheetSprite(sheetSpriteTexture, 325.0f/1024.0f, 0.0f/1024.0f, 98.0f/1024.0f, 75.0f/1024.0f, .2f);
    SheetSprite enemy1 = SheetSprite(sheetSpriteTexture, 423.0f/1024.0f, 728.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    SheetSprite enemy2 = SheetSprite(sheetSpriteTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    SheetSprite enemy3 = SheetSprite(sheetSpriteTexture, 425.0f/1024.0f, 552.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, .2f);
    SheetSprite laser = SheetSprite(sheetSpriteTexture, 849.0f/1024.0f, 364.0f/1024.0f, 9.0f/1024.0f, 57.0f/1024.0f, .2f);
    SheetSprite meteor1 = SheetSprite(sheetSpriteTexture, 651/1024.0f,447/1024.0f,43/1024.0f,43/1024.0f,.2f);
    SheetSprite meteor2 = SheetSprite(sheetSpriteTexture, 674/1024.0f, 219/1024.0f, 43/1024.0f, 43/1024.0f);
    Entity player = Entity(0.0f, -0.8f, 0.0f, 0.0f, spaceship.width,spaceship.height, spaceship.u ,spaceship.v, spaceship.textureID,0.2f);
    user.push_back(player);
    for(int i = 0; i <11; i++){
        Entity bullet = Entity(0.0f,-0.55f,0.0f,0.0f, laser.width, laser.height, laser.u, laser.v, laser.textureID, 0.2f);
        bullets.push_back(bullet);
    }
    
    //adds 2 of every object only want ten entities at a time
    for(int i = 0; i < 2; i++){
        Entity enemy_black = Entity(0.0f,2.5f,0.0f,0.0f, enemy1.width, enemy1.height, enemy1.u, enemy1.v, enemy1.textureID, 0.2f);
        enemy_black.alien();
        //enemy_black.left();
        Entity enemy_blue = Entity(0.0f,2.5f,0.0f,0.0f, enemy2.width, enemy2.height, enemy2.u, enemy2.v, enemy2.textureID, 0.2f);
        enemy_blue.alien();
        Entity enemy_green = Entity(0.0f,2.5f,0.0f,0.0f, enemy3.width, enemy3.height, enemy3.u, enemy3.v, enemy3.textureID, 0.2f);
        enemy_green.alien();
        //enemy_green.left();
        Entity meteor_brown = Entity(0.0f,2.5f,0.0f,0.0f, meteor1.width, meteor1.height, meteor1.u, meteor1.v, meteor1.textureID, 0.2f);
        Entity meteor_grey = Entity(0.0f,2.5f,0.0f,0.0f, meteor2.width, meteor2.height, meteor2.u, meteor2.v, meteor2.textureID, 0.2f);
        foreign_objects.push_back(enemy_black);
        foreign_objects.push_back(enemy_blue);
        foreign_objects.push_back(enemy_green);
        foreign_objects.push_back(meteor_brown);
        foreign_objects.push_back(meteor_grey);
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
        case STATE_END_GAME:
            title.Update();
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
        case STATE_END_GAME:
            title.End();
    }
}


int main(int argc, char *argv[])
{
    GameSetup();
    EntitySetup();
    while (!done) {
        srand(time(NULL));
        while (SDL_PollEvent(&event)) {
            const Uint8 *keys = SDL_GetKeyboardState(NULL);
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            //if key is pressed down
            else if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                }
                else if(keys[SDL_SCANCODE_Q]){
                    mode = STATE_END_GAME;
                }
            }
        }
        
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue;
        }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;
        Render();
        SDL_GL_SwapWindow(displayWindow);
        }
    SDL_Quit();
    return 0;
}
