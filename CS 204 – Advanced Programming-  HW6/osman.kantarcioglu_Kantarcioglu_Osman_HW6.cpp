#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip> // for put_time
#include <random>
/******************************************************************************
Sabanci  University CS204 Advanced Programming
Coded by: Osman Kantarcioglu    -   30891   - Section:B2
******************************************************************************/
using namespace std;

int targetNumber,winner;
bool gameEnd ;
bool roundEnd ;
bool print1 = false;
int idxRoundWinner;
vector<int> playerScores;
mutex coutMtx, gameMtx;
/******************************************************************************
//This function is used to diplay current time/ Taken from recitation
******************************************************************************/
void currentTimeDisplayer() {
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* ptm = new struct tm;
    localtime_s(ptm, &tt);
    cout << put_time(ptm, "%H:%M:%S") << " ";
}
/******************************************************************************
//This function is used to generate random int/ Taken from recitation
******************************************************************************/
int random_range(const int& min, const int& max) {
    static thread_local mt19937 generator(random_device{}());
    uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}
/******************************************************************************
//this is the host thread function
******************************************************************************/
void hostThread(int randRangeMin, int randRangeMax, int numRound) {
    gameEnd = false;
    int currentRound = 1;
    while (currentRound <= numRound) {
        // Start a new round
        roundEnd = false;
        targetNumber = random_range(randRangeMin, randRangeMax);
        coutMtx.lock();
        if (currentRound == 1) {//round print message

            cout << endl << endl;
            cout << "---------------------------------------------------" << endl;
            cout << "Game started at: ";
            currentTimeDisplayer();
            cout << endl;
            cout << "Round 1 will start 3 seconds later" << endl << endl;
            cout << "Target number is " << targetNumber << endl << endl;
            coutMtx.unlock();
            // Wait for 3 seconds before starting the first round
        }
        else { //also round print message when different than 1
            cout << endl;
            cout << "---------------------------------------------------" << endl;
            cout << "Round " << currentRound << " started at: ";
            currentTimeDisplayer();
            cout << endl;
            cout << "Target number is " << targetNumber << endl << endl;
            coutMtx.unlock();
        }
        // Wait for round to end
        while (!roundEnd) {
            this_thread::sleep_for(chrono::milliseconds (1));
        }
        playerScores[winner]++;
        currentRound++;
    }
    // Signal game end
    gameMtx.lock();
    gameEnd = true;
    gameMtx.unlock();
}
/******************************************************************************
//this is the player thread function
******************************************************************************/
void playerThread(int randRangeMin, int randRangeMax, int playerId,tm*ptm) {
    this_thread::sleep_until(chrono::system_clock::from_time_t(mktime(ptm)));
    bool finish = false;
    while (finish == false) {   //it will continue until gameend return true
        if (gameEnd) {
            finish = true;
        }
        int guess = random_range(randRangeMin, randRangeMax); //take guess
        time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
        ptm = new struct tm;
        localtime_s(ptm, &tt);
        gameMtx.lock();
        if (!roundEnd) {
            if (guess == targetNumber) {//print guess message
                coutMtx.lock();
                cout << "Player with id " << playerId << " guessed " << guess << " correctly at: "<<put_time(ptm,"%X")<<endl;
                winner = playerId;
                roundEnd = true;
                coutMtx.unlock();
            }
            else {//print guess message
                coutMtx.lock();
                cout << "Player with id " << playerId << " guessed " << guess << " incorrectly at: "<<put_time(ptm,"%X")<<endl;
                coutMtx.unlock();
            }
        }
        gameMtx.unlock();
        this_thread::sleep_for(chrono::seconds(1));
    }
}

int main() {
    // Required inputs have been taken
    int numPlayer, numRound, randRangeMin, randRangeMax;
    cout << "Please enter number of players:" << endl;
    cin >> numPlayer; //player input taken
    while (numPlayer < 1) {
        cout << "Number of players cannot be lower than 1!" << endl;
        cout << "Please enter number of players:" << endl;
        cin >> numPlayer;
    }
    cout << "Please enter number of rounds:" << endl;
    cin >> numRound; //round input taken
    while (numRound < 1) {
        cout << "Number of rounds cannot be lower than 1!" << endl;
        cout << "Please enter number of rounds:" << endl;
        cin >> numRound;
    }
    cout << "Please enter the randomization range:" << endl;
    cin >> randRangeMin >> randRangeMax;
    while (randRangeMin > randRangeMax) {
        cout << "Lower bound has to be smaller than or equal to higher bound" << endl;
        cout << "Please enter the randomization range:" << endl;
        cin >> randRangeMin >> randRangeMax; //range input taken
    }
    playerScores.resize(numPlayer, 0);
    // Threads are going to be initialized.
    // 1 thread for host and for players
    // Thread for host has been initialized
    thread host(hostThread, randRangeMin, randRangeMax, numRound);
    time_t tt = chrono::system_clock::to_time_t(chrono::system_clock::now());
    struct tm* ptm = new struct tm;
    localtime_s(ptm, &tt);
    ptm->tm_sec += 3;
    vector<thread> threadList;
    // Threads for players have been initialized one by one
    for (int idx = 0; idx < numPlayer; ++idx) {
        threadList.push_back(thread(playerThread, randRangeMin, randRangeMax, idx,ptm));
    }
    for (int idx = 0; idx < threadList.size(); ++idx) {
        threadList[idx].join();
    }
    host.join();
    coutMtx.lock();
    cout << endl;
    cout << "Game is over!" << endl;
    cout << "Leaderboard: " << endl;
    for (int idx = 0; idx < playerScores.size(); ++idx) {
        cout << "Player " << idx << " has won " << playerScores[idx] << " times" << endl;
    }
    coutMtx.unlock();
    return 0;
}