# Définir les chemins de recherche pour SDL2_ttf
set(SDL2_TTF_SEARCH_PATHS
        ~/Library/Frameworks
        /Library/Frameworks
        /usr/local
        /usr
        /sw # Fink
        /opt/local # DarwinPorts
        /opt/csw # Blastwave
        /opt
        ${SDL2_TTF_PATH}
)

# Recherche du répertoire d'inclusion de SDL2_ttf
find_path(SDL2_TTF_INCLUDE_DIR SDL_ttf.h
        HINTS
        $ENV{SDL2_TTFDIR}
        PATH_SUFFIXES include/SDL2_ttf include
        PATHS ${SDL2_TTF_SEARCH_PATHS}
)

# Vérification de la plateforme pour ajuster les suffixes de chemin pour les bibliothèques
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PATH_SUFFIXES lib64 lib/x64 lib)
else()
    set(PATH_SUFFIXES lib/x86 lib)
endif()

# Recherche de la bibliothèque SDL2_ttf
find_library(SDL2_TTF_LIBRARY_TEMP
        NAMES SDL2_ttf
        HINTS
        $ENV{SDL2_TTFDIR}
        PATH_SUFFIXES ${PATH_SUFFIXES}
        PATHS ${SDL2_TTF_SEARCH_PATHS}
)

# Si la bibliothèque SDL2_ttf a été trouvée, on procède à l'intégration
if(SDL2_TTF_LIBRARY_TEMP)
    # Pour macOS, SDL2_ttf utilise Cocoa en tant que backend
    if(APPLE)
        set(SDL2_TTF_LIBRARY_TEMP ${SDL2_TTF_LIBRARY_TEMP} "-framework Cocoa")
    endif()

    # Pour MinGW, ajout de bibliothèques spécifiques
    if(MINGW)
        set(SDL2_TTF_LIBRARY_TEMP ${MINGW32_LIBRARY} ${SDL2_TTF_LIBRARY_TEMP})
    endif()

    # Définir la variable finale pour SDL2_ttf
    set(SDL2_TTF_LIBRARY ${SDL2_TTF_LIBRARY_TEMP} CACHE STRING "Where the SDL2_ttf Library can be found")

    # Marquer le chemin temporaire comme interne pour qu'il ne soit pas visible dans l'interface CMake
    set(SDL2_TTF_LIBRARY_TEMP "${SDL2_TTF_LIBRARY_TEMP}" CACHE INTERNAL "")
endif(SDL2_TTF_LIBRARY_TEMP)

# Inclure le module CMake pour gérer la recherche de paquets
include(FindPackageHandleStandardArgs)

# Gestion standard des arguments de paquet
find_package_handle_standard_args(SDL2_ttf REQUIRED_VARS SDL2_TTF_LIBRARY SDL2_TTF_INCLUDE_DIR)
