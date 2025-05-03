cmake_minimum_required(VERSION 3.29)
project(Projet_C C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules)

# Paths to the SDL2 and related libraries
set(SDL2_PATH "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Objet/Taquin - version graphique/SDL/SDL2-2.30.9/x86_64-w64-mingw32")
set(SDL2_TTF_PATH "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Objet/Taquin - version graphique/SDL/SDL2_ttf-2.22.0/x86_64-w64-mingw32")
set(SDL2_MIXER_PATH "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Objet/Taquin - version graphique/SDL/SDL2_mixer-2.8.0/x86_64-w64-mingw32")
set(SDL2_IMAGE_PATH "C:/Users/alain/OneDrive/Documents/X3(ESIEA)/Objet/Taquin - version graphique/SDL/SDL2_image-2.8.2/x86_64-w64-mingw32")

# Locate SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

# Add SDL2_ttf
include_directories(${SDL2_TTF_PATH}/include)
link_directories(${SDL2_TTF_PATH}/lib)

# Add SDL2_mixer
include_directories(${SDL2_MIXER_PATH}/include)
link_directories(${SDL2_MIXER_PATH}/lib)

# Add SDL2_image
include_directories(${SDL2_IMAGE_PATH}/include)
link_directories(${SDL2_IMAGE_PATH}/lib)

# Add the executable
add_executable(Projet_C main.c taquin.h taquin.c graphics.c events.c)

# Link libraries
target_link_libraries(Projet_C ${SDL2_LIBRARY} SDL2_ttf SDL2_mixer SDL2_image)
