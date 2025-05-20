#include <iostream>
#include <random>
#include <time.h>

#include <Eigen/Dense>
#include <Eigen/LU>

bool simulatRandNxNGame(uint N){
    Eigen::MatrixXf board(N, N);

    uint pieceCount[2] {0, 0};
    uint roundingTerm {static_cast<uint>((N*N % 2 == 0) ? 0 : 1)}; // player 1 has ceil(N/2) and player 0 has floor(N/2)

    uint maxPieceCount[2] {static_cast<uint>(N*N/2), static_cast<uint>(N*N/2) + roundingTerm};

    //std::cout << maxPieceCount[0] << ", " << maxPieceCount[1] << "\n";

    // gen rand board state
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    for(uint i = 0; i < N; ++i){
        for(uint j = 0; j < N; ++j){
            int randInd = dis(gen);

            if(pieceCount[randInd] >= maxPieceCount[randInd]){
                //std::cout << randInd << " has reached the max moves" << "\n";
                randInd = !randInd;
            }
            //std::cout << pieceCount[randInd] << " < " << maxPieceCount[randInd] << "\n";
            board(i,j) = randInd;
            pieceCount[randInd]++;

        }
    }

    //std::cout << "matrix: \n"<< board << "\n";

    return board.determinant() != 0;
}

void simulateNxNCaseKTimes(uint N, int K){
    int playerOneWins {};
    int numOfRounds {K};

    std::cout << "simulating " << N << "x" << N << " case " << K << " times.\n";

    for(int i = 0; i < numOfRounds; ++i){
        playerOneWins += simulatRandNxNGame(N);
    }

    float playerOneWinPerc {static_cast<float>(playerOneWins)/static_cast<float>(numOfRounds)};
    std::cout << "player1 won " << playerOneWins << " times. Which is " << playerOneWinPerc*100 << "% of the time\n";
    std::cout << "player0 won " << numOfRounds-playerOneWins << " times. Which is " << (1-playerOneWinPerc)*100 << "% of the time\n";
}


// 3x3: 46.86% 0
int main(){

    clock_t tStart {clock()};
    simulateNxNCaseKTimes(12, 1000000000);
    printf("Time taken: %.2fs\n", static_cast<double>(clock() - tStart)/CLOCKS_PER_SEC);

    /*
    int playerOneWins {};
    int numOfRounds {100};

    std::cout << "simulating " << numOfRounds << " games.\n";
    std::cout << "[";
    for(int i = 0; i < numOfRounds; ++i){
        playerOneWins += simulateRand3x3Play();
        if(i%(numOfRounds/10) == 0) std::cout << "." << std::flush;
    }
    std::cout << "]\n";

    float playerOneWinPerc {static_cast<float>(playerOneWins)/static_cast<float>(numOfRounds)};
    std::cout << "player1 won " << playerOneWins << " times. Which is " << playerOneWinPerc*100 << "% of the time\n";
    std::cout << "player0 won " << numOfRounds-playerOneWins << " times. Which is " << (1-playerOneWinPerc)*100 << "% of the time\n";
    return 0;
    */
}
