#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define GRID_SIZE 10
#define NUM_SHIPS 4
#define EASY 0
#define HARD 1
#define MAX_RADAR_SWEEPS 3

typedef struct
{
    char name[20];
    int size;
    int hits;
    int sunk;
    int coordinates[5][2];
} Ship;

typedef struct
{
    char display;
    int hasShip;
    int isHit;
    int hasSmoke;
} GridCell;

typedef struct
{
    char name[50];
    GridCell grid[GRID_SIZE][GRID_SIZE];
    Ship ships[NUM_SHIPS];
    int radarCount;
    int smokeCount;
    int artilleryAvailable;
    int torpedoAvailable;
    int shipsSunk;
} Player;

void initializeGrid(GridCell grid[GRID_SIZE][GRID_SIZE]);
void displayGrid(GridCell grid[GRID_SIZE][GRID_SIZE], int trackingDifficulty);
int getTrackingDifficulty();
void initializePlayer(Player *player);
void placeShips(Player *player);
int validateAndPlaceShip(GridCell grid[GRID_SIZE][GRID_SIZE], Ship *ship, int row, int col, char *orientation);
void performMove(Player *attacker, Player *defender, int trackingDifficulty);
void showMoveOptions(Player *player);
void performFire(Player *attacker, Player *defender, char *coord);
void performRadar(Player *attacker, Player *defender, char *coord);
void performSmoke(Player *player, char *coord);
void performArtillery(Player *attacker, Player *defender, char *coord);
void performTorpedo(Player *attacker, Player *defender, char *input);
void updateShipStatus(Player *player, int row, int col);
void unlockSpecialMoves(Player *player);
int checkWinCondition(Player *player);
void gameLoop(Player *player1, Player *player2, int trackingDifficulty);
void clearScreen();
void clearInputBuffer();

int main()
{
    srand(time(NULL)); // to randomize dr zalgout said we can do it like dr zalghout said

    Player player1, player2;
    int trackingDifficulty;

    trackingDifficulty = getTrackingDifficulty();

    initializePlayer(&player1);
    clearScreen();

    initializePlayer(&player2);
    clearScreen();

    printf("\n%s, place your ships.\n", player1.name);
    placeShips(&player1);
    clearScreen();

    printf("\n%s, place your ships.\n", player2.name);
    placeShips(&player2);
    clearScreen();

    Player *currentPlayer, *opponent;
    if (rand() % 2 == 0)
    {
        currentPlayer = &player1;
        opponent = &player2;
    }
    else
    {
        currentPlayer = &player2;
        opponent = &player1;
    }
    printf("%s will go first.\n", currentPlayer->name);

    gameLoop(currentPlayer, opponent, trackingDifficulty);

    return 0;
}

void initializeGrid(GridCell grid[GRID_SIZE][GRID_SIZE])
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        for (int j = 0; j < GRID_SIZE; j++)
        {
            grid[i][j].display = '~';
            grid[i][j].hasShip = 0;
            grid[i][j].isHit = 0;
            grid[i][j].hasSmoke = 0;
        }
    }
}

void displayGrid(GridCell grid[GRID_SIZE][GRID_SIZE], int trackingDifficulty)
{
    printf("   A B C D E F G H I J\n");
    for (int i = 0; i < GRID_SIZE; i++)
    {
        printf("%2d", i + 1);
        for (int j = 0; j < GRID_SIZE; j++)
        {
            if (trackingDifficulty == HARD && grid[i][j].isHit == 0)
            {
                printf(" ~");
            }
            else
            {
                printf(" %c", grid[i][j].display);
            }
        }
        printf("\n");
    }
}

int getTrackingDifficulty()
{
    int choice;
    printf("Select tracking difficulty level:\n");
    printf("1. Easy (tracks hits and misses)\n");
    printf("2. Hard (tracks hits only)\n");
    printf("Enter choice (1 or 2): ");
    scanf("%d", &choice);
    while (choice != 1 && choice != 2)
    {
        printf("Invalid choice. Please enter 1 or 2: ");
        scanf("%d", &choice);
    }
    clearInputBuffer();
    return (choice == 1) ? EASY : HARD;
}

void initializePlayer(Player *player)
{ // got help from my friend (Fadel) on this due to some errors
    printf("Enter player name: ");
    fgets(player->name, sizeof(player->name), stdin);
    player->name[strcspn(player->name, "\n")] = '\0';
    initializeGrid(player->grid);
    player->radarCount = MAX_RADAR_SWEEPS;
    player->smokeCount = 0;
    player->artilleryAvailable = 0;
    player->torpedoAvailable = 0;
    player->shipsSunk = 0;

    char *shipNames[NUM_SHIPS] = {"Carrier", "Battleship", "Destroyer", "Submarine"};
    int shipSizes[NUM_SHIPS] = {5, 4, 3, 2};

    for (int i = 0; i < NUM_SHIPS; i++)
    {
        strncpy(player->ships[i].name, shipNames[i], sizeof(player->ships[i].name));
        player->ships[i].size = shipSizes[i];
        player->ships[i].hits = 0;
        player->ships[i].sunk = 0;
        memset(player->ships[i].coordinates, 0, sizeof(player->ships[i].coordinates));
    }
}

void placeShips(Player *player)
{
    char coord[10];
    char orientation[15];

    for (int i = 0; i < NUM_SHIPS; i++)
    {
        int validPlacement = 0;
        while (!validPlacement)
        {
            printf("%s, place your %s (size %d).\n", player->name, player->ships[i].name, player->ships[i].size);
            printf("Enter starting coordinate (e.g., B3): ");
            fgets(coord, sizeof(coord), stdin);
            coord[strcspn(coord, "\n")] = '\0';

            printf("Enter orientation (horizontal/vertical): ");
            fgets(orientation, sizeof(orientation), stdin);
            orientation[strcspn(orientation, "\n")] = '\0';
            int row, col;
            if (sscanf(coord, "%c%d", &coord[0], &row) != 2)
            {
                printf("Invalid coordinate format. Try again.\n");
                continue;
            }
            row -= 1;
            col = toupper(coord[0]) - 'A';

            validPlacement = validateAndPlaceShip(player->grid, &player->ships[i], row, col, orientation);
            if (!validPlacement)
            {
                printf("Invalid placement. Try again.\n");
            }
            else
            {
                printf("Ship placed successfully.\n");
            }
        }
    }
}

int validateAndPlaceShip(GridCell grid[GRID_SIZE][GRID_SIZE], Ship *ship, int row, int col, char *orientation)
{
    int carl = 0, carl1 = 0;
    if (strcmp(orientation, "horizontal") == 0)
    {
        carl1 = 1;
    }
    else if (strcmp(orientation, "vertical") == 0)
    {
        carl = 1;
    }
    else
    {
        return 0;
    }

    if (row < 0 || col < 0 || row >= GRID_SIZE || col >= GRID_SIZE || row + carl * (ship->size - 1) >= GRID_SIZE || col + carl1 * (ship->size - 1) >= GRID_SIZE)// here qwe check if the boat length and the location we p[laecd it on cause the boat to exit the grid]
    {
        return 0;
    }

    for (int i = 0; i < ship->size; i++) // here we check if the boat we placed is in a place or might go over a boat 
    {
        if (grid[row + dr * i][col + carl1 * i].hasShip)
        {
            return 0;
        }
    }

    for (int i = 0; i < ship->size; i++) // if both conditions are false we place the ship
    {
        grid[row + dr * i][col + carl1 * i].hasShip = 1;
        ship->coordinates[i][0] = row + carl * i;
        ship->coordinates[i][1] = col + carl1 * i;
    }
    return 1;
}

void performMove(Player *attacker, Player *defender, int trackingDifficulty) { // here we take input from the user for a locationa nd the move he wants like fire torpedo artillary , sweep....
    char input[50];
    printf("\n%s's turn.\n", attacker->name);
    displayGrid(defender->grid, trackingDifficulty);
    showMoveOptions(attacker);

    printf("Enter your move: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';  

    if (strncmp(input, "Fire", 4) == 0) {
        char coord[10];
        if (sscanf(input, "Fire %s", coord) == 1) {
            performFire(attacker, defender, coord);
        } else {
            printf("Invalid command format. You lose your turn.\n");
        }
    } else if (strncmp(input, "Radar", 5) == 0) {
        if (attacker->radarCount > 0) {
            char coord[10];
            if (sscanf(input, "Radar %s", coord) == 1) {
                performRadar(attacker, defender, coord);
            } else {
                printf("Invalid command format. You lose your turn.\n");
            }
        } else {
            printf("No radar sweeps remaining. You lose your turn.\n");
        }
    } else if (strncmp(input, "Smoke", 5) == 0) {
        if (attacker->smokeCount > 0) {
            char coord[10];
            if (sscanf(input, "Smoke %s", coord) == 1) {
                performSmoke(attacker, coord);// waiting for this func to be implemented later
            } else {
                printf("Invalid command format. You lose your turn.\n");
            }
        } else {
            printf("No smoke screens available. You lose your turn.\n");
        }
    } else if (strncmp(input, "Artillery", 9) == 0) {
        if (attacker->artilleryAvailable) {
            char coord[10];
            if (sscanf(input, "Artillery %s", coord) == 1) {
                performArtillery(attacker, defender, coord);// waiting for this func to be implemented later
                attacker->artilleryAvailable = 0;  
            } else {
                printf("Invalid command format. You lose your turn.\n");
            }
        } else {
            printf("Artillery not available. You lose your turn.\n");
        }
    } else if (strncmp(input, "Torpedo", 7) == 0) {
        if (attacker->torpedoAvailable) {
            char param[10];
            if (sscanf(input, "Torpedo %s", param) == 1) {
                performTorpedo(attacker, defender, param);// waiting for this func to be implemented later
                attacker->torpedoAvailable = 0; 
            } else {
                printf("Invalid command format. You lose your turn.\n");
            }
        } else {
            printf("Torpedo not available. You lose your turn.\n");
        }
    } else {
        printf("Invalid command. You lose your turn.\n");
    }
}

void showMoveOptions(Player *player) {// dis[lays the name of allowed moves so the pllayer can enter it correctly ]
    printf("Available moves:\n");
    printf("- Fire [coordinate]\n");
    if (player->radarCount > 0) printf("- Radar [coordinate] (Remaining: %d)\n", player->radarCount);
    if (player->smokeCount > 0) printf("- Smoke [coordinate] (Available: %d)\n", player->smokeCount);
    if (player->artilleryAvailable) printf("- Artillery [coordinate] (Available)\n");
    if (player->torpedoAvailable) printf("- Torpedo [row/column] (Available)\n");
} 

void performFire(Player *attacker, Player *defender, char *coord) {// performs the fire operation that is unlimeted and shoots the opps grid 
    int row, col;
    if (sscanf(coord, "%c%d", &coord[0], &row) != 2) {
        printf("Invalid coordinate format. You lose your turn.\n");
        return;
    }
    row -= 1;
    col = toupper(coord[0]) - 'A';

    if (row < 0 || row >= GRID_SIZE || col < 0 || col >= GRID_SIZE) {
        printf("Invalid coordinates. You lose your turn.\n");
        return;
    }

    GridCell *cell = &defender->grid[row][col];

    if (cell->isHit) {
        printf("You already targeted this location. You lose your turn.\n");
        return;
    }

    cell->isHit = 1;

    if (cell->hasShip) {
        cell->display = '*';
        printf("Hit!\n");
        updateShipStatus(defender, row, col);
    } else {
        cell->display = 'o';
        printf("Miss.\n");
    }
}

void performRadar(Player *attacker, Player *defender, char *coord) {//alows player to do the radar
    int row, col;
    if (sscanf(coord, "%c%d", &coord[0], &row) != 2) {
        printf("Invalid coordinate format. You lose your turn.\n");
        return;
    }
    row -= 1;
    col = toupper(coord[0]) - 'A';

    if (row < 0 || row >= GRID_SIZE - 1 || col < 0 || col >= GRID_SIZE - 1) {
        printf("Invalid coordinates. You lose your turn.\n");
        return;
    }

    int found = 0;
    for (int i = row; i <= row + 1; i++) {
        for (int j = col; j <= col + 1; j++) {//we check if we csn use radar due to smoke 
            if (defender->grid[i][j].hasSmoke) continue;
            if (defender->grid[i][j].hasShip) {
                found = 1;
                break;
            }
        }
        if (found) break;
    }

    if (found) {
        printf("Enemy ships found.\n");
    } else {
        printf("No enemy ships found.\n");
    }
    attacker->radarCount--;
}


