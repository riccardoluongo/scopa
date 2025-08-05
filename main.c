#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

struct carta{
    char seme[8];
    uint8_t val;
};

struct player{
    uint8_t scope;
    uint8_t lunga;
    uint8_t denari;
    uint8_t settebello;
    uint8_t settanta;
    uint8_t pos_carte_prese;
    struct carta mazzo[4];
    struct carta carte_prese[40];
};

struct carta mazzo[40];
struct carta tavolo[40];
struct carta null_card = {.val = 0};
struct carta carta_presa = {.val = 99};
struct player player;
struct player cpu;
struct carta prese[15];
uint8_t trovata = 0; //usata per trova(), indica se la carta è stata trovata nel tavolo
uint8_t conta_ris = 0;
uint8_t min_len = 15;
int8_t posizione_mazzo = 39;
char * semi[4] = {"Spade", "Coppe", "Denari", "Bastoni"};
unsigned short int turno = 1;

void riempi_mazzo(){
    uint8_t k = 0;
    for(uint8_t i = 0; i < 4; i++){
        for(uint8_t j = 1; j <= 10; j++){
            strcpy(mazzo[k].seme, semi[i]);
            mazzo[k++].val = j;
        }
    }
}

size_t swap(struct carta arr[], size_t pos1, size_t pos2){
    struct carta temp = arr[pos1];
    arr[pos1] = arr[pos2];
    arr[pos2] = temp;

    return pos2;
}

void mischia(){
    posizione_mazzo = 39;

    for(uint8_t i = 0; i < 40; i++){
        swap(mazzo, rand() % 40, rand() % 40);
    }
}

void stampa_mazzo(struct carta mazzo[]){
    for(uint8_t i = 0; mazzo[i].val > 0; i++)
        if (mazzo[i].val != 99) printf("%s %d\n", mazzo[i].seme, mazzo[i].val);
}

void prendi_carte(uint8_t n, struct carta mazzo_temp[]){
    uint8_t i;
    for(i = 0; i < n; i++)
        mazzo_temp[i] = mazzo[posizione_mazzo--];
    mazzo_temp[i] = null_card;
}

uint8_t trova_fine_mazzo(struct carta mazzo[]){
    uint8_t i = 0;

    while(mazzo[i].val != 0)
        i++;
    return i;
}

int confronta_valore(const void * a, const void * b){
    return(((struct carta *)a) -> val - ((struct carta *)b) -> val);
}

uint8_t trova(struct carta arr[], uint8_t size, uint8_t target, short int index, struct carta current[], uint8_t currLen, uint8_t sum) {
    if (sum > target) return 0;
    if (sum == target) {
        if (currLen < min_len || conta_ris++ == 0) {
            min_len = currLen;
            uint8_t i;

            for(i = 0; i < currLen; i++)
                prese[i] = current[i];
            prese[i] = null_card;
        }

        trovata = 1;
        return 1;
    }

    for (uint8_t i = index; i < size; i++) {
        if (i > index && arr[i].val == arr[i-1].val) continue;

        current[currLen] = arr[i];
        trova(arr, size, target, i + 1, current, currLen + 1, sum + arr[i].val);
    }
    return trovata ? 1 : 0;
}

short int binsearch(struct carta arr[], uint8_t low, uint8_t high, uint8_t x){
    while (low <= high){
        int mid = low + (high - low) / 2;

        if (arr[mid].val == x)
            return mid;

        if (arr[mid].val < x)
            low = mid + 1;
        else
            high = mid - 1;
    }

    return -1;
}

uint8_t conta_carte(struct carta mazzo[]){
    uint8_t num = 0, i = 0;

    while(mazzo[i].val > 0)
        if(mazzo[i++].val != 99) num++;
    return num;
}

uint8_t gioca(struct player * player, uint8_t carta){
    uint8_t fine_tavolo = trova_fine_mazzo(tavolo), dim_carta = sizeof(struct carta), fine_prese;
    trovata = conta_ris = 0;
    struct carta prese_temp[15];

    qsort(tavolo, fine_tavolo, dim_carta, confronta_valore);

    if(trova(tavolo, dim_carta, player->mazzo[carta].val, 0, prese_temp, 0, 0)){
        fine_prese = trova_fine_mazzo(prese);

        for(uint8_t i = 0; i < fine_prese; i++){
            qsort(tavolo, fine_tavolo, dim_carta, confronta_valore);

            short int j = binsearch(tavolo, 0, fine_tavolo, prese[i].val);
            if(j >= 0){
                player->carte_prese[player->pos_carte_prese++] = prese[i];
                tavolo[j] = carta_presa;
            }
        }
        player->carte_prese[player->pos_carte_prese++] = player->mazzo[carta];
        player->mazzo[carta] = carta_presa;

        if(conta_carte(tavolo) == 0){
            player->scope++;
            return 2;
        }
        return 1;
    } else{
        tavolo[fine_tavolo++] = player->mazzo[carta];
        player->mazzo[carta] = carta_presa;
        tavolo[fine_tavolo] = null_card;
        return 0;
    }
}

uint8_t punteggio_totale(struct player * player){
    return player->scope + player->settanta + player->settebello + player->lunga;
}

void init(){
    riempi_mazzo();

    uint8_t num = 0, i = 0;
    do{
        mischia();
        prendi_carte(3, player.mazzo);
        prendi_carte(3, cpu.mazzo);
        prendi_carte(4, tavolo);

        while(i < 3)
            if(tavolo[i++].val == 10) num++;
    } while(num >= 3);
}

void calcola_punti(){
    uint8_t denari_player = 0, denari_cpu = 0, trovato = 0, sette_player = 0, sette_cpu = 0, punti_cpu, punti_player, vince_player;
    static uint8_t primo_player = 2;

    if(player.pos_carte_prese > cpu.pos_carte_prese)
        player.lunga++;
    else if(cpu.pos_carte_prese > player.pos_carte_prese)
        cpu.lunga++;

    for(int8_t i = player.pos_carte_prese; i >= 0; i--){
        if(strcmp(player.carte_prese[i].seme, "Denari") == 0) denari_player++;
        if(player.carte_prese[i].val == 7) sette_player ++;
    }
    for(int8_t i = cpu.pos_carte_prese; i >= 0; i--){
        if(strcmp(cpu.carte_prese[i].seme, "Denari") == 0) denari_cpu++;
        if(cpu.carte_prese[i].val == 7) sette_cpu ++;
    }
    if(denari_player > denari_cpu)
        player.denari++;
    else if(denari_cpu > denari_player)
        cpu.denari++;

    for(int8_t i = player.pos_carte_prese; i >= 0; i--)
        if(strcmp(player.carte_prese[i].seme, "Denari") == 0 && player.carte_prese[i].val == 7){
            player.settebello++;
            trovato = 1;
            break;
        }
    if(!trovato)
        for(int8_t i = cpu.pos_carte_prese; i >= 0; i--)
            if(strcmp(cpu.carte_prese[i].seme, "Denari") == 0 && cpu.carte_prese[i].val == 7){
                cpu.settebello++;
                break;
            }

    if(sette_player > sette_cpu)
        player.settanta++;
    if(sette_cpu >sette_player)
        cpu.settanta++;

    if((punti_player = punteggio_totale(&player)) >= 11 || (punti_cpu = punteggio_totale(&cpu)) >= 11){
        uint8_t scelta, vince_player;
        vince_player = punti_player > punti_cpu;

        printf("\e[1;1H\e[2Jpartita terminata!\nvincitore: %s (%d-%d)\n\npunteggi giocatore:\n Scope: %d\tSettanta: %d\n Settebello: %d\tCarte a lungo: %d\n\npunteggi CPU:\n Scope: %d\tSettanta: %d\n Settebello: %d\tCarte a lungo: %d\n\n1) gioca di nuovo 0) esci\n> ", vince_player  ? "giocatore" : "CPU", vince_player ? punti_player : punti_cpu, vince_player ? punti_cpu : punti_player, player.scope, player.settanta, player.settebello, player.lunga, cpu.scope, cpu.settanta, cpu.settebello, cpu.lunga);
        do
            scanf("%d", &scelta);
        while(scelta != 1 && scelta != 0);

        if(scelta == 0)
            exit(0);

        posizione_mazzo = 39;
        tavolo[0] = null_card;
        player.pos_carte_prese = cpu.pos_carte_prese = 0;
        player.denari = player.lunga = player.scope = player.settanta = player.settebello = 0;
        cpu.denari = cpu.lunga = cpu.scope = cpu.settanta = cpu.settebello = 0;
        return;
    }

    printf("\e[1;1H\e[2Jmano terminata!\npunteggi giocatore:\n Scope: %d\tSettanta: %d\n Settebello: %d\tCarte a lungo: %d\n\npunteggi CPU:\n Scope: %d\tSettanta: %d\n Settebello: %d\tCarte a lungo: %d\n\npunti totali giocatore: %d\tpunti totali CPU: %d\npremi Enter per continuare...", player.scope, player.settanta, player.settebello, player.lunga, cpu.scope, cpu.settanta, cpu.settebello, cpu.lunga, punteggio_totale(&player), punteggio_totale(&cpu));

    char c;
    while((c = getchar()) != '\n');
    getchar();

    posizione_mazzo = 39;
    tavolo[0] = null_card;
    turno = primo_player == 1 ? primo_player++ : primo_player--;
    player.pos_carte_prese = cpu.pos_carte_prese = 0;
}

int main(){
    srand(time(NULL));

    uint8_t input, scelta;

    printf("Scopa v0.1 by Riccardo Luongo\n\n1) nuova partita\n0) esci\n\n> ");
    do
        scanf("%d", &scelta);
    while(scelta != 1 && scelta != 0);

    switch(scelta){ //using switch here to prepare for more options in the menu
        case 1:
            break;
        case 0:
            return 0;
    }

    loop:
    printf("\e[1;1H\e[2J");
    init();

    do{
        if(turno++ % 2 != 0){
            if(conta_carte(player.mazzo) == 0){
                if(posizione_mazzo < 0){
                    calcola_punti();
                    goto loop;
                }

                printf("-> distribuendo le carte al giocatore...\n");
                prendi_carte(3, player.mazzo);
            }

            printf("\ncarte sul tavolo:\n");
            for(int i = 0; tavolo[i].val != 0; i++)
                if(tavolo[i].val != 99)
                    printf(" %d %s ", tavolo[i].val, tavolo[i].seme);
            putchar('\n');

            printf("mazzo:\n");
            for(int i = 0; player.mazzo[i].val != 0; i++)
                if(player.mazzo[i].val != 99)
                    printf(" (%d) %d %s", i+1, player.mazzo[i].val, player.mazzo[i].seme);
            putchar('\n'), putchar('\n');

            do{
                printf("carta da giocare: ");
                scanf("%d", &input);

                if(input >= 1 && input-- <= 3){
                    if(player.mazzo[input].val != 99){
                        if(gioca(&player, input) == 2 && turno != 38){
                            printf("scopa! Premi Enter per continuare...");

                            char c;
                            while((c = getchar()) != '\n');
                            getchar();
                        }

                        printf("\e[1;1H\e[2J");
                        break;
                    }
                    else
                        printf("-> carta gia' giocata, riprova!\n\n");
                } else{
                    printf("-> input non valido, riprova!\n\n");
                    char c;
                    while((c = getchar()) != '\n');
                }
            } while(1);
        } else{
            if(conta_carte(player.mazzo) == 0){
                if(posizione_mazzo < 0){
                    calcola_punti();
                    goto loop;
                }

                printf("-> distribuendo le carte alla CPU...\n");
                prendi_carte(3, cpu.mazzo);
            }

            struct carta prese_temp[15];
            uint8_t trovata = 0;

            for(uint8_t i = 0; i <= 2; i++)
                if(cpu.mazzo[i].val != 99)
                    if(trova(tavolo, sizeof(struct carta), cpu.mazzo[i].val, 0, prese_temp, 0, 0)){
                        printf("-> la CPU gioca la carta: %d %s\n", cpu.mazzo[i].val, cpu.mazzo[i].seme);
                        if(gioca(&cpu, i) == 2 && (posizione_mazzo > 0 || conta_carte(cpu.mazzo) > 1)){
                            printf("-> la CPU ha fatto scopa, premi Enter per continuare...");

                            char c;
                            while((c = getchar()) != '\n');
                            getchar();
                        }

                        trovata = 1;
                        break;
                    }
            if(!trovata){
                uint8_t carta = 0;

                while(cpu.mazzo[carta].val == 99)
                    carta++;

                printf("-> la CPU gioca la carta: %d %s\n", cpu.mazzo[carta].val, cpu.mazzo[carta].seme);
                gioca(&cpu, carta);
            }
        }
    } while(1);
}
