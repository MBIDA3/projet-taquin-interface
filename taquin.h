#ifndef TAQUIN_H
#define TAQUIN_H

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>


#define GRID_SIZE_EASY 3   // Grille 3x3 pour facile
#define GRID_SIZE_NORMAL 4 // Grille 4x4 pour normal
#define GRID_SIZE_HARD 5   // Grille 5x5 pour difficile
#define MAX_MUSICS 3


#define MAX_HISTORY 100

typedef enum {
    EASY,
    NORMAL,
    HARD
}Difficulty;

typedef struct {
    int **grid;
    int empty_x, empty_y;
    Difficulty difficulty;
    int time_elapsed;
    int time_elapsed_off;
    int moves_count;
    char moves_history[MAX_HISTORY][20];
    int scrol_offset;
} GameState;


typedef struct {
    char message[100];
    Uint32 display_time; // Temps en millisecondes où le message doit disparaître
} TemporaryMessage;

typedef struct {
    int moving_number;  // Numéro de la case en mouvement
    int start_x, start_y;  // Position initiale
    int end_x, end_y;  // Position finale
    bool active;  // Indique si une animation est en cours
    float progress;  // Progression entre 0.0 et 1.0
} AnimationState;



typedef  struct {
    SDL_Color background;
    SDL_Color text;
    SDL_Color highlight;
    SDL_Color grid;
    SDL_Color title;
} Theme;



typedef enum {
    MENU_PRINCIPAL,
    MENU_DIFFICULTE,
    OPTIONS,
    MENU_MUSIQUE,
    HISTORIQUE,
    CHARGER,
    JEU
} MenuState;

extern TTF_Font *font;
extern SDL_Renderer *renderer;
extern GameState game;
extern MenuState current_menu;
extern TemporaryMessage temp_message;
extern AnimationState animation ;
extern Theme theme_light;
extern const Theme theme_dark;
extern Theme current_theme;

extern Mix_Chunk *move_sound;
extern Mix_Chunk *select_sound;
extern Mix_Music *background_music;
extern Mix_Chunk  *accept_sound;
extern Mix_Chunk * back_sound;

extern int scroll_offset;


extern Mix_Music *background_musics[MAX_MUSICS];  // Tableau de musiques
extern int current_music_index;  // Indice de la musique sélectionnée

void init_game(Difficulty difficulty);
void render_game(SDL_Renderer *renderer,TTF_Font*font);
void show_main_menu(SDL_Renderer *renderer, int selected_option);
void show_difficulty_menu(SDL_Renderer *renderer, int selected_option);
void show_option_menu(SDL_Renderer*renderer, int selected_option);
void handle_menu_input(SDL_Event *event, int *selected_option, SDL_Renderer *renderer);
void handle_difficulty_input(SDL_Event *e, int *selected_option);
void afficher_message(SDL_Renderer *renderer, const char *message, int x, int y);
void afficher_message_with_color(SDL_Renderer *renderer, const char *message, int x, int y, SDL_Color color);
void handle_game_input(SDL_Event *event);
void afficherNumero(SDL_Renderer*renderer,TTF_Font*font, int numero, int x, int y, int largeur, int hauteur);
void handle_key(SDL_Keycode key);
bool check_victory(GameState *game);
void game_loop(SDL_Renderer *renderer, TTF_Font *font, MenuState previous_menu);
void add_move_to_history(const char *move);
void handle_victory(SDL_Renderer *renderer, TTF_Font *font);
void reset_game_history();
void update_timer(Uint32 start_time);
void load_game(const char *filename);
void save_game(const char *filename);
void show_load_menu(SDL_Renderer *renderer, int selected_option);
void show_logo(SDL_Renderer *renderer);
void handle_load_menu_input(SDL_Event *event, int *selected_option);
void handle_options_menu_input(SDL_Event *event, int *selected_option);
void show_logo2(SDL_Renderer *renderer);
void show_player_history(SDL_Renderer *renderer, TTF_Font *font);
void render_background(SDL_Renderer *renderer,Uint32 ticks);
void save_player_to_history(const char *player_name);
void show_victory_window(SDL_Renderer *renderer, TTF_Font *font);
void transition_fade(SDL_Renderer *renderer);
void stop_background_music();
void load_musics();
void show_music_menu(SDL_Renderer *renderer, int selected_option);
void handle_music_menu_input(SDL_Event *event, int *selected_option);
void play_selected_music();
#endif

