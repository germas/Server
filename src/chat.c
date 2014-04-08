#include "Server.h"
#include <math.h>

time_t gameStart;
status_t currentState;

void chatGameStart(){
    gameStart = time(NULL);
}

void sendChat(PKT_CHAT* chat, teamNo_t teams[MAX_PLAYERS], SOCKET outswitch){

    OUTMASK outM;
    OUT_ZERO(outM);
    int i;
    packet_t type = 4;
    time_t currentTime;
    double decryptLvl;
    double timeDiff;

    // Send server message
    if(chat->sendingPlayer == MAX_PLAYERS){
        OUT_SETALL(outM);
        write(outswitch, &type, sizeof(packet_t));
        write(outswitch, chat, netPacketSizes[type]);
        write(outswitch, &outM, sizeof(OUTMASK));
        return;
    }

    // Write plain to player's team
    for(i = 0; i < MAX_PLAYERS; ++i){
        if(i != chat->sendingPlayer && teams[i] == teams[chat->sendingPlayer]){
            OUT_SET(outM, i);
        }
    }
    write(outswitch, &type, sizeof(packet_t));
    write(outswitch, chat, netPacketSizes[type]);
    write(outswitch, &outM, sizeof(OUTMASK));

    // If robber chat, send encrypted to cops

    // determine % decryped
    OUT_ZERO(outM);
    currentTime = time(NULL);
    timeDiff = (currentTime - gameStart);
    decryptLvl = (timeDiff / CHAT_DECRYPT_TIME);
    decryptLvl *= 100;
    // cap at 100%
    decryptLvl = (decryptLvl > 100) ? 100 : decryptLvl;

    if(teams[chat->sendingPlayer] == TEAM_ROBBERS){
        for(i = 0; i < MAX_MESSAGE && chat->message[i] != 0; ++i){

            if(chat->message[i] != ' ' && rand() % 100 > decryptLvl){
                // random asc 33-126
                chat->message[i] = (rand() % 93) + 33;
                //chat->message[i] = '*';
            }
        }


        // Write encrypted to cops
        for(i = 0; i < MAX_PLAYERS; ++i){
            if(teams[i] == TEAM_COPS){
                OUT_SET(outM, i);
            }
        }
        write(outswitch, &type, sizeof(packet_t));
        write(outswitch, chat, netPacketSizes[type]);
        write(outswitch, &outM, sizeof(OUTMASK));

    }
}
