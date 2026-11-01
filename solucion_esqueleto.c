// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// CONSTANTES DEL JUEGO
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ESTRUCTURAS DE DATOS

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de proximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de particulas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// VARIABLES GLOBALES 
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// EJERCICIO 1: STRINGS 

char* strDuplicate(char* src) {
    // TODO: Contar los caracteres de src, reservar con malloc,
    //       copiar todo y devolver el puntero nuevo.
    // cant. caracteres src 
    int len= 0;
    while(src[len]!= '\0'){
        len++; 
    }
    // pido memoria. len + 1 (por el terminador)
    char* copia = malloc((len + 1) * sizeof(char));
    if (copia == NULL) return NULL;
    for (int i=0; i<len; i++){
        copia[i]= src[i];
    }
    copia[len]= '\0';  // agrego terminador
    return copia;
}

// Devuelve la cantidad de caracteres de st, sin contar el '\0'
int strLength(char* st) {
    int len = 0;
    while (st[len] != '\0') {
        len++;
    }
    return len;
}
int strCompare(char* s1, char* s2) {
    // TODO: Devolver 0 si son iguales, 1 si s1 < s2, -1 si s1 > s2.
    //        Es la convencion INVERSA a strcmp.
    // '\0' vale 0 
    int i=0;
    while (s1[i] != '\0' && s1[i]== s2[i]){
        i ++; 
    }
    if (s1[i]== s2[i]){
        return 0; 
    } else if(s1[i] < s2[i]){
        return 1; 
    }else{
        return -1; 
    }
}

char* strConcatenate(char* src1, char* src2) {
    // TODO: Reservar un string nuevo con src1 seguido de src2, y liberar src1 y src2.
    //       Precondicion: src1 y src2 vienen de malloc y son punteros distintos.
    int len1 = strLength(src1);
    int len2 = strLength(src2);
    char* rta = malloc((len1 + len2 + 1)* sizeof(char));
    if (rta == NULL) return NULL;   // no libero src1 ni src2 para no perder los datos
    for(int i = 0; i < len1; i++){
        rta[i] = src1[i];
    }
    for(int i = 0; i < len2; i++){
        rta[i + len1] = src2[i];
    }
    rta[len1 + len2] = '\0'; 

    free(src1);
    free(src2);
    return rta;
}

// EJERCICIO 2: FUNCIONES A IMPLEMENTAR 
void gameBoardDelete(GameBoard* board);
GameBoard* gameBoardNew() {
    // Reservar memoria para el GameBoard.
    GameBoard* board = malloc(sizeof(GameBoard));
    if (board == NULL) return NULL;

    // inicializar los campos 
        //punteros en null - Inicializar queue_head, queue_tail y first_particle en NULL.
    board->first_row = NULL;           
    board->queue_head = NULL;          
    board->queue_tail = NULL;
    board->first_particle = NULL; 
        //  score y tick_timer en 0    
    board->score = 0;
    board->tick_timer = 0;
        // en 0 para que no quede basura
    board->current_piece.col = 0;     
    board->current_piece.row = 0;
    board->current_piece.type = 0;
    board->current_piece.rotation = 0;
        // sin pieza activa
    board->current_piece.active = 0;  

    // Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    // Cada fila nueva se agrega al principio de la lista.
    for (int r = 0; r < GRID_ROWS; r++) {
        RowNode* nueva = malloc(sizeof(RowNode));
        if (nueva == NULL) {
            gameBoardDelete(board);   // libero las filas ya creadas y el tablero
            return NULL;
        }
        for (int c = 0; c < GRID_COLS; c++) {
            nueva->cells[c] = 0;      // fila vacia
        }
        nueva->next = board->first_row;   // enlazo ADENTRO del for, una vez por fila
        board->first_row = nueva;         // El puntero first_row apunta al ultimo nodo creado
    }

    return board;
}

void gameBoardDelete(GameBoard* board) {
    // Debe tolerar board == NULL.
    if (board == NULL) return;

    // Recorrer y liberar todos los RowNode con free().
    // Libero las filas. Guardo el siguiente antes del free, porque despues de liberar un nodo ya no puedo leer su next
    RowNode* fila = board->first_row;
    while (fila != NULL) {
        RowNode* sig = fila->next;
        free(fila);
        fila = sig;
    }

    // Recorrer y liberar todos los PieceQueueNode.
    PieceQueueNode* pieza = board->queue_head;
    while (pieza != NULL) {
        PieceQueueNode* sig = pieza->next;
        free(pieza);
        pieza = sig;
    }

    // Recorrer y liberar todos los ParticleNode.
    ParticleNode* part = board->first_particle;
    while (part != NULL) {
        ParticleNode* sig = part->next;
        free(part);
        part = sig;
    }
    // Liberar la estructura GameBoard.
    free(board);
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    if (board == NULL) return;

    // Crear un ParticleNode con memoria dinamica.
    ParticleNode* nueva = malloc(sizeof(ParticleNode));
    if (nueva == NULL) return;
    // Inicializar x, y, velocidades aleatorias y un life_timer.
    nueva->x = x;
    nueva->y = y;
    nueva->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    nueva->vel_y = ((rand() % 50) / -10.0f) - 2.0f;
    nueva->color = color;
    nueva->life_timer = 30 + (rand() % 30);

    // Agregarlo al principio de la lista enlazada simple de particulas.
    nueva->next = board->first_particle;
    board->first_particle = nueva;
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    if (board == NULL) return 0;
    // contador de filas que se borran 
    int res= 0; 
    // necesitamos para gameBoardAddParticle()
    int r= 0; //posicion de la fila actual en pantalla 
    // punteros para recorrer las filas 
    RowNode* prev = NULL; 
    RowNode* fila = board->first_row; 

    // Recorrer la lista de RowNode.
    while (fila != NULL) {
        // fila llena empieza como True 
        int FilaLlena =1; 
        // por cada fila recorro todos sus columnas si encuentra una fila con valor 0, 
        // cambia llena a false y termina (nos alcanza con q una bloque sea 0)
        // TODO: Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
        for (int c = 0; c < GRID_COLS; c++) {
            if (fila->cells[c] == 0) {
                FilaLlena = 0;
                break;
            }
        }
        RowNode* sig= fila-> next;   // guardamos para poder hacer el free y no perder el puntero 

        if (FilaLlena){
            // efecto rompe fila 
            // TODO: Por cada celda destruida, invocar a gameBoardAddParticle().
            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(board,
                                     OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2,
                                     OFFSET_Y + r * CELL_SIZE + CELL_SIZE / 2,
                                     fila->cells[c]);
            }
            // desconecto esa fila 
            // si es el primero de la fila 
            if (prev == NULL){
                board-> first_row = sig; 
            
            // si esta en otra posicion de la lista 
            }else{
                prev->next =sig; 
            }
            free(fila); 
            res++; 
        }else {
            prev = fila; 
        }
        fila=sig;
        r++; 
    }

    // TODO: Crear un nuevo RowNode vacio (lleno de ceros).
    // TODO: Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
    // agrego todas las nuevas filas juntas 
    for (int i = 0; i < res; i++) {
        RowNode* nueva = malloc(sizeof(RowNode));
        if (nueva == NULL) return res;
        for (int c = 0; c < GRID_COLS; c++) {
            nueva->cells[c] = 0;
        }
        nueva->next = board->first_row;
        board->first_row = nueva;
    }

    return res;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    if (board == NULL) return;
    // TODO: Crear un PieceQueueNode con el 'type'.
    PieceQueueNode* nueva = malloc(sizeof(PieceQueueNode));
    if (nueva == NULL) return;
    // en el campo type del nodo nuevo guarda el numero que entra como parametro(int type)
    nueva->type = type;
    nueva->next = NULL; //es el ultimo

    // TODO: Insertarlo al final de la cola (queue_tail) y actualizar punteros.
    // Cola vacia, el nuevo es a la vez el primero y el ultimo
    if (board->queue_head == NULL) {
        board->queue_head = nueva;
        board->queue_tail = nueva;
    } else {
        
        
        board->queue_tail->next = nueva;    // lo engancho despues del ultimo
        board->queue_tail = nueva;         // y paso a considerarlo el nuevo ultimo
    }
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    // TODO: Si la cola esta vacia, retornar 0.
    if (board == NULL) return 0;
    if (board -> queue_head == NULL) return 0; 
    
    // TODO: Extraer el primer nodo (queue_head).
    PieceQueueNode* primero = board->queue_head;

    // Guardar el type en la variable out_type  
    *out_type = primero->type;    
    // actualizar el head
    board->queue_head = primero->next;
    // si saque el ultimo entonces tail tiene que ser null sino sigue teniendo el mismo valor
    if (board->queue_head == NULL){
        board->queue_tail = NULL;
    }
    
    //liberar el nodo
    free(primero);
    return 1;
}

// LOGICA DE DIBUJADO Y COLISIONES (No modificar)

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// TESTS 

// Contadores globales: cuantos chequeos se hicieron y cuantos fallaron
int tests_total = 0;
int tests_fallidos = 0;

// Imprime [ok] si la condicion es verdadera (distinta de cero) y [FALLO] si es falsa
void chequear(int condicion, char* nombre) {
    tests_total++;
    if (condicion) {
        printf("  [ok]    %s\n", nombre);
    } else {
        tests_fallidos++;
        printf("  [FALLO] %s\n", nombre);
    }
}

// Cuenta cuantos RowNode tiene la lista del tablero
int contarFilas(GameBoard* board) {
    int cant = 0;
    RowNode* fila = board->first_row;
    while (fila != NULL) {
        cant++;
        fila = fila->next;
    }
    return cant;
}

// Cuenta cuantas particulas hay en la lista
int contarParticulas(GameBoard* board) {
    int cant = 0;
    ParticleNode* p = board->first_particle;
    while (p != NULL) {
        cant++;
        p = p->next;
    }
    return cant;
}

// Llena todas las celdas de la fila numero 'indice' con un valor distinto de 0
void llenarFila(GameBoard* board, int indice) {
    RowNode* fila = getRowAt(board, indice);
    for (int c = 0; c < GRID_COLS; c++) {
        fila->cells[c] = 1;
    }
}

// Devuelve 1 si todas las celdas de la fila valen 0, y 0 si no
int filaVacia(RowNode* fila) {
    for (int c = 0; c < GRID_COLS; c++) {
        if (fila->cells[c] != 0) return 0;
    }
    return 1;
}

void correrTests() {
    // strDuplicate 
    printf("strDuplicate\n");
    char* d1 = strDuplicate("");
    chequear(d1 != NULL && d1[0] == '\0', "string vacio");

    char* d2 = strDuplicate("x");
    chequear(d2 != NULL && d2[0] == 'x' && d2[1] == '\0', "un solo caracter");

    char* original = "Hola mundo";
    char* d3 = strDuplicate(original);
    chequear(d3 != NULL && strCompare(d3, original) == 0, "varios caracteres: mismo contenido");
    chequear(d3 != original, "varios caracteres: es una copia (otro puntero)");
    free(d1);
    free(d2);
    free(d3);

    // strCompare
    printf("== strCompare ==\n");
    chequear(strCompare("", "") == 0, "\"\" vs \"\" da 0");
    chequear(strCompare("a", "a") == 0, "\"a\" vs \"a\" da 0");
    chequear(strCompare("a", "b") == 1, "\"a\" vs \"b\" da 1 (s1 < s2)");
    chequear(strCompare("b", "a") == -1, "\"b\" vs \"a\" da -1 (s1 > s2)");
    chequear(strCompare("casa", "casas") == 1, "\"casa\" vs \"casas\" da 1 (prefijo es menor)");
    chequear(strCompare("casas", "casa") == -1, "\"casas\" vs \"casa\" da -1");

    // strConcatenate 
    // Los argumentos se crean con strDuplicate porque strConcatenate los libera.
    // Despues de la llamada NO se usan mas a1, b1, a2, b2. Solo se libera el resultado.
    printf("strConcatenate\n");
    char* a1 = strDuplicate("");
    char* b1 = strDuplicate("abc");
    char* r1 = strConcatenate(a1, b1);
    chequear(r1 != NULL && strCompare(r1, "abc") == 0, "\"\" + \"abc\" da \"abc\"");
    free(r1);

    char* a2 = strDuplicate("hola ");
    char* b2 = strDuplicate("mundo");
    char* r2 = strConcatenate(a2, b2);
    chequear(r2 != NULL && strCompare(r2, "hola mundo") == 0, "\"hola \" + \"mundo\" da \"hola mundo\"");
    chequear(r2 != NULL && strLength(r2) == 10, "el resultado tiene 10 caracteres");
    free(r2);

    // Cola de piezas
    printf("Cola de piezas\n");
    GameBoard* board = gameBoardNew();
    chequear(board != NULL, "gameBoardNew devuelve un tablero");
    if (board == NULL) {
        printf("No se puede seguir sin tablero.\n");
        return;
    }
    chequear(contarFilas(board) == GRID_ROWS, "el tablero nuevo tiene 20 filas");
    chequear(board->queue_head == NULL && board->queue_tail == NULL && board->first_particle == NULL,
             "cola y particulas empiezan vacias (NULL)");
    chequear(board->score == 0 && board->tick_timer == 0 && board->current_piece.active == 0,
             "score, tick_timer y pieza activa en 0");

    for (int i = 0; i < 5; i++) {
        gameBoardEnqueuePiece(board, i);
    }

    int salida = 99;
    int orden_ok = 1;
    for (int i = 0; i < 5; i++) {
        int resultado = gameBoardDequeuePiece(board, &salida);
        if (resultado != 1 || salida != i) orden_ok = 0;
    }
    chequear(orden_ok, "5 push y 5 pop: salen en el mismo orden (FIFO)");

    salida = 99;
    chequear(gameBoardDequeuePiece(board, &salida) == 0, "el 6to pop devuelve 0");
    chequear(salida == 99, "el 6to pop no modifica out_type");
    chequear(board->queue_head == NULL && board->queue_tail == NULL,
             "queue_head y queue_tail quedan en NULL");

    gameBoardEnqueuePiece(board, 6);
    chequear(board->queue_head != NULL && board->queue_head == board->queue_tail,
             "push despues de vaciar: head y tail apuntan al nuevo nodo");
    chequear(gameBoardDequeuePiece(board, &salida) == 1 && salida == 6,
             "pop despues del push devuelve 6");

    // Dejamos una pieza en la cola a proposito: gameBoardDelete la tiene que liberar
    gameBoardEnqueuePiece(board, 3);

    // Tablero: una linea 
    printf("Tablero: una linea\n");
    RowNode* primera_antes = board->first_row;
    llenarFila(board, 19);
    int borradas = gameBoardCheckAndClearLines(board);
    chequear(borradas == 1, "devuelve 1 linea borrada");
    chequear(board->first_row != primera_antes, "first_row es una fila nueva (otro nodo)");
    chequear(filaVacia(board->first_row), "la fila nueva esta vacia (todo ceros)");
    chequear(contarFilas(board) == GRID_ROWS, "la lista sigue teniendo 20 filas");
    chequear(contarParticulas(board) == GRID_COLS, "se crearon 10 particulas (una por celda)");

    // Tablero: dos lineas, incluyendo la primera de la lista
    printf("Tablero: dos lineas (filas 0 y 1)\n");
    llenarFila(board, 0);
    llenarFila(board, 1);
    borradas = gameBoardCheckAndClearLines(board);
    chequear(borradas == 2, "devuelve 2 lineas borradas");
    chequear(contarFilas(board) == GRID_ROWS, "la lista sigue teniendo 20 filas");
    chequear(filaVacia(getRowAt(board, 0)) && filaVacia(getRowAt(board, 1)),
             "las dos filas de arriba estan vacias");
    chequear(contarParticulas(board) == 3 * GRID_COLS, "30 particulas en total");

    // Tablero: filas separadas con una incompleta en el medio
    printf("Tablero: filas 5 y 7 llenas, fila 6 incompleta\n");
    llenarFila(board, 5);
    llenarFila(board, 7);
    getRowAt(board, 6)->cells[3] = 2;
    borradas = gameBoardCheckAndClearLines(board);
    chequear(borradas == 2, "borra solo las 2 filas llenas");
    chequear(contarFilas(board) == GRID_ROWS, "la lista sigue teniendo 20 filas");
    // las 2 filas nuevas quedaron arriba, asi que la incompleta bajo de la posicion 6 a la 7
    chequear(getRowAt(board, 7)->cells[3] == 2, "la fila incompleta sigue en la lista");

    // Liberar memoria
    printf("== gameBoardDelete ==\n");
    gameBoardDelete(NULL);
    chequear(1, "gameBoardDelete(NULL) no se rompe");
    gameBoardDelete(board);

    printf("\nResultado: %d de %d chequeos OK\n\n", tests_total - tests_fallidos, tests_total);
    if (tests_fallidos > 0) {
        printf("HAY TESTS FALLIDOS\n");
    }
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
