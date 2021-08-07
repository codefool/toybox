//============================================================================
// Name        : mastermind.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <vector>

typedef std::vector<short> Guess;
typedef std::vector<Guess> GuessList;

std::uint8_t COLORS    = 6;
std::uint8_t POSITIONS = 4;


// implementation of Knuth's codebreaker algorithm described in his 1976 paper
// "The Computer As Master Mind", Journal of Recreational Mathematics, Vol 9(1), 1976-77
// https://www.cs.uni.edu/~wallingf/teaching/cs3530/resources/knuth-mastermind.pdf
// Retrieved 5.24.2020
//
// 1. Create a set of all possible solutions for a given number of slots n
// 2. Submit a test solution with two seed values, e.g, (1,1,2,2), this results in a
//    number of "black" pegs and "white" pegs, where:
//    - a black peg indicates a correct guess in the correct position, and
//    - a white peg indicates a correct guess in the wrong position, and
//    - w + b <= n
//
//    Note: It is impossible to have n-1 black hits and 1 white hit
//
// 3. If b = n, the game is solved and the algorithm terminates
// 4. Remove all members of the set that DO NOT fit the response from step 2 AS IF
//    the guess were the code
// 5. Perform minmax procedure for all possible codes on remaining S.

// determine the black/white peg count for the given pattern vs the given solution
//
// Return the "score" as the white-peg count * 100 + black-peg count.
// e.g., 2 white pegs, 2 black is 202. Since we're only ever concerned of a score
// that indicates a win (i.e. black-peg-count == POSITIONS), then we just need
// to return a score that differentiates it from all other possible scores. This
// scheme allows integer comparisons and indexes, and direct test for a win condition.

int peg_score(Guess pattern, Guess guess) {
	int black_cnt{0};
	int white_cnt{0};
	auto pitr = pattern.begin();
	auto gitr = guess.begin();

	// first, determine if any elements of the guess occur in
	// the same positions as the pattern. These are black-peg's.
	// if a match if found, remove from both patterns.
	while(pitr != pattern.end()) {
		if(*pitr == *gitr) {
			black_cnt++;
			pitr = pattern.erase(pitr);
			gitr = guess.erase(gitr);
		} else {
			++pitr;
			++gitr;
		}
	}

	// If the game is won, then pattern will be empty. Otherwise,
	// for each remaining entry in pattern, determine if it exists
	// in guess. These are white-peg's.
	for(pitr = pattern.begin(); pitr != pattern.end(); ++pitr) {
		gitr = std::find(guess.begin(), guess.end(),*pitr);
		if(gitr != guess.end()) {
			white_cnt++;
			guess.erase(gitr);
		}
	}

	return (white_cnt * 100) + black_cnt;
}

void genSeries(GuessList &ser, int n, int m, Guess base) {
	if(m == 0) {
		ser.insert(ser.end(),base);
	} else {
		base.push_back(0);
		for(int i = 0; i < n; i++) {
			base.back() = i;
			genSeries(ser, n, m-1, base);
		}
	}
}

// generate a possible solution set from an input set of [0..n)
// of m elements each.
GuessList solutionSet(int n, int m) {
	GuessList ret;
	Guess seed;
	genSeries(ret, n, m, seed);
	return ret;
}

std::ostream& operator<<(std::ostream& os, Guess const &obj) {
	for(auto itr : obj) {
		os << (int)itr;
	}
	return os;
}

void prune(GuessList &ser, Guess const &guess, int const s) {
	auto itr = ser.begin();
	while(itr != ser.end()) {
		auto result = peg_score(guess, *itr);
		if(result != s) {
			itr = ser.erase(itr);
		} else {
			++itr;
		}
	}
}

GuessList minimax(GuessList &master, GuessList& cand) {

    std::map<int, int> scoreCount;
    std::map<Guess, int> scores;
    GuessList nextGuesses;

    // this is an O(n*m), but no other way to do this.
    // given that n and m are at most COLORS^POSITIONS
    // this isn't super concerning. It would be interesting
    // to see if this could be reduced somehow.
    for(Guess mitr : master) {
    	// for each possible answer (master), compare it to
    	// all the remaining answers (cand) and group them
    	// by peg score.
    	for(Guess gitr : cand) {
    		auto score = peg_score(mitr,gitr);
    		if(scoreCount.count(score) > 0) {
    			scoreCount.at(score)++;
    		} else {
    			scoreCount.emplace(score,1);
    		}
    	}

    	// now find the the most popular peg score
    	// in the case of ties this takes the first
    	// one, but it's of no consequence to the
    	// algorithm.
        int max = 0;
        for (auto s : scoreCount) {
            if (s.second > max) {
                max = s.second;
            }
        }

        // remember this guess and the number of
        // candidate answers that can be excluded if
        // we use this guess.
        scores.emplace(mitr, max);
        scoreCount.clear();
    }

    // find the minimum peg score match count
    int min = std::numeric_limits<int>::max();
	for (auto s : scores) {
	   if (s.second < min) {
		   min = s.second;
	   }
	}

    // Find the guesses with the least number of
    // peg score matches. This the pool of best
	// choice for the next guess.
	for (auto s : scores) {
		if (s.second == min) {
			nextGuesses.push_back(s.first);
		}
	}

    return nextGuesses;
}


Guess get_next_guess(GuessList next_guess, GuessList cand, GuessList allGuess) {
	Guess nextGuess;

	for(Guess g : next_guess) {
		if( std::find(cand.begin(), cand.end(), g) != cand.end()) {
			return g;
		}
	}
	for(Guess g : next_guess) {
		if( std::find(allGuess.begin(), allGuess.end(), g) != allGuess.end()) {
			return g;
		}
	}
	std::cout << "No valid next guess found\n";
	return nextGuess;
}

void erase(GuessList& list, Guess const &guess) {
	auto itr = std::find(list.begin(),list.end(),guess);
	if(itr != list.end())
		list.erase(itr);
}

std::ostream& operator<<(std::ostream& os, Guess &guess) {
	for(auto x : guess)
		os << x;
	return os;
}

// search a string to see if all chars are printable. If the
// only non-printable character is the last, and it is NULL,
// consider it printable.
#include <cctype>
bool printable(std::string& s) {
	auto it = s.crbegin();
	if (*it != '\0' && !std::isprint(*it)) {
		// last char is not NULL terminator and not otherwise
		// printable, so fail
		return false;
	}

	for (++it; it != s.crend(); ++it)
		if (!std::isprint(*it))
			return false;

	return true;
}


int main() {
	GuessList all_solutions = solutionSet(COLORS,POSITIONS); // 6 colors, 4 positions
	GuessList guesses;
	guesses.insert(guesses.end(), all_solutions.begin(), all_solutions.end());

	Guess secret{1,2,3,4};
	Guess guess{0,0,1,1};
	int cnt{0};

	while(true) {
		std::cout << "Try " << cnt++ << ' ' << secret << ' ' << guess << std::endl;
		//Remove currentGuess from possible solutions
		erase(all_solutions, guess);
		erase(guesses, guess);

		auto score = peg_score(secret,guess);

		if(score == POSITIONS) {
			std::cout << "Game won!\n";
			break;
		}

		prune(guesses, guess, score);
		auto next_guess = minimax(all_solutions,guesses);
		guess = get_next_guess(next_guess, guesses, all_solutions);
	}

	std::string s{"hello, world"};
	std::cout << s << ':' << printable(s) << std::endl;
	s.back() = '\0';
	std::cout << s << ':' << printable(s) << std::endl;
	s = "\0\0\1this\0x7f\0xffis\0x1fa binary string\0x7f";
	std::cout << s << ':' << printable(s) << std::endl;

	return 0;
}
