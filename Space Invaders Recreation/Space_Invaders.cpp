#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

struct PlayerData
{
	char name[1000];
	int xp;
	int level;
	int money;
	int health;
	int energy;
	int armor;
	int damage;
};

struct Items
{
	char name[100];
	int amount;
	int price;
};

struct Upgrades
{
	int lim;
	int cost;
};

struct Enemy
{
	int damage;
	int index;
	int pos[2];
	int dir;
};

struct Bullet
{
	int damage;
	int pos[2];
};

PlayerData playerDatas[100];
PlayerData curPlayer;
Items items[5];
Items score[5];
Upgrades upgrades[3];
Enemy enemies[200];
Bullet enemyBulletPos[200];
Bullet playerBulletPos[20];
pthread_t th1, th2, th3, th4, th5, th6;
int dataAmount = 0;
int c;
int playerInput = 1;
int loadInput = 1;
int p_x, p_y;
int b_x, b_y;
int lim_x, lim_y;
int lobbyMapLength = 0;
int mapLength = 0;
int spaceShipLength[4] = {0};
int npcPos[3][3] = {{26, 4, 0}, {6, 16, 0}, {27, 16, 0}};
int leaderboardPos[3][2] = {{5, 9}, {5, 10}, {5, 11}};
int spaceShipOffset[4][2] = {{2, 2}, {3, 2}, {1, 2}, {3, 2}};
int coolBulletPos[10][3][3];
int xDir[4] = {1, -1, 0, 0};
int yDir[4] = {0, 0, 1, -1};
int playerNextToNpc = 0;
int playerNextToLeaderboard = 0;
int playerNextToDoor = 0;
int playerAtCenter = 0;
int playerAtExit = 0;
int playerInteracting = 0;
int upFail = 0;
int chosenSpaceship = 0;
int gameInProgress = 0;
int bullet = 0;
int curEnergy = 0;
int curHealth = 0;
int haveDrawn = 0;
int curScore = 0;
int reloading = 0;
int unavailable = -1;
int available = -1;
int floodfillin = 0;
char key;
char lastTenMoves[17] = {'a'};
char lobbyMap[50][50];
char itemName[5][20] = {"Potion", "Max Potion", "Energy", "Max Energy", "Bomb"};
char enemiesShip[3] = {'v', 'x', 'o'};
char gameMap[30][100];
char spaceship[4][50][50];
char spaceShipName[4][30] = {"Default Spaceship", "Great Spaceship", "Little Spaceship", "Rocket Spaceship"};
char cheatCodes[4][17] = {"WASDWASDWASDWASD", "ASDASDASDASD", "WSWSWSADADAD", "DASWDASWDASWDASW"};
char gameMapMod[30][100];

void mainMenu();
void game();
void lobby();
void saveGame();

int get_pow(int num, int pow)
{
	int ans = 1;
	for (int i = 0; i < pow; i++){
		ans *= num;
	}
	
	return ans;
}

int getRandomNum(int min, int max)
{
	return rand() % (max + 1 - min) + min;
}

double getRandomDouble(double min, double max)
{
	double div = RAND_MAX / (max - min);
	return min + (rand() / div);
}

void updateLastMove(char lastMove)
{
	for (int i = 1; i < 16; i++){
		lastTenMoves[i - 1] = lastTenMoves[i];
	}
	lastTenMoves[15] = lastMove;
	for (int i = 0; i < 4; i++){
		int correct = 1;
		for (int j = strlen(cheatCodes[i]) - 1; j >= 0; j--){
			if (cheatCodes[i][j] != lastTenMoves[j + strlen(lastTenMoves) - strlen(cheatCodes[i])]){
				correct = 0;
			}
		}
		if (correct){
			switch(i){
				case 0:
					items[4].amount = 99;
					break;
				case 1:
					for (int i = 0; i < 4; i++){
						items[i].amount = 99;
					}
					break;
				case 2:
					curPlayer.health = upgrades[0].lim;
					curPlayer.energy = upgrades[1].lim;
					curPlayer.armor = upgrades[2].lim;
					break;
				case 3:
					curPlayer.level = 100;
					break;
			}
		}
	}
}

void sortLeaderboard(int startIndex, int endIndex)
{
	int pivot = endIndex;
	int bigFromRightIndex = endIndex - 1;;
	int smalFromLeftIndex = startIndex;
	int gotSmal = 0;
	int gotBig = 0;
	if (smalFromLeftIndex >= bigFromRightIndex){
		return;
	}
	while (smalFromLeftIndex < bigFromRightIndex){
		gotSmal = 0;
		gotBig = 0;
		for (int i = smalFromLeftIndex; i < pivot; i++){
			if (score[i].amount < score[pivot].amount){
				smalFromLeftIndex = i;
				gotSmal = 1;
				break;
			}
		}
		for (int i = bigFromRightIndex; i >= smalFromLeftIndex; i--){
			if (score[i].amount > score[pivot].amount){
				bigFromRightIndex = i;
				gotBig = 1;
				break;
			}
		}
		if (bigFromRightIndex > smalFromLeftIndex && gotSmal && gotBig){
			Items temp = score[bigFromRightIndex];
			score[bigFromRightIndex] = score[smalFromLeftIndex];
			score[smalFromLeftIndex] = temp;
		}
		else if (!gotSmal || !gotBig){
			break;
		}
	}
	if (gotSmal){
		Items temp = score[pivot];
		score[pivot] = score[smalFromLeftIndex];
		score[smalFromLeftIndex] = temp;	
	}	
	sortLeaderboard(startIndex, smalFromLeftIndex - 1);
	sortLeaderboard(smalFromLeftIndex + 1, endIndex);
}

void setting()
{
	system("cls");
	printf("\n\n\tWelcome to the game!\n"
	"\tBasic Movement\n"
	"\t'w' - Move Up\n"
	"\t'a' - Move Left\n"
	"\t's' - Move Down\n"
	"\t'd' - Move Right\n\n"
	"\tLobby Arena\n"
	"\t'SPACE' - Interact\n"
	"\t'y' - Talk?\n"
	"\t'o' - Open Backpack\n\n"
	"\tGame Arena\n"
	"\t'SPACE' - Shoot\n"
	"\t'r' - Reload\n"
	"\t'f' - Skill\n"
	"\t'1' - Use Potion\n"
	"\t'2' - Use Max Potion\n"
	"\t'3' - Use Energy\n"
	"\t'4' - Use Max Energy\n"
	"\t'g' - Use Bomb\n\n"
	"\tBack to menu ? [press enter]\n");
	getchar();
	mainMenu();
}

void drawMainMenu()
{
	system("cls");
	char text[5][15] = {"NEW GAME", "LOAD GAME", "SETTING", "HOW TO PLAY", "EXIT"};
	printf("\n\n");
	for (int i = 0; i < 5; i++){
		printf("\t%-20s", text[i]);
		if (playerInput - 1 == i){
			printf("<");
		}
		printf("\n");
	}
}

void drawStationScreen()
{
	char text[2][30] = {"Welcome to station", "Select your spaceship!"};
	system("cls");
	printf("\n\n%28s\n"
	"%30s\n\n"
	"%27s\n\n", text[0], text[1], spaceShipName[loadInput - 1]);
	
	for (int i = 0; i < spaceShipLength[loadInput - 1]; i++){
		printf("\t\t%s\n", spaceship[loadInput - 1][i]);
	}

	printf("\n\n\t[press enter to choose]\n"
	"\t< ------------------- >\n");
	printf("\n");
}

void drawItemScreen()
{
	system("cls");
	printf("\n\n\tWelcome to the item shop\n");
	for (int i = 0; i < 5; i++){
		printf("\t%d. %s [$%d]", i + 1, items[i].name, items[i].price);
		if (loadInput - 1 == i){
			printf("\t\t<");
		}
		printf("\n");
	}
	printf("\t6. Back");
	if (upFail){
		int amount = 0;
		printf("\n\n\tHow many %s do you want to buy?\n\t- ", items[loadInput - 1].name);
		scanf("%d", &amount);
		while ((c = getchar()) != EOF && c != '\n'){}
		upFail = 0;	
		
		if (curPlayer.money >= items[loadInput - 1].price * amount){
			items[loadInput - 1].amount += amount;
			curPlayer.money -= items[loadInput - 1].price * amount;
			printf("\n\n\tSuccessfully purchased %d %s! [press enter]", amount, items[loadInput - 1]);
		}
		else{
			printf("\n\n\tYou don't have enough money! [press enter]");			
		}
		
		getchar();
		
		drawItemScreen();
	}
	else{
		if (loadInput == 6){
			printf("\t\t<");
		}
	}	
	printf("\n");
}

void drawUpgradeScreen()
{
	system("cls");
	printf("\n\n\tDo you need anything to upgrade?\n");
	for (int i = 0; i < 3; i++){
		printf("\t%d. ", i + 1);
		switch (i){
			case 0:
				printf("HP %d", curPlayer.health);
				break;
			case 1:
				printf("Energy %d", curPlayer.energy);
				break;
			case 2:
				printf("Armor %d", curPlayer.armor);
				break;
		}
		printf("/%d - $%d", upgrades[i].lim, upgrades[i].cost);
		if (loadInput - 1 == i){
			printf("\t\t<");
		}
		printf("\n");
	}
	printf("\t%d. Back", 4);
	if (upFail){
		switch(upFail){
			case 1:
				printf("\n\n\tYou already at the max levels [press enter]");
				break;
			case 2:
				printf("\n\n\tSuccessfully upgraded skills [press enter]");
				break;
			case 3:
				printf("\n\n\tYou don't have enough money! [press enter]");
				break;
			case 4:
				printf("\n\n\tGood bye! Don't forget to come again [press enter]");
				break;
		}
		getchar();
		upFail = 0;
		drawUpgradeScreen();
	}
	else{
		if (loadInput == 4){
			printf("\t\t<");
		}
	}	
	printf("\n");
}

void drawLoadGame()
{
	system("cls");
	printf("\n\n\tSave Data :\n");
	for (int i = 0; i < dataAmount; i++){
		printf("\t%d. %s [Level : %d]", i + 1, playerDatas[i].name, playerDatas[i].level);
		if (loadInput - 1 == i){
			printf("\t\t<");
		}
		printf("\n");
	}
	printf("\t%d. Back", dataAmount + 1);
	if (loadInput - 1 == dataAmount){
		printf("\t\t<");
	}
	printf("\n");
}

void drawLobby()
{
	for (int y = 0; y < lobbyMapLength; y++){
		for (int x = 0; lobbyMap[y][x] != '\0'; x++){
			if (x == p_x && y == p_y){
				printf("P");
			}
			else if (lobbyMap[y][x] != '\n'){
				printf("%c", lobbyMap[y][x]);
			}
		}
		if (y == 3){
			printf("\t[%s]", curPlayer.name);
		}
		else if (y == 4){
			printf("\tLevel: %d", curPlayer.level);
		}
		else if (y == 5){
			printf("\tMoney: %d", curPlayer.money);
		}
		else if ((y == 11 || y == 12) && playerInteracting){
			for (int i = 0; i < 3; i++){
				if (npcPos[i][2] == 1){
					switch (y){
						case 11:
							switch (i){
								case 0:		
									printf("\tWelcome to Spaceship Station");
									break;
								case 1:
									printf("\tHohoo! This is an upgrade shop");
									break;
								case 2:
									printf("\tWelcome to item shop!");
									break;
							}
							break;
						case 12:
							switch (i){
								case 0:	
									printf("\tDo you want to buy any weapon? [y/n]");	
									break;
								case 1:
									printf("\tAnything I can do for you? [y/n]");
									break;
								case 2:
									printf("\tDo you want to buy any item? [y/n]");
									break;
							}
							break;
					}
				}
			}
		}
		else if (y == 14){
			if (playerNextToNpc || playerAtCenter){
				printf("\tPress SPACE To Interact");
			}
			else if (playerNextToLeaderboard){
				printf("\tPress SPACE to see Leaderboard");
			}
			else if (playerNextToDoor){
				printf("\tYou need level 10 to unlock spaceship");
			}
			else if (playerAtExit){
				printf("\tPress SPACE to Exit!");
			}
		}
		printf("\n");
	}
}

void stationScreen()
{
	loadInput = 1;
	int finished = 0;
	drawStationScreen();
	while (!finished){
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'a':
				case 'A':
					loadInput--;
					if (loadInput < 1){
						loadInput += 4;
					}
					break;
				case 'd':
				case 'D':
					loadInput++;
					if (loadInput > 4){
						loadInput -= 4;
					}
					break;
				case '\r':			
					printf("\n\n\tSpaceship Chosen [press enter]\n");
					chosenSpaceship = loadInput - 1;
					getchar();
					finished = 1;
					break;
			}
			drawStationScreen();
		}
	}	
	system("cls");
}

void itemScreen()
{
	loadInput = 1;
	int finished = 0;
	drawItemScreen();
	while (!finished){
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'w':
				case 'W':
					if (loadInput > 1){
						loadInput--;
					}
					break;
				case 's':
				case 'S':
					if (loadInput < 6){
						loadInput++;
					}
					break;
				case '\r':			
					if (loadInput == 6){
						finished = 1;
					}
					else{
						upFail = 1;	
					}
					break;
			}
			drawItemScreen();
		}
	}	
	system("cls");
}

void upgradeScreen()
{
	loadInput = 1;
	int finished = 0;
	drawUpgradeScreen();
	while (!finished){
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'w':
				case 'W':
					if (loadInput > 1){
						loadInput--;
					}
					break;
				case 's':
				case 'S':
					if (loadInput < 4){
						loadInput++;
					}
					break;
				case '\r':
					int curLvl = 0;
					switch (loadInput){
						case 1:
							curLvl = curPlayer.health;
							break;
						case 2:
							curLvl = curPlayer.energy;
							break;
						case 3:
							curLvl = curPlayer.armor;
							break;
						case 4:
							finished = 1;
							upFail = 4;
							break;
					}
					if (loadInput != 4){
						if (curPlayer.money >= upgrades[loadInput - 1].cost && upgrades[loadInput - 1].lim > curLvl){
							switch (loadInput){
								case 1:
									curPlayer.health++;
									break;
								case 2:
									curPlayer.energy++;
									break;
								case 3:
									curPlayer.armor++;
									break;
							}
							curPlayer.money -= upgrades[loadInput - 1].cost;
							upFail = 2;
						}
						else if (curPlayer.money < upgrades[loadInput - 1].cost){
							upFail = 3;
						}	
						else if (upgrades[loadInput - 1].lim <= curLvl){
							upFail = 1;
						}
					}		
					break;
			}
			drawUpgradeScreen();
		}
	}	
	saveGame();
	system("cls");
}

void howToPlay()
{
	system("cls");
	printf("\n\n\tSpace Invader\n\n\n"
	"\tThe objective of the game is, basically, to shoot the invaders with your shooter\n"
	"\twhile avoiding their shots and prevention an invasion. Amassing a high score is a further\n"
	"\tobjective and one that must be prioritised against your continued survival\n\n\n"
	"\tThere is no time limit,\n"
	"\texcept for the fact that if you do not shoot them all before they reach\n"
	"\tthe bottom of the screen the game ends\n\n\n"
	"\tBack to menu ? [press enter]\n");
	getchar();
	mainMenu();
}

void saveGame()
{
	FILE *f;
	f = fopen("./database/player.dat", "w");
	for (int i = 0; i < dataAmount; i++){
		PlayerData data = playerDatas[i];
		if (strcmp(data.name, curPlayer.name) == 0){
			data = curPlayer;
			playerDatas[i] = curPlayer;
		}
		if (i == dataAmount - 1){
			fprintf(f, "%s#%d#%d#%d#%d#%d#%d#%d", data.name, data.money, data.xp, data.level, data.health, data.energy, data.armor, data.damage);
		}
		else{
			fprintf(f, "%s#%d#%d#%d#%d#%d#%d#%d\n", data.name, data.money, data.xp, data.level, data.health, data.energy, data.armor, data.damage);
		}
	}
	fclose(f);
	
	sortLeaderboard(0, 4);
	f = fopen("./database/score.dat", "w");
	for (int i = 0; i < 5; i++){
		if (i == 4){
			fprintf(f, "%s#%d", score[i].name, score[i].amount);
		}
		else{
			fprintf(f, "%s#%d\n", score[i].name, score[i].amount);
		}
	}
	
	fclose(f);
}

void exitGame()
{
	system("cls");
	FILE *f;
	f = fopen("./assets/logo.txt", "r");
	char logo[1000][1000];
	char motto[100] = "Imagine that this is a very cool motto";
	int index = 0;
	while (!feof(f)){
//		fscanf(f, "%[^\n]\n", &logo[index++]);
		fgets(logo[index++], 1000, f);
	}
	for (int i = 0; i < index; i++){
		for (int j = 0; logo[i][j] != '\0'; j++){
			printf("%c", logo[i][j]);
		}	
	}
	printf("\n\t\t\t");
	for (int i = 0; motto[i] != '\0'; i++){
		printf("%c", motto[i]);
		usleep(20000);
	}

	fclose(f);
	exit(0);
}

void newGame()
{
	char name[1000];	
	int usable = 0;
	
	while (!usable){
		usable = 1;
		system("cls");
		printf("\n\n\tSelect Your New Name: ");
		scanf("%[^\n]", &name);
		while ((c = getchar()) != EOF && c != '\n'){}
		if (strlen(name) < 3){
			usable = 0;
			printf("\n\tPlease insert more than 3 characters [press enter]\n");
			getchar();
			continue;
		}		
		for (int i = 0; i < dataAmount; i++){
			int same = 1;
			for (int j = 0; name[j] != '\0'; j++){
				if (name[j] != playerDatas[i].name[j]){
					same = 0;
				}	
			}
			if (same){
				usable = 0;
				printf("\n\tSorry but your name already exists! [press enter]\n");
				getchar();
				continue;
			}
		}	
		for (int j = 0; name[j] != '\0'; j++){
			if (name[j] == '#'){
				usable = 0;
				printf("\n\tName can't contain '#'! [press enter]\n");
				getchar();
				break;
			}
		}
	}
	playerDatas[dataAmount].armor = 1;
	playerDatas[dataAmount].damage = 1;
	playerDatas[dataAmount].energy = 50;
	playerDatas[dataAmount].health = 100;
	playerDatas[dataAmount].level = 0;
	playerDatas[dataAmount].money = 0;
	strcpy(playerDatas[dataAmount].name, name);
	playerDatas[dataAmount].xp = 0;
	curPlayer = playerDatas[dataAmount];
	dataAmount++;
	saveGame();
	lobby();
}

void loadGame()
{
	loadInput = 1;
	int finished = 0;
	drawLoadGame();
	while (!finished){
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'w':
				case 'W':
					if (loadInput > 1){
						loadInput--;
					}
					break;
				case 's':
				case 'S':
					if (loadInput < dataAmount + 1){
						loadInput++;
					}
					break;
				case '\r':
					finished = 1;
					break;
			}
			drawLoadGame();
		}
	}
	if (loadInput - 1 == dataAmount){
		mainMenu();
	}
	else{
		curPlayer = playerDatas[loadInput - 1];	
		lobby();
	}		
}

void openBackpack()
{
	system("cls");
	printf(
	"\n\n\t%s's backpack\n"
	"\t===================\n", curPlayer.name);
	for (int i = 0; i < 5; i++){
		if (items[i].amount > 0){
			printf("\t- %s: %d\n", items[i].name, items[i].amount);
		}
	}
	printf("\n\n\tback to game [press enter]\n");
	getchar();
	system("cls");
}

void openLeaderboard()
{
	sortLeaderboard(0, 4);
	system("cls");
	printf(
	"\n\n\tC Space Invader Scoreboard\n"
	"\t====================================\n\n");
	for (int i = 0; i < 5; i++){
		printf("\t%d. %-25s | %d\n", i + 1, score[i].name, score[i].amount);
	}
	printf("\n\n\tback to game [press enter]\n");
	getchar();
	system("cls");
}

void lobby()
{
	playerAtExit = 0;
	int finished = 0;
	system("cls");
	p_x = 17;
	p_y = 19;
	if (curPlayer.level < 10){
		lobbyMap[7][5] = ' ';
		lobbyMap[7][10] = ' ';
		for (int i = 6; i < 10; i++){
			lobbyMap[6][i] = '-';
		}	
	}
	else{
		lobbyMap[7][5] = '/';
		lobbyMap[7][10] = '\\';
		for (int i = 6; i < 10; i++){
			lobbyMap[6][i] = ' ';
		}	
	}
	drawLobby();
	while (!finished){
		if (_kbhit){
			key = _getch();
			system("cls");
			switch(key){
				case 'w':
				case 'W':
					if (lobbyMap[p_y - 1][p_x] == ' '){
						p_y--;
					}
					updateLastMove('W');
					break;
				case 'a':
				case 'A':
					if (lobbyMap[p_y][p_x - 1] == ' '){
						p_x--;	
					}
					updateLastMove('A');
					break;
				case 's':
				case 'S':
					if (lobbyMap[p_y + 1][p_x] == ' '){
						p_y++;	
					}
					updateLastMove('S');
					break;
				case 'd':
				case 'D':
					if (lobbyMap[p_y][p_x + 1] == ' '){
						p_x++;
					}
					updateLastMove('D');
					break;
				case 'o':
				case 'O':
					openBackpack();
					break;
				case ' ':
					if (playerNextToLeaderboard){
						openLeaderboard();
					}
					else if (playerNextToNpc){
						playerInteracting = 1;
					}
					else if (playerAtExit){
						saveGame();
						finished = 1;
					}
					else if (playerAtCenter){
						game();
					}
					break;
				case 'y':
				case 'Y':
					if (playerInteracting){
						upFail = 0;
						if (npcPos[0][2] == 1){
							stationScreen();
						}	
						else if (npcPos[1][2] == 1){
							upgradeScreen();
						}
						else if (npcPos[2][2] == 1){
							itemScreen();
						}
					}	
					break;
				case 'n':
				case 'N':
					playerInteracting = 0;
					break;
			}
			playerNextToNpc = 0;
			playerNextToLeaderboard = 0;
			playerNextToDoor = 0;
			playerAtCenter = 0;
			playerAtExit = 0;
			for (int i = 0; i < 3; i++){
				npcPos[i][2] = 0;
				int dis = get_pow(npcPos[i][0] - p_x, 2) + get_pow(npcPos[i][1] - p_y, 2);
				if (dis <= 1){
					playerNextToNpc = 1;
					npcPos[i][2] = 1;
				}
				if (p_x == leaderboardPos[i][0] && p_y == leaderboardPos[i][1]){
					playerNextToLeaderboard = 1;
				}
			}
			if (!playerNextToNpc){
				playerInteracting = 0;
			}
			for (int i = 6; i < 10; i++){
				if (p_y == 7 && p_x == i && curPlayer.level < 10){
					playerNextToDoor = 1;
				}
			}
			if (p_x == 17 && p_y == 10){
				playerAtCenter = 1;
			}
			else if (p_y == 20 && p_x == 17){
				playerAtExit = 1;
			}
			drawLobby();
		}
	}
	
	mainMenu();
}

void drawGame()
{
	system("cls");
	haveDrawn = 1;
	if (!floodfillin && gameInProgress){
		for (int i = 0; i < mapLength; i++){
			strcpy(gameMapMod[i], gameMap[i]);	
		}
		for (int i = 0; i < 200; i++){
			if (enemies[i].damage != 0){
				gameMapMod[enemies[i].pos[1]][enemies[i].pos[0]] = enemiesShip[enemies[i].index];
				gameMapMod[enemies[i].pos[1]][enemies[i].pos[0] - 1] = '\\';
				gameMapMod[enemies[i].pos[1]][enemies[i].pos[0] + 1] = '/';
			}
			if (enemyBulletPos[i].damage != 0){
				gameMapMod[enemyBulletPos[i].pos[1]][enemyBulletPos[i].pos[0]] = '*';
			}
		}
		
		for (int i = 0; i < 20; i++){
			if (playerBulletPos[i].damage != 0){
				gameMapMod[playerBulletPos[i].pos[1]][playerBulletPos[i].pos[0]] = '^';
			}
		}
		
		for (int i = 0; i < 10; i++){
			for (int j = 0; j < 3; j++){
				if (coolBulletPos[i][j][2] == 1){
					gameMapMod[coolBulletPos[i][j][1]][coolBulletPos[i][j][0]] = '^';
				}
			}
		}
		for (int i = b_y - spaceShipOffset[chosenSpaceship][1]; i <= b_y + spaceShipOffset[chosenSpaceship][1]; i++){
			for (int j = b_x - spaceShipOffset[chosenSpaceship][0]; j <= b_x + spaceShipOffset[chosenSpaceship][0]; j++){
				gameMapMod[i][j] = spaceship[chosenSpaceship][i - b_y + spaceShipOffset[chosenSpaceship][1]][j - b_x + spaceShipOffset[chosenSpaceship][0]];
			}	
		}				
	}		
	for (int i = 0; i < mapLength; i++){
		printf("%s", gameMapMod[i]);
		switch (i){
			case 6:
				printf("\tC Space Invader");
				break;
			case 7:
				printf("\t%s", spaceShipName[chosenSpaceship]);
				break;
			case 9:
				printf("\tScore: %d", curScore);
				break;
			case 10:
				printf("\tBullets: %d/10", bullet);
				break;
			case 12:
				printf("\tHP [%d/%d]", curHealth, curPlayer.health);
				break;
			case 13:
				printf("\tEnergy %d/%d", curEnergy, curPlayer.energy);
				break;
			case 15:
				if (reloading){
					printf("\tReloading...");
				}
				else if (unavailable != -1){
					printf("\tYou don't have any %s left!", items[unavailable].name);
				}
				else if (available != -1){
					if (available == 4){
						printf("\tBOMB!");
					}
					else{
						printf("\tUsed %s!", items[available].name);
					}	
				}
				break;
		}
		printf("\n");
	}		
}

void *moveBullet(void *arg)
{
	usleep(2500);
	if (gameInProgress){
		for (int i = 0; i < 200; i++){
			if (enemyBulletPos[i].damage != 0){
				enemyBulletPos[i].pos[1]++;
				if (enemyBulletPos[i].pos[1] > lim_y){
					enemyBulletPos[i].damage = 0;
					enemyBulletPos[i].pos[0] = 0;
					enemyBulletPos[i].pos[1] = 0;
				}
				else{
					if (enemyBulletPos[i].pos[1] - b_y <= spaceShipOffset[chosenSpaceship][1] && enemyBulletPos[i].pos[1] - b_y >= -spaceShipOffset[chosenSpaceship][1]){
						if (enemyBulletPos[i].pos[0] - b_x <= spaceShipOffset[chosenSpaceship][0] && enemyBulletPos[i].pos[0] - b_x >= -spaceShipOffset[chosenSpaceship][0]){
							int protection = getRandomNum(0, curPlayer.armor);
							enemyBulletPos[i].damage -= protection;
							curHealth -= enemyBulletPos[i].damage;
							enemyBulletPos[i].damage = 0;
							enemyBulletPos[i].pos[0] = 0;
							enemyBulletPos[i].pos[1] = 0;
							if (curHealth <= 0){
								gameInProgress = 0;
							}
						}
					}
				}
			}		
		}
		for (int i = 0; i < 20; i++){
			if (playerBulletPos[i].damage != 0){
				playerBulletPos[i].pos[1]--;
				if (playerBulletPos[i].pos[1] < 0){
					playerBulletPos[i].damage = 0;
					playerBulletPos[i].pos[0] = 0;
					playerBulletPos[i].pos[1] = 0;
				}
				else{
					for (int j = 0; j < 200; j++){
						if (playerBulletPos[i].pos[1] == enemies[j].pos[1] && enemies[j].damage != 0){
							if (playerBulletPos[i].pos[0] - enemies[j].pos[0] <= 1 && playerBulletPos[i].pos[0] - enemies[j].pos[0] >= -1){
								enemies[j].damage = 0;
								enemies[j].dir = -1;
								enemies[j].index = 0;
								enemies[j].pos[0] = 0;
								enemies[j].pos[1] = 0;
								playerBulletPos[i].damage = 0;
								playerBulletPos[i].pos[0] = 0;
								playerBulletPos[i].pos[1] = 0;
								curScore += 10;
								break;
							}
						}
					}
				}
			}
		}
		for (int i = 0; i < 10; i++){
			for (int j = 0; j < 3; j++){
				if (coolBulletPos[i][j][2] == 1){
					coolBulletPos[i][j][1]--;
					coolBulletPos[i][j][0] += j - 1;
					if (coolBulletPos[i][j][1] < 0){
						coolBulletPos[i][j][2] = 0;
					}
					else{
						for (int k = 0; k < 200; k++){
							if (coolBulletPos[i][j][1] == enemies[k].pos[1] && enemies[k].damage != 0){
								if (coolBulletPos[i][j][0] - enemies[k].pos[0] <= 1 && coolBulletPos[i][j][0] - enemies[k].pos[0] >= -1){
									enemies[k].damage = 0;
									enemies[k].dir = -1;
									enemies[k].index = 0;
									enemies[k].pos[0] = 0;
									enemies[k].pos[1] = 0;
									coolBulletPos[i][j][2] = 0;
									curScore += 10;
									break;
								}
							}
						}
					}
				}
			}
		}
		drawGame();
		pthread_join(th3, NULL);
		pthread_create(&th3, NULL, &moveBullet, NULL);
		pthread_join(th3, NULL);
		return NULL;
	}	
}

void floodfill(int tes_x, int tes_y)
{
	if (tes_y < -1 || tes_x < 1 || tes_x > lim_x || tes_y > lim_y + 1 || gameMapMod[tes_y][tes_x] == '.') return;
	gameMapMod[tes_y][tes_x] = '.';
	usleep(100);
	for (int i = 0; i < 4; i++){
		floodfill(tes_x + xDir[i], tes_y + yDir[i]);
	}
}

void *enemyAttack(void *par)
{
	int enemyIndex = *((int *)par);
	usleep(getRandomDouble(0, 1) * 1000000);
	if (enemies[enemyIndex].damage != 0 && gameInProgress){
		for (int i = 0; i < 200; i++){
			if (enemyBulletPos[i].damage == 0){
				enemyBulletPos[i].damage = enemies[enemyIndex].damage;
				enemyBulletPos[i].pos[0] = enemies[enemyIndex].pos[0];
				enemyBulletPos[i].pos[1] = enemies[enemyIndex].pos[1] + 1;
				break;
			}
		}
		pthread_exit(NULL);
	}
	
}

void *reload(void *arg)
{
	sleep(3);
	bullet = 10;
	reloading = 0;
	pthread_exit(NULL);
}

void *spawnEnemies(void *arg)
{
	sleep(3);
	srand(time(NULL));
	if (gameInProgress){
		for (int i = 0; i < 200; i++){
			if (enemies[i].damage != 0){
				int *tesz = (int*)malloc(sizeof(*tesz));
				*tesz = i;
				pthread_create(&th4, NULL, &enemyAttack, tesz);
				enemies[i].pos[1]++;
				if (enemies[i].pos[1] >= mapLength){
					gameInProgress = 0;
				}
			}
		}
		int randomX = getRandomNum(3, 38);
		for (int i = 0; i < 3; i++){
			int enemyIndex = getRandomNum(0, 2);
			for (int j = 0; j < 200; j++){
				if (enemies[j].damage == 0){
					switch (enemyIndex){
						case 0:
							enemies[j].damage = 10;
							break;
						case 1:
							enemies[j].damage = 20;
							break;
						case 2:
							enemies[j].damage = 25;
							break;
					}
					enemies[j].index = enemyIndex;
					enemies[j].pos[0] = randomX + 4 * i;
					break;
				}
			}
		}	
		pthread_join(th1, NULL);
		pthread_create(&th1, NULL, &spawnEnemies, NULL);
		pthread_join(th1, NULL);
		return NULL;
	}	
}

void *moveEnemies(void *arg)
{
	sleep(4);
	if (gameInProgress){
		for (int i = 0; i < 200; i++){
			if (enemies[i].damage != 0){
				(enemies[i].dir == 1) ? enemies[i].pos[0]++ : enemies[i].pos[0]--;
				enemies[i].dir *= -1;
			}
		}
		pthread_join(th2, NULL);
		pthread_create(&th2, NULL, &moveEnemies, NULL);
		pthread_join(th2, NULL);
	}
}

void *increaseEnergy(void *arg)
{
	usleep(200000);
	curEnergy++;
	if (curEnergy > curPlayer.energy) curEnergy = curPlayer.energy;
	pthread_join(th5, NULL);
	pthread_create(&th5, NULL, &increaseEnergy, NULL);
	pthread_join(th5, NULL);
}

void game()
{	
	for (int i = 0; i < 200; i++){
		enemies[i].pos[0] = 0;
		enemies[i].pos[1] = 0;
		enemies[i].damage = 0;
		enemies[i].index = 0;
		enemies[i].dir = -1;
		enemyBulletPos[i].damage = 0;		
	}
	for (int i = 0; i < 20; i++){
		playerBulletPos[i].damage = 0;
	}
	for (int i = 0; i < 10; i++){
		for (int j = 0; j < 3; j++){
			coolBulletPos[i][j][2] = 0;
		}
	}
	available = -1;
	floodfillin = 0;
	unavailable = -1;
	curScore = 0;
	bullet = 10;
	curEnergy = curPlayer.energy;
	curHealth = curPlayer.health;
	gameInProgress = 1;
	pthread_create(&th1, NULL, &spawnEnemies, NULL);
	pthread_create(&th2, NULL, &moveEnemies, NULL);
	pthread_create(&th3, NULL, &moveBullet, NULL);
	pthread_create(&th5, NULL, &increaseEnergy, NULL);
	int start_x = spaceShipOffset[chosenSpaceship][0];
	int start_y = spaceShipOffset[chosenSpaceship][1];
	lim_x = 48 - spaceShipOffset[chosenSpaceship][0];
	lim_y = mapLength - 1 - spaceShipOffset[chosenSpaceship][1];
	b_x = 25;
	b_y = 15;
	while (gameInProgress){
//		drawGame();
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'w':
				case 'W':
					if (b_y - 1 > start_y){
						b_y--;	
					}				
					break;
				case 'a':
				case 'A':
					if (b_x - 1 > start_x){
						b_x--;
					}	
					break;
				case 's':
				case 'S':
					if (b_y + 1 < lim_y){
						b_y++;
					}
					break;
				case 'd':
				case 'D':
					if (b_x + 1 < lim_x){
						b_x++;
					}			
					break;
				case ' ':
					if (bullet > 0 && haveDrawn && !reloading){
						bullet--;
						haveDrawn = 0;
						for (int i = 0; i < 20; i++){
							if (playerBulletPos[i].damage == 0){
								playerBulletPos[i].damage = curPlayer.damage;
								playerBulletPos[i].pos[0] = b_x;
								playerBulletPos[i].pos[1] = b_y - 3;
								break;
							}
						}
					}
					break;
				case 'r':
				case 'R':
					if (!reloading){
						reloading = 1;
						pthread_create(&th6, NULL, &reload, NULL);
					}			
					break;
				case 'f':
				case 'F':
					if (curEnergy >= 30 && !reloading && haveDrawn && bullet >= 3){
						curEnergy -= 30;
						bullet -= 3;
						haveDrawn = 0;
						for (int i = 0; i < 10; i++){
							int canUse = 1;
							for (int j = 0; j < 3; j++){
								if (coolBulletPos[i][j][2] != 0){
									canUse = 0;
								}
							}
							if (canUse){
								for (int j = 0; j < 3; j++){
									coolBulletPos[i][j][0] = b_x;
									coolBulletPos[i][j][1] = b_y - 3;
									coolBulletPos[i][j][2] = 1;
								}
								break;
							}
						}
					}		
					break;
				case '1':
					if (items[0].amount > 0){
						items[0].amount--;
						curHealth += 50;
						if (curHealth > curPlayer.health){
							curHealth = curPlayer.health;
						}
						available = 0;
					}
					else{
						unavailable = 0;
					}
					break;
				case '2':
					if (items[1].amount > 0){
						items[1].amount--;
						curHealth = curPlayer.health;
						available = 1;
					}
					else{
						unavailable = 1;
					}
					break;
				case '3':
					if (items[2].amount > 0){
						items[2].amount--;
						curEnergy += 50;
						if (curEnergy > curPlayer.energy){
							curEnergy = curPlayer.energy;
						}
						available = 2;
					}
					else{
						unavailable = 2;
					}
					break;
				case '4':
					if (items[3].amount > 0){
						items[3].amount--;
						curEnergy = curPlayer.energy;
						available = 3;
					}
					else{
						unavailable = 3;
					}
					break;
				case 'g':
				case 'G':
					if (items[4].amount > 0 && !floodfillin){
						floodfillin = 1;
						items[4].amount--;
						floodfill(b_x, b_y);
						available = 4;
						for (int i = 0; i < 200; i++){
							if (enemies[i].damage != 0){
								enemies[i].damage = 0;
								enemies[i].dir = -1;
								enemies[i].index = 0;
								enemies[i].pos[0] = 0;
								enemies[i].pos[1] = 0;
								curScore += 10;
							}
						}
						floodfillin = 0;
					}	
					else{
						unavailable = 4;
					}
					break;
			}
		}
	}
	usleep(1000);
	system("cls");
	curPlayer.xp += (int)(curScore * 0.75);
	curPlayer.money += (int)(curScore * 0.75);
	if (curPlayer.xp >= 100){
		curPlayer.level += curPlayer.xp / 100;
		curPlayer.xp %= 100;
		if (curPlayer.level > 100){
			curPlayer.level = 100;
		}
	}
	int lowestScore = score[0].amount;
	int lowestIndex = 0;
	for (int i = 1; i < 5; i++){
		if (score[i].amount < lowestScore){
			lowestScore = score[i].amount;
			lowestIndex = i;
		}
	}
	if (curScore > lowestScore){
		score[lowestIndex].amount = curScore;
		strcpy(score[lowestIndex].name, curPlayer.name);
	}
	system("cls");
	printf("\n\n\tC Space Invader\n"
	"\t------------------\n"
	"\tYour score: %d\n"
	"\tYou gain %d exp\n"
	"\tYour current level is: %d\n\n"
	"\tContinue Game [press enter]\n", curScore, (int)(curScore * 0.75), curPlayer.level);
	getchar();
	system("cls");
}

void mainMenu()
{
//	playerInput = 1;
	int finished = 0;
	drawMainMenu();
	while (!finished){
		if (_kbhit){
			key = _getch();
			switch (key){
				case 'w':
				case 'W':
					if (playerInput > 1){
						playerInput--;
					}
					break;
				case 's':
				case 'S':
					if (playerInput < 5){
						playerInput++;
					}
					break;
				case '\r':
					finished = 1;
					break;
			}
			drawMainMenu();
		}
	}
	switch (playerInput){
		case 1:
			newGame();
			break;
		case 2:
			loadGame();
			break;
		case 3:
			setting();
			break;
		case 4:
			howToPlay();
			break;
		case 5:
			exitGame();
			break;
	}
}

int main()
{
	srand(time(NULL));
	
	char playerData[1000][1000];
	char scoreData[10][1000];
	int index = 0;
	FILE *f;
	
	for (int i = 0; i < 3; i++){
		int lim = 0;
		int cost = 0;
		switch (i){
			case 0:
				lim = 300;
				cost = 50;
				break;
			case 1:
				lim = 500;
				cost = 50;
				break;
			case 2:
				lim = 30;
				cost = 30;
				break;
		}
		upgrades[i].lim = lim;
		upgrades[i].cost = cost;
	}
	
	f = fopen("./assets/lobby.txt", "r");
	while (!feof(f)){
		fgets(lobbyMap[lobbyMapLength++], 50, f);
	}
	fclose(f);
	lobbyMap[npcPos[1][1]][npcPos[1][0]] = 'U';
	lobbyMap[npcPos[0][1]][npcPos[0][0]] = 'W';
	lobbyMap[npcPos[2][1]][npcPos[2][0]] = 'I';
	
	f = fopen("./database/score.dat", "r");
	while (!feof(f)){
		fgets(scoreData[index++], 1000, f);
	}
	
	for (int i = 0; i < index; i++){
		int onNum = 0;
		score[i].amount = 0;
		for (int j = 0; scoreData[i][j] != '\0'; j++){
			if (scoreData[i][j] == '#'){
				onNum++;
			}
			else if (!onNum){
				score[i].name[j] = scoreData[i][j];
			}
			else if (scoreData[i][j] - '0' >= 0 && scoreData[i][j] - '0' <= 9){
				score[i].amount *= 10;
				score[i].amount += scoreData[i][j] - '0';
			}
		}	
	}
	
	fclose(f);
	
	f = fopen("./database/player.dat", "r");
	
	while (!feof(f)){
		fgets(playerData[dataAmount++], 1000, f);
	}
	
	for (int i = 0; i < 5; i++){
		items[i].amount = 0;
		strcpy(items[i].name, itemName[i]);
		switch (i){
			case 0:
				items[i].price = 5;
				break;
			case 1:
				items[i].price = 20;
				break;
			case 2:
				items[i].price = 3;
				break;
			case 3:
				items[i].price = 15;
				break;
			case 4:
				items[i].price = 10;
				break;
		}
	}
	
	for (int i = 0; i < dataAmount; i++){
		strcpy(playerDatas[i].name, "");
		playerDatas[i].armor = 0;
		playerDatas[i].damage = 0;
		playerDatas[i].energy = 0;
		playerDatas[i].health = 0;
		playerDatas[i].level = 0;
		playerDatas[i].money = 0;
		playerDatas[i].xp = 0;
		int curStuf = 0;
		for (int j = 0; playerData[i][j] != '\0'; j++){
			if (playerData[i][j] == '#'){
				curStuf++;
			}
			else{
				switch (curStuf){
					case 0:
						playerDatas[i].name[j] = playerData[i][j];
						break;
					case 1:
						playerDatas[i].money *= 10;
						playerDatas[i].money += (playerData[i][j] - '0');					
						break;
					case 2:
						playerDatas[i].xp *= 10;
						playerDatas[i].xp += (playerData[i][j] - '0');			
						break;
					case 3:
						playerDatas[i].level *= 10;
						playerDatas[i].level += (playerData[i][j] - '0');			
						break;
					case 4:
						playerDatas[i].health *= 10;
						playerDatas[i].health += (playerData[i][j] - '0');	
						break;
					case 5:
						playerDatas[i].energy *= 10;
						playerDatas[i].energy += (playerData[i][j] - '0');			
						break;
					case 6:
						playerDatas[i].armor *= 10;
						playerDatas[i].armor += (playerData[i][j] - '0');			
						break;
					case 7:
						if (playerData[i][j] - '0' >= 0 && playerData[i][j] - '0' <= 9){
							playerDatas[i].damage *= 10;
							playerDatas[i].damage += (playerData[i][j] - '0');
						}						
						break;
				}	
			}		
		}	
	}
	fclose(f);
	
	f = fopen("./assets/game.txt", "r");
	while (!feof(f)){
		fgets(gameMap[mapLength++], 100, f);
		gameMap[mapLength - 1][strcspn(gameMap[mapLength - 1], "\n")] = 0;
	}
	fclose(f);
	
	//get spaceship
	f = fopen("./assets/space_1.txt", "r");
	while (!feof(f)){
		fgets(spaceship[0][spaceShipLength[0]++], 50, f);
		spaceship[0][spaceShipLength[0] - 1][strcspn(spaceship[0][spaceShipLength[0] - 1], "\n")] = 0;
	}
	fclose(f);
	
	f = fopen("./assets/space_2.txt", "r");
	while (!feof(f)){
		fgets(spaceship[1][spaceShipLength[1]++], 50, f);
		spaceship[1][spaceShipLength[1] - 1][strcspn(spaceship[1][spaceShipLength[1] - 1], "\n")] = 0;
	}
	fclose(f);
	
	f = fopen("./assets/space_3.txt", "r");
	while (!feof(f)){
		fgets(spaceship[2][spaceShipLength[2]++], 200, f);
		spaceship[2][spaceShipLength[2] - 1][strcspn(spaceship[2][spaceShipLength[2] - 1], "\n")] = 0;
	}
	fclose(f);
	
	f = fopen("./assets/space_4.txt", "r");
	while (!feof(f)){
		fgets(spaceship[3][spaceShipLength[3]++], 50, f);
		spaceship[3][spaceShipLength[3] - 1][strcspn(spaceship[3][spaceShipLength[3] - 1], "\n")] = 0;
	}
	fclose(f);
	
	mainMenu();
	return 0;
}
