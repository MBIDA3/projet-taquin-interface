#include <SDL.h>
#include <SDL_ttf.h>
#include "taquin.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL_mixer.h>
#include <SDL_image.h>

TTF_Font *font;
SDL_Renderer *renderer = NULL;  // Define the global renderer
MenuState current_menu = MENU_PRINCIPAL;  // Initialisation du menu principal


int main(int argc, char *argv[]) {
    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erreur SDL_Init : %s\n", SDL_GetError());
        return -1;
    }
    // Intitalisation des sons SDL_MIXER
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("Erreur lors de l'initialisation de SDL_mixer : %s\n", Mix_GetError());
        return -1;

    }

    select_sound = Mix_LoadWAV("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/Select.wav");
    if (!select_sound) {
        printf("Erreur lors du chargement du son de sélection : %s\n", Mix_GetError());
    }

    background_music = Mix_LoadMUS("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/background.mp3");
    if (!background_music) {
        printf("Erreur lors du chargement de la musique de fond : %s\n", Mix_GetError());
    }
    accept_sound = Mix_LoadWAV("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/accepet.wav");
    if (!accept_sound){
        printf("Erreur lors du chargement de la musique : %s\n", Mix_GetError());
    }
    back_sound = Mix_LoadWAV("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/back.wav");
    if (!back_sound){
        printf("Erreur lors du chargement de la musique : %s\n", Mix_GetError());
    }

    move_sound = Mix_LoadWAV("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Son/move.wav");
    if (!move_sound) {
        printf("Erreur lors du chargement du son de déplacement : %s\n", Mix_GetError());
    }

    // Initialisation de TTF
    if (TTF_Init() < 0) {
        printf("Erreur TTF_Init : %s\n", TTF_GetError());
        SDL_Quit();
        return -1;
    }

    // Charger la police
    font = TTF_OpenFont("C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Projet C/Font/POSTERABLE.ttf", 24);
    if (font == NULL) {
        printf("Erreur de chargement de la police : %s\n", TTF_GetError());
        return -1;
    }

    // Création de la fenêtre
    SDL_Window *window = SDL_CreateWindow("Taquin", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 700, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);  // Use the global renderer

    if (!window) {
        printf("Erreur de création de la fenêtre : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }

    if (!renderer) {
        printf("Erreur de création du renderer : %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_CloseFont(font);
        TTF_Quit();
        SDL_Quit();
        return -1;
    }
    // Initialisation du jeu avec une difficulté par défaut
    init_game(NORMAL);

    int selected_option = 0;
    SDL_Event e;
    int running = 1;
    load_musics();

    show_logo(renderer);
    show_logo2(renderer);

    current_music_index = 0;  // Indice de la première musique
    play_selected_music();
    while (running) {  // Boucle de jeu
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }

            // Gestion des entrées utilisateur en fonction du menu actuel
            switch (current_menu) {
                case MENU_PRINCIPAL:
                    handle_menu_input(&e, &selected_option, renderer); // Modification : ajout du renderer comme argument
                    break;
                case MENU_DIFFICULTE:
                    handle_difficulty_input(&e, &selected_option);// Gérer le menu de difficulté
                    break;
                case HISTORIQUE:
                    break;
                case CHARGER:
                    handle_load_menu_input(&e, &selected_option);
                    break;
                case OPTIONS:
                    handle_options_menu_input(&e,&selected_option);
                    break;
                case MENU_MUSIQUE:
                    handle_music_menu_input(&e,&selected_option);
                    break;
                case JEU:
                    // Gérer les entrées du jeu
                    game_loop(renderer, font, MENU_PRINCIPAL);
                    handle_game_input(&e);
                    break;
            }
        }

        // Affichage du menu ou du jeu en fonction du menu sélectionné
        switch (current_menu) {
            case MENU_PRINCIPAL:
                show_main_menu(renderer, selected_option);
                break;
            case MENU_DIFFICULTE:
                show_difficulty_menu(renderer, selected_option);
                break;
            case CHARGER:
                show_load_menu(renderer, selected_option);
                break;
            case HISTORIQUE:
                show_player_history(renderer, font);
                break;
            case OPTIONS:
                show_option_menu(renderer,selected_option);
                break;
            case MENU_MUSIQUE:
                show_music_menu(renderer,selected_option);
                break;
            case JEU:
                render_game(renderer, font);
                break;
        }
    }

    // Libérer les ressources avant de quitter
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    TTF_Quit();

    return 0;
}

