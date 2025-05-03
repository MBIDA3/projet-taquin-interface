#include "taquin.h"
#include <stdlib.h>
#include <time.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <string.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

#define CASE_SIZE 100
#define SPACE_SIZE 10

GameState game;
int running = 0;
int victoire = 0;
bool stop_timer = false;
bool has_victory_displayed = false;
Difficulty current_difficulty = NORMAL;
Mix_Music *background_musics[MAX_MUSICS] = {NULL};
int current_music_index = 0;  // Par défaut, première musique
TemporaryMessage temp_message = {"", 0};

int scroll_index = 0;
int max_visible_lines = 100;

Mix_Chunk *move_sound = NULL;
Mix_Chunk *select_sound = NULL;
Mix_Music *background_music = NULL;
Mix_Chunk *accept_sound = NULL;
Mix_Chunk *back_sound = NULL;


Theme theme_light = {
        {255,255,255},
        {0,0,0},
        {200,200,200},
        {50,50,50},
        {100,150,200},
};

const Theme theme_dark = {
        {0,0,0},
        {255,255,255},
        {50,50,50},
        {200,200,200},
        {100,100,150},
};

Theme current_theme = theme_dark;


bool check_victory(GameState *ptr);

void init_game(Difficulty difficulty) {

    int grid_size;
    int num = 1;

    switch (difficulty) {
        case EASY:
            grid_size = GRID_SIZE_EASY;
            break;
        case NORMAL:
            grid_size = GRID_SIZE_NORMAL;
            break;
        case HARD:
            grid_size = GRID_SIZE_HARD;
            break;
    }

    game.difficulty = difficulty;
    game.grid = malloc(grid_size * sizeof(int *));
    for (int i = 0; i < grid_size; i++) {
        game.grid[i] = malloc(grid_size * sizeof(int));
    }


    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            game.grid[i][j] = num++;
        }
    }
    game.grid[grid_size - 1][grid_size- 1] = 0; // Case vide
    game.empty_x = grid_size - 1;
    game.empty_y = grid_size - 1;

    srand(time(NULL));

    for (int i = 0; i < 100; i++) {

        int direction = rand() % 4;

        switch (direction) {
            case 0: handle_key(SDLK_UP); break;
            case 1: handle_key(SDLK_DOWN); break;
            case 2: handle_key(SDLK_LEFT); break;
            case 3: handle_key(SDLK_RIGHT); break;
        }
    }
    reset_game_history();
}

void handle_key(SDL_Keycode key) {
    int grid_size = (game.difficulty == EASY) ? GRID_SIZE_EASY :
                    (game.difficulty == NORMAL) ? GRID_SIZE_NORMAL :
                    GRID_SIZE_HARD;

    int new_x = game.empty_x;
    int new_y = game.empty_y;

    switch (key) {
        case SDLK_UP:    add_move_to_history("HAUT"),new_x--; break;
        case SDLK_DOWN:  add_move_to_history("BAS"),new_x++; break;
        case SDLK_LEFT:  add_move_to_history("GAUCHE"),new_y--; break;
        case SDLK_RIGHT: add_move_to_history("DROITE"),new_y++; break;
        case SDLK_f: {
            const char *filename = (game.difficulty == EASY) ? "save_game_easy.txt" :
                                   (game.difficulty == NORMAL) ? "save_game_normal.txt" :
                                   "save_game_hard.txt";save_game(filename);break;// Sauvegarde basée sur le niveau
        }
        case SDLK_k: // Scroll vers le haut
            scroll_index++;
            break;
        case SDLK_l:
            scroll_index--;
            break;
        case SDLK_s:
            stop_background_music();
            break;
    }
    Mix_PlayChannel(-1, move_sound, 0); // le son de déplacement

    if (new_x >= 0 && new_x < grid_size && new_y >= 0 && new_y < grid_size) {

        int number = game.grid[new_x][new_y];  // Numéro de la case à déplacer
        int start_x = new_y * 100;  // Coordonnée de départ
        int start_y = new_x * 100;  // Coordonnée de départ
        int end_x = game.empty_y * 100;  // Coordonnée de destination
        int end_y = game.empty_x * 100;  // Coordonnée de destination

        // Échange les valeurs dans la grille
        game.grid[game.empty_x][game.empty_y] = number;
        game.grid[new_x][new_y] = 0;
        game.empty_x = new_x;
        game.empty_y = new_y;


    }
}



void show_difficulty_menu(SDL_Renderer *renderer, int selected_option) {

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    const char *title = "CHOISIR UNE DIFFICULTE";
    SDL_Color title_color = {255, 255, 255};  // Blanc
    afficher_message(renderer, title, 250, 50);


    const char *options[] = {"Facile (3x3)", "Normal (4x4)", "Difficile (6x6)","Retour"};
    for (int i = 0; i < 4; i++) {
        SDL_Color color = (i == selected_option) ? current_theme.highlight : current_theme.text;
        afficher_message(renderer, options[i], 300, 200 + i * 50);


    }
    int cursor_x = 270;
    int cursor_width = 200;
    int cursor_height = 40;
    int cursor_y = 190 + selected_option * 50;

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect cursor_rect = {cursor_x, cursor_y, cursor_width, cursor_height};
    SDL_RenderDrawRect(renderer, &cursor_rect);
    SDL_RenderPresent(renderer);
}


void handle_difficulty_input(SDL_Event *event, int *selected_option) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (*selected_option > 0){Mix_PlayChannel(-1, select_sound, 0); (*selected_option)--;}
                break;
            case SDLK_DOWN:
                if (*selected_option < 3) { Mix_PlayChannel(-1, select_sound, 0);(*selected_option)++;}
                break;
            case SDLK_RETURN:  // Action sur sélection
                switch (*selected_option) {
                    case 0:
                        printf("Difficulté sélectionnée : FACILE\n");
                        current_difficulty = *selected_option;
                        init_game(EASY);
                        play_selected_music();
                        current_menu = JEU;  // Passage directement au jeu
                        break;
                    case 1:
                        printf("Difficulté sélectionnée : NORMAL\n");
                        current_difficulty = *selected_option;
                        init_game(NORMAL);
                        play_selected_music();
                        current_menu = JEU;
                        break;
                    case 2:
                        printf("Difficulté sélectionnée : DIFFICILE\n");
                        current_difficulty = *selected_option;
                        init_game(HARD);
                        play_selected_music();
                        current_menu = JEU;
                        break;
                    case 3:
                        current_menu = MENU_PRINCIPAL;
                        Mix_PlayChannel(-1, back_sound, 0);
                        break;
                }
                break;
            case SDLK_ESCAPE:  // Retour au menu principal
                current_menu = MENU_PRINCIPAL;// Retour au menu principal
                //init_game();
                break;
        }
    }
}

void handle_options_menu_input(SDL_Event *event, int *selected_option) {
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (*selected_option > 0) { Mix_PlayChannel(-1, select_sound, 0); (*selected_option)--;}
                break;
            case SDLK_DOWN:
                if (*selected_option < 2) {Mix_PlayChannel(-1, select_sound, 0); (*selected_option)++;}
                break;
            case SDLK_RETURN:
                switch (*selected_option) {
                    case 0:  // Activer le thème clair
                        current_theme = theme_light;
                        break;
                    case 1:  // Activer le thème sombre
                        current_theme = theme_dark;
                        break;
                    case 2:  // Retour au menu principal
                        current_menu = MENU_PRINCIPAL;
                        Mix_PlayChannel(-1, back_sound, 0);
                        break;
                }
                break;
            case SDLK_ESCAPE:
                current_menu = MENU_PRINCIPAL;  // Retour au menu principal
                break;
        }
    }
}

void render_game(SDL_Renderer *renderer, TTF_Font *font) {

    SDL_SetRenderDrawColor(renderer, current_theme.background.r, current_theme.background.g, current_theme.background.b, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect menu_rect = {550, 0, 450, 497};
    SDL_RenderFillRect(renderer, &menu_rect);

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Temps : %02d:%02d", game.time_elapsed / 60, game.time_elapsed % 60);
    afficher_message(renderer, buffer, 560, 50);

    afficher_message(renderer, "Mouvements :", 560, 120);
    SDL_Rect history_clip = {560, 150, 380, 120};
    SDL_RenderSetClipRect(renderer, &history_clip);

    int y_offset = 150;
    for (int i = 0; i < max_visible_lines; i++) {
        int history_index = scroll_index + i;
        if (history_index < game.moves_count) {
            char buffer[100];
            snprintf(buffer, sizeof(buffer), "- %s", game.moves_history[history_index]);
            afficher_message(renderer, buffer, 570, y_offset);
            y_offset += 20;
        }
    }

    SDL_RenderSetClipRect(renderer, NULL);

    const char *difficulty_text = game.difficulty == EASY ? "Facile" :
                                  game.difficulty == NORMAL ? "Normal" : "Difficile";
    snprintf(buffer, sizeof(buffer), "Niveau : %s", difficulty_text);
    afficher_message(renderer, buffer, 560, 300);

    int grid_size;
    if (game.difficulty == EASY) {
        grid_size = GRID_SIZE_EASY;
    } else if (game.difficulty == NORMAL) {
        grid_size = GRID_SIZE_NORMAL;
    } else if (game.difficulty == HARD) {
        grid_size = GRID_SIZE_HARD;
    }
    play_selected_music();
    int cell_width = 550 / grid_size;
    int cell_height = 500 / grid_size;

    bool victory = check_victory(&game);

    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            SDL_Rect cell = {j * cell_width, i * cell_height, cell_width, cell_height};

            if (victory) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 150);
                SDL_RenderFillRect(renderer, &cell);
            } else if (game.grid[i][j] != 0) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderDrawRect(renderer, &cell);
                afficherNumero(renderer, font, game.grid[i][j], cell.x, cell.y, cell_width, cell_height);
            } else {
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderFillRect(renderer, &cell);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderDrawRect(renderer, &cell);
            }
        }
    }

    if (current_menu == JEU) {
        SDL_Rect menu_rect = {550, 500, 260, 200};
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &menu_rect);
        afficher_message(renderer, "OPTION :", 560, 520);
        afficher_message(renderer, "F : Sauvegarder", 560, 590);
        afficher_message(renderer, "C : Charger", 560, 620);
    }
    handle_victory(renderer, font);

    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
}

bool check_victory(GameState *game) {
    int grid_size = (game->difficulty == EASY) ? GRID_SIZE_EASY :
                    (game->difficulty == NORMAL) ? GRID_SIZE_NORMAL :
                    GRID_SIZE_HARD;

    int num = 1;  // La séquence correcte commence à 1
    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            if (i == grid_size - 1 && j == grid_size - 1) {
                // Dernière case doit être vide (0)
                if (game->grid[i][j] != 0) return false;
            } else if (game->grid[i][j] != num++) {
                return false;  // Mauvaise case
            }
        }
    }
    return true;
}



void afficher_message(SDL_Renderer *renderer, const char *message, int x, int y) {
    SDL_Color color = {255, 255, 255};  // Blanc
    SDL_Surface *surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void afficher_message_with_color(SDL_Renderer *renderer, const char *message, int x, int y, SDL_Color color) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, message, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void show_main_menu(SDL_Renderer *renderer, int selected_option) {
    Uint32 ticks = SDL_GetTicks();
    render_background(renderer,ticks);

    SDL_SetRenderDrawColor(renderer,current_theme.background.r, current_theme.background.b, current_theme.background.b,255);  // Fond noir
    SDL_RenderClear(renderer);

    // Dessiner le titre
    afficher_message(renderer, "Menu Principal (PROJET C)", 250, 40);

    const char *options[] = {"DEMARRER", "CHARGER PARTIE", "HISTORIQUE DES JOUEURS" ,"OPTIONS","MUSIQUE","QUITTER"};
    int x = 300;
    int y_start = 160;

    for (int i = 0; i < 6; i++) {
        SDL_Color color = (i == selected_option) ? current_theme.highlight : current_theme.text;
        afficher_message_with_color(renderer, options[i], x, y_start + i * 70, color);

        if (i == selected_option) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect cursor_rect = {x - 50, y_start + i * 70, 30, 30};
            SDL_RenderFillRect(renderer, &cursor_rect);  // Dessiner un curseur à gauche de l'option
        }
    }

    const char *descriptions[] = {
            "Commencez une nouvelle partie.",
            "Chargez une partie sauvegardee.",
            "Consultez l'historique des joueurs.",
            "Option du jeu",
            "Musique de la partie",
            "Quittez le jeu."
    };

    afficher_message(renderer, descriptions[selected_option], 100, 550);


    SDL_RenderPresent(renderer);
}


void handle_menu_input(SDL_Event *event, int *selected_option, SDL_Renderer *renderer) {
    stop_background_music();
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (*selected_option > 0) {
                    Mix_PlayChannel(-1, select_sound, 0);
                    (*selected_option)--;
                }
                break;
            case SDLK_DOWN:
                if (*selected_option < 5) {
                    Mix_PlayChannel(-1, select_sound, 0);  // Jouer le son de sélection
                    (*selected_option)++;
                }
                break;
            case SDLK_RETURN:
                switch (*selected_option) {
                    case 0:  // Démarrer une nouvelle partie
                        printf("Démarrer une nouvelle partie\n");
                        current_menu = MENU_DIFFICULTE;
                        Mix_PlayChannel(-1, accept_sound, 0);
                        break;
                    case 1:  // Charger une partie
                        printf("Charger une partie (non implémenté)\n");
                        current_menu = CHARGER;
                        Mix_PlayChannel(-1, accept_sound, 0);
                        break;
                    case 2:  // Historique des joueurs
                        printf("Afficher l'historique des joueurs (non implémenté)\n");
                        current_menu = HISTORIQUE;
                        Mix_PlayChannel(-1, accept_sound, 0);
                        break;
                    case 3:
                        current_menu = OPTIONS;
                        Mix_PlayChannel(-1, accept_sound, 0);
                        break;
                    case 4:  // Musique
                        current_menu = MENU_MUSIQUE;
                        Mix_PlayChannel(-1, accept_sound, 0);
                        break;

                    case 5:  // Quitter
                        printf("Quitter le jeu\n");
                        SDL_Quit();
                        exit(0);
                        break;
                }
                break;
            case SDLK_ESCAPE:
                current_menu = MENU_PRINCIPAL;
                break;
        }
    }
    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);

        // Déterminez quelle option a été cliquée en fonction des coordonnées
        if (y >= 190 && y < 260) {
            *selected_option = 0;  // Option "DEMARRER"
        } else if (y >= 260 && y < 330) {
            *selected_option = 1;  // Option "CHARGER PARTIE"
        } // Continuez pour les autres options

        if (event->button.button == SDL_BUTTON_LEFT) {
            // Validez l'option sélectionnée
        }
    }
}

void handle_game_input(SDL_Event *event) {
    if (event->type == SDL_QUIT) {
        running = 0;
    } else if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_f) {
            save_game("save_game.txt");
            printf("Partie sauvegardée\n");
        } else if(event->key.keysym.sym == SDLK_c){
            current_menu = CHARGER;
        } else{
            handle_key(event->key.keysym.sym);
        }
    } else if (event->type == SDL_MOUSEBUTTONDOWN) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        if (x >= 10 && x <= 100 && y >= 10 && y <= 40) {
            current_menu = MENU_PRINCIPAL;
        }
    }
}


void afficherNumero(SDL_Renderer *renderer, TTF_Font *font, int numero, int x, int y, int largeur, int hauteur) { // permet de mettre les numéros sur les cases
    // Convertir le numéro en texte
    char texte[5];
    snprintf(texte, sizeof(texte), "%d", numero);

    // Définir la couleur du texte
    SDL_Color couleurTexte = {255, 255, 255};  // Blanc

    // Créer une surface pour le texte
    SDL_Surface *surfaceTexte = TTF_RenderText_Solid(font, texte, couleurTexte);
    if (surfaceTexte == NULL) {
        printf("Erreur de création du texte : %s\n", TTF_GetError());
        return;
    }
    SDL_Texture *textureTexte = SDL_CreateTextureFromSurface(renderer, surfaceTexte);
    SDL_FreeSurface(surfaceTexte);  // Libérer la surface après avoir créé la texture

    if (textureTexte == NULL) {
        printf("Erreur de création de la texture : %s\n", SDL_GetError());
        return;
    }
    SDL_Rect rectTexte = {x + largeur / 4, y + hauteur / 4, largeur / 2, hauteur / 2};
    SDL_RenderCopy(renderer, textureTexte, NULL, &rectTexte);
    SDL_DestroyTexture(textureTexte);
}


void handle_victory(SDL_Renderer *renderer, TTF_Font *font) {
    // Vérifie la condition de victoire
    if (check_victory(&game) && !has_victory_displayed) {
        stop_timer = true;  // Arrête le chronomètre
        game.time_elapsed_off = game.time_elapsed;  // Sauvegarde le temps écoulé
        has_victory_displayed = true;  // Marque la victoire comme affichée

        transition_fade(renderer);

        // Affiche la fenêtre de victoire
        show_victory_window(renderer, font);

        // Retour au menu principal après interaction
        current_menu = MENU_PRINCIPAL;
    }
}



void update_timer(Uint32 start_time) { // chronomètre
    if (!stop_timer) {  // Mettre à jour uniquement si le jeu n'est pas terminé
        Uint32 current_time = SDL_GetTicks();
        game.time_elapsed = (current_time - start_time) / 1000;  // Temps en secondes
    }
}




void game_loop(SDL_Renderer *renderer, TTF_Font *font, MenuState previous_menu) { // boucle principal du jeu
    SDL_Event e;
    bool running = true;
    Uint32 start_time = SDL_GetTicks();
    stop_timer = false;
    play_selected_music();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false; // Quitte la boucle
                current_menu = MENU_PRINCIPAL; // Retourne au menu principal en cas de fermeture
            }


            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode key = e.key.keysym.sym;

                if (key == SDLK_ESCAPE) {
                    current_menu = previous_menu; // Retour au menu précédent
                    running = false;
                } else {
                    handle_key(key);
                }
            }
        }

        if (!stop_timer) {
            update_timer(start_time);
        }

        // Vérifie la victoire
        handle_victory(renderer, font);

        // Rendu du jeu
        render_game(renderer, font);

    }
}

void add_move_to_history(const char *move) { // ajout d'un mouvements dans l'historique de mouvements
    // S'assurer que l'historique ne dépasse pas la taille maximale
    if (game.moves_count < MAX_HISTORY) {
        // Ajouter le mouvement à l'historique
        strncpy(game.moves_history[game.moves_count], move, sizeof(game.moves_history[game.moves_count]) - 1);
        game.moves_history[game.moves_count][sizeof(game.moves_history[game.moves_count]) - 1] = '\0'; // Assurer que la chaîne est terminée
        game.moves_count++; // Incrémenter le compteur des mouvements
    } else {
        // Si l'historique est plein, décaler tous les éléments vers le haut pour faire de la place
        for (int i = 1; i < MAX_HISTORY; i++) {
            strncpy(game.moves_history[i - 1], game.moves_history[i], sizeof(game.moves_history[i - 1]) - 1);
        }
        // Ajouter le nouveau mouvement à la fin de l'historique
        strncpy(game.moves_history[MAX_HISTORY - 1], move, sizeof(game.moves_history[MAX_HISTORY - 1]) - 1);
        game.moves_history[MAX_HISTORY - 1][sizeof(game.moves_history[MAX_HISTORY - 1]) - 1] = '\0'; // Terminer la chaîne
    }
}

void reset_game_history() { // effacer l'historique
    game.moves_count = 0;  // Réinitialiser le compteur de mouvements
    memset(game.moves_history, 0, sizeof(game.moves_history));  // Vider l'historique des mouvements
}

void save_game(const char *filename) { //sauvegarde d'un niveau
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Erreur lors de l'ouverture du fichier de sauvegarde.\n");
        return;
    }

    fprintf(file, "%d\n", game.difficulty);
    fprintf(file, "%d %d\n", game.empty_x, game.empty_y);
    fprintf(file, "%d\n", game.moves_count);
    fprintf(file, "%d\n", game.time_elapsed);

    int grid_size = (game.difficulty == EASY) ? GRID_SIZE_EASY :
                    (game.difficulty == NORMAL) ? GRID_SIZE_NORMAL : GRID_SIZE_HARD;

    for (int i = 0; i < grid_size; i++) {
        for (int j = 0; j < grid_size; j++) {
            fprintf(file, "%d ", game.grid[i][j]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printf("Partie sauvegardée dans %s.\n", filename);

    snprintf(temp_message.message, sizeof(temp_message.message), "Sauvegarde reussie !");
    temp_message.display_time = SDL_GetTicks() + 2000; // 2 secondes à partir de maintenan
}


void load_game(const char *filename) { // Charger une partie selon le niveau
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur lors de l'ouverture du fichier de sauvegarde.\n");
        return;
    }

    fscanf(file, "%d", &game.difficulty);
    fscanf(file, "%d %d", &game.empty_x, &game.empty_y);
    fscanf(file, "%d", &game.moves_count);
    fscanf(file, "%d", &game.time_elapsed);

    int grid_size = (game.difficulty == EASY) ? GRID_SIZE_EASY :
                    (game.difficulty == NORMAL) ? GRID_SIZE_NORMAL : GRID_SIZE_HARD;

    // Libérer la mémoire existante avant de recharger une grille
    if(game.grid) {
        for (int i = 0; i < grid_size; i++) {
            free(game.grid[i]);
        }
        free(game.grid);
    }

    game.grid = malloc(grid_size * sizeof(int *));
    for (int i = 0; i < grid_size; i++) {
        game.grid[i] = malloc(grid_size * sizeof(int));
        for (int j = 0; j < grid_size; j++) {
            fscanf(file, "%d", &game.grid[i][j]);
        }
    }

    fclose(file);
    printf("Partie chargée depuis %s.\n", filename);
}


void show_load_menu(SDL_Renderer *renderer, int selected_option) { // afficher la fenêtre des sauvegardes selon les niveaux
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    afficher_message(renderer, "Menu de Chargement", 300, 50);

    const char *options[] = {"Sauvegarde Facile", "Sauvegarde Normale", "Sauvegarde Difficile", "Retour"};

    for (int i = 0; i < 4; i++) {
        SDL_Color color = (i == selected_option)  ? (SDL_Color){255, 0, 0} : (SDL_Color){255, 255, 255};
        afficher_message(renderer, options[i], 300, 150 + i * 50);
    }
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect cursor_rect = {260, 150 + selected_option * 50, 20, 20};
    SDL_RenderFillRect(renderer, &cursor_rect);

    SDL_RenderPresent(renderer);
}

void handle_load_menu_input(SDL_Event *event, int *selected_option) { // Manipulation du menu de chargement
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (*selected_option > 0) {   Mix_PlayChannel(-1, select_sound, 0); (*selected_option)--; }
                break;
            case SDLK_DOWN:
                if (*selected_option < 3) {   Mix_PlayChannel(-1, select_sound, 0);(*selected_option)++;}
                break;
            case SDLK_RETURN: {
                const char *filename = NULL;

                switch (*selected_option) {
                    case 0: filename = "save_game_easy.txt"; break;
                    case 1: filename = "save_game_normal.txt"; break;
                    case 2: filename = "save_game_hard.txt"; break;
                    case 3: current_menu = MENU_PRINCIPAL; Mix_PlayChannel(-1, back_sound, 0); return;
                }

                if (filename) {
                    FILE *file = fopen(filename, "r");
                    if (file) {
                        fclose(file);
                        load_game(filename);
                        current_menu = JEU;  // Charger la partie et passer au jeu
                    } else {
                        snprintf(temp_message.message, sizeof(temp_message.message), "Aucune sauvegarde trouvee pour ce niveau !");
                        temp_message.display_time = SDL_GetTicks() + 2000;  // Afficher le message 2 secondes
                    }
                }
                break;
            }
            case SDLK_ESCAPE:
                current_menu = MENU_PRINCIPAL;
                break;
        }
    }
}
void render_background(SDL_Renderer *renderer,Uint32 ticks){
    int red = (ticks / 10) % 255;
    int green = (ticks/15) % 255;
    int blue = (ticks/20) % 255;
    SDL_SetRenderDrawColor(renderer,red,green,blue,255);
    SDL_RenderClear(renderer);
}
void show_logo(SDL_Renderer *renderer) { // affiche le logo 2
    SDL_Surface *logo = SDL_LoadBMP("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/image/ESIEA.bmp");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, logo);

    SDL_Rect rect = {180, 180, 427, 300};  // Taille et position du logo
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_RenderPresent(renderer);

    SDL_Delay(3000);  // Afficher pendant 2 secondes
    SDL_FreeSurface(logo);
    SDL_DestroyTexture(texture);
    SDL_RenderPresent(renderer);
    SDL_RenderClear(renderer);
    transition_fade(renderer);

}
void show_logo2(SDL_Renderer *renderer) { // affiche le logo 1
    SDL_Surface *logo = SDL_LoadBMP("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/image/logo.bmp");
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, logo);

    SDL_Rect rect = {90, 110, 600, 500};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_RenderPresent(renderer);

    SDL_Delay(2000);  // Afficher pendant 2 secondes
    SDL_FreeSurface(logo);
    SDL_DestroyTexture(texture);
    transition_fade(renderer);
}

void show_option_menu(SDL_Renderer*renderer, int selected_option){ // affichier les options theme
    SDL_SetRenderDrawColor(renderer,current_theme.background.r,current_theme.background.g,current_theme.background.b,255);
    SDL_RenderClear(renderer);

    afficher_message(renderer, "OPTION DU JEU", 300, 50);

    const char *options[] = {"THEME CLAIR","THEME SOMBRE", "RETOUR"};
    int x = 300;
    int y_start = 190;

    for(int i = 0; i < 3; i++){
        SDL_Color color = (i == selected_option)? current_theme.highlight : current_theme.text;
        afficher_message_with_color(renderer,options[i],x,y_start + i * 70,color);
        if (i == selected_option) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_Rect cursor_rect = {x - 50, y_start + i * 70, 30, 30};
            SDL_RenderFillRect(renderer, &cursor_rect);
        }
    }
    SDL_RenderPresent(renderer);
}

void show_victory_window(SDL_Renderer *renderer, TTF_Font *font) { // affichier l'écran de victoir
    // Dimensions de la fenêtre de victoire
    SDL_Rect victory_window = {200, 150, 450, 300};  // Position et taille de la fenêtre
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // Fond sombre
    SDL_RenderFillRect(renderer, &victory_window);

    // Ajouter une bordure blanche à la fenêtre
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &victory_window);

    SDL_Color white = {255, 255, 255};  // Couleur blanche
    SDL_Color blue = {100, 150, 255};  // Couleur pour le texte
    afficher_message_with_color(renderer, "VICTOIRE !", 300, 170, blue);
    afficher_message_with_color(renderer, "Entrez votre nom (4 lettres max) :", 230, 220, white);

    // Initialisation du nom de joueur
    char player_name[5] = "";  // Nom du joueur
    int name_length = 0;

    SDL_RenderPresent(renderer);

    SDL_Event e;
    bool waiting = true;

    SDL_StartTextInput();

    // Boucle d'attente pour la saisie
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                exit(0);  // Quitter le jeu
            }

            // Gestion de la saisie du nom
            if (e.type == SDL_TEXTINPUT && name_length < 4) {
                player_name[name_length++] = e.text.text[0];
                player_name[name_length] = '\0';

                // Rafraîchir la fenêtre avec le texte mis à jour
                SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // Rafraîchir le fond
                SDL_RenderFillRect(renderer, &victory_window);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Bordure blanche
                SDL_RenderDrawRect(renderer, &victory_window);

                afficher_message_with_color(renderer, "VICTOIRE !", 300, 170, blue);
                afficher_message_with_color(renderer, "Entrez votre nom (4 lettres max) :", 230, 220, white);
                afficher_message_with_color(renderer, player_name, 300, 270, white);

                SDL_RenderPresent(renderer);
            }

            // Suppression de caractères avec Backspace
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_BACKSPACE && name_length > 0) {
                player_name[--name_length] = '\0';

                // Rafraîchir la fenêtre avec le texte mis à jour
                SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
                SDL_RenderFillRect(renderer, &victory_window);
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Bordure blanche
                SDL_RenderDrawRect(renderer, &victory_window);

                afficher_message_with_color(renderer, "VICTOIRE !", 300, 170, blue);
                afficher_message_with_color(renderer, "Entrez votre nom (4 lettres max) :", 230, 220, white);
                afficher_message_with_color(renderer, player_name, 300, 270, white);

                SDL_RenderPresent(renderer);
            }

            // Retour automatique au menu principal après la saisie du nom
            if (name_length == 4) {
                save_player_to_history(player_name);  // Sauvegarde le nom du joueur
                waiting = false;
                current_menu = MENU_PRINCIPAL;  // Retour au menu principal
            }
        }
    }

    SDL_StopTextInput();
}

void save_player_to_history(const char *default_name) { // sauvegarder l'historique des joueurs après victoir
    char player_name[5] = "";  // Nom du joueur, maximum 4 caractères
    int name_length = 0;

    SDL_StartTextInput();  // Activer la saisie texte avec SDL
    SDL_Event e;
    bool waiting = true;

    // Attendre que l'utilisateur entre un nom valide
    while (waiting) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_TEXTINPUT) {
                if (name_length < 4) {
                    player_name[name_length++] = e.text.text[0];
                    player_name[name_length] = '\0';  // Fin de chaîne
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && name_length > 0) {
                    player_name[--name_length] = '\0';
                } else if (e.key.keysym.sym == SDLK_RETURN) {
                    waiting = false; // Valider la saisie et quitter la boucle
                }
            }
        }
    }
    SDL_StopTextInput();  // Désactiver la saisie texte avec SDL

    // Si aucun nom n'a été saisi, utiliser un nom par défaut
    if (name_length == 0) {
        strncpy(player_name, default_name, 4);
        player_name[4] = '\0';  // Fin de chaîne sécurisée
    }

    // Ajouter les informations à l'historique
    FILE *file = fopen("player.txt", "a");
    if (!file) {
        printf("Erreur : Impossible d'ouvrir le fichier d'historique.\n");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(file, "%s, %02d:%02d, %s, %04d-%02d-%02d\n",
            player_name,
            game.time_elapsed_off / 60, game.time_elapsed_off % 60,
            game.difficulty == EASY ? "Facile" :
            game.difficulty == NORMAL ? "Normal" : "Difficile",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);

    fclose(file);

    // Transition to main menu
    current_menu = MENU_PRINCIPAL;
}



void show_player_history(SDL_Renderer *renderer, TTF_Font *font) { // Affichier l'historique des joueurs
    SDL_Rect history_window = {100, 100, 600, 400}; // Position et taille de la fenêtre
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderFillRect(renderer, &history_window); // Fond sombre pour la fenêtre

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &history_window); // Bordure blanche

    afficher_message(renderer, "Historique des joueurs", 120, 120);

    FILE *file = fopen("player.txt", "r");
    if (!file) {
        afficher_message(renderer, "Aucun historique disponible.", 150, 160);
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
        return;
    }

    // Chargement des lignes depuis le fichier
    char lines[MAX_HISTORY][256];
    int line_count = 0;

    while (fgets(lines[line_count], sizeof(lines[line_count]), file)) {
        line_count++;
    }
    fclose(file);

    int scroll_index = 0;
    int visible_lines = 10; // Nombre de lignes visibles
    SDL_Event e;
    bool viewing = true;

    while (viewing) {
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &history_window);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &history_window);

        afficher_message(renderer, "Historique des joueurs", 280, 120); // Titre centré

        // En-tête des colonnes
        afficher_message_with_color(renderer, "Nom", 120, 160, (SDL_Color){255, 255, 0});
        afficher_message_with_color(renderer, "Temps", 240, 160, (SDL_Color){255, 255, 0});
        afficher_message_with_color(renderer, "Niveau", 360, 160, (SDL_Color){255, 255, 0});
        afficher_message_with_color(renderer, "Date", 480, 160, (SDL_Color){255, 255, 0});

        // Affichage des lignes avec scroll
        int y_offset = 190;
        for (int i = 0; i < visible_lines; i++) {
            int line_index = scroll_index + i;
            if (line_index < line_count) {
                char name[5], time[10], level[10], date[20];
                sscanf(lines[line_index], "%4[^,], %[^,], %[^,], %[^\n]", name, time, level, date);

                afficher_message(renderer, name, 120, y_offset);
                afficher_message(renderer, time, 240, y_offset);
                afficher_message(renderer, level, 360, y_offset);
                afficher_message(renderer, date, 480, y_offset);

                y_offset += 30;
            }
        }

        // Instructions en bas
        afficher_message_with_color(renderer, "K: Haut | L: Bas | ESC: Retour", 430, 510, (SDL_Color){200, 200, 200});
        SDL_RenderPresent(renderer);

        // Gestion des événements
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                viewing = false;
                break;
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_k && scroll_index > 0) {
                    scroll_index--; // Scroll vers le haut
                } else if (e.key.keysym.sym == SDLK_l && scroll_index + visible_lines < line_count) {
                    scroll_index++; // Scroll vers le bas
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    current_menu = MENU_PRINCIPAL;
                    viewing = false; // Quitter l'historique pour le menu principal
                }
            }
        }
    }
}


void transition_fade(SDL_Renderer *renderer) {
    int steps = 50;  // Nombre d'étapes pour la transition
    Uint8 alpha;     // Opacité (de 0 à 255)

    for (int i = 0; i <= steps; i++) {
        alpha = (255 * i) / steps;  // Calcul de l'opacité

        // Définir la couleur de dessin avec l'opacité
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);

        // Dessiner un rectangle noir couvrant toute la fenêtre
        SDL_Rect full_screen = {0, 0, 800, 700};  // Dimensions de l'écran
        SDL_RenderFillRect(renderer, &full_screen);

        SDL_RenderPresent(renderer);  // Afficher le résultat
        SDL_Delay(15);  // Ajuster la vitesse de la transition
    }
}


void stop_background_music() {  // Arrête la musique
    if (Mix_PlayingMusic()) {
        Mix_HaltMusic();
    }
}

void load_musics() { // fonction pour charger la musique de fond
    const char *music_files[MAX_MUSICS] = {
            "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/background.mp3",
            "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/background2.mp3",
            "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/Frank_Saint.mp3",
    };

    for (int i = 0; i < MAX_MUSICS; i++) {
        background_musics[i] = Mix_LoadMUS(music_files[i]);
        if (!background_musics[i]) {
            printf("Erreur lors du chargement de la musique %s : %s\n", music_files[i], Mix_GetError());
        } else {
            printf("Musique chargée : %s\n", music_files[i]);
        }
    }
}



void show_music_menu(SDL_Renderer *renderer, int selected_option) { // affichier le menu de la musqiue
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    afficher_message(renderer, "SELECTIONNEZ UNE MUSIQUE", 250, 50);

    const char *options[MAX_MUSICS + 1] = {
            "Beat Brazilia ", "Werenoir La league - instru", "Franklin Saint",  "Retour"
    };

    for (int i = 0; i < MAX_MUSICS + 1; i++) {
        SDL_Color color = (i == selected_option) ? (SDL_Color){255, 0, 0} : (SDL_Color){255, 255, 255};
        afficher_message(renderer, options[i], 300, 150 + i * 50);
    }

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_Rect cursor_rect = {260, 150 + selected_option * 50, 20, 20};
    SDL_RenderFillRect(renderer, &cursor_rect);

    SDL_RenderPresent(renderer);
}

void handle_music_menu_input(SDL_Event *event, int *selected_option) { // selectionner les musiques
    if (event->type == SDL_KEYDOWN) {
        switch (event->key.keysym.sym) {
            case SDLK_UP:
                if (*selected_option > 0) {
                    Mix_PlayChannel(-1, select_sound, 0);
                    (*selected_option)--;
                }
                break;
            case SDLK_DOWN:
                if (*selected_option < MAX_MUSICS) {
                    Mix_PlayChannel(-1, select_sound, 0);
                    (*selected_option)++;
                }
                break;
            case SDLK_RETURN:
                if (*selected_option == MAX_MUSICS) {  // Retour au menu principal
                    current_menu = MENU_PRINCIPAL;
                    Mix_PlayChannel(-1, back_sound, 0);
                } else {
                    current_music_index = *selected_option;
                    play_selected_music();
                    current_menu = MENU_PRINCIPAL;  // Retour au menu principal
                }
                break;
            case SDLK_ESCAPE:
                current_menu = MENU_PRINCIPAL;  // Retour au menu principal
                break;
        }
    }
}

void play_selected_music() { // lancer la musqiue du jeu
    if (Mix_PlayingMusic()) {
        if (Mix_GetMusicType(NULL) == MUS_MP3 && background_musics[current_music_index]) {
            return;
        }
        Mix_HaltMusic();  // Arrête la musique actuelle
    }

    if (background_musics[current_music_index]) {
        Mix_PlayMusic(background_musics[current_music_index], -1);  // Joue la musique sélectionnée en boucle
    } else {
        printf("Erreur : aucune musique chargée à l'indice %d\n", current_music_index);
    }
}



