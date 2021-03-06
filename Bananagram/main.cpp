//
//  main.cpp
//  bananagram
//
//  Created by Michah Lerner on 2/7/17.
//  Copyright © 2017 Michah Lerner. All rights reserved.
//

/*
 Linux:
    mkae -C Bananagram clobber
    make -C Bananagram
    Bananagram/banana -h
    Bananagram/banana -W IF,AFT,ALOES,TEAR,HIT,DO,FOE,BARD,DO,BASH,JOT -L BASEDHABTEDIALFOROO
 Xcode:
    xcodebuild -project Bananagram.xcodeproj -alltargets clean
    xcodebuild -project Bananagram.xcodeproj -alltargets
    build/Release/bananagram -h
    build/Release/bananagram -W IF,AFT,ALOES,TEAR,HIT,DO,FOE,BARD,DO,BASH,JOT -L BASEDHABTEDIALFOROO

    build/Release/bananagram -F TestData/google-10000-english-usa.txt -S -T -M 100000
 */

#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <numeric>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "simple_utils.h"
#include "Clocker.h"
#include "Dictionary.h"
#include "Board.h"
#include "Place.h"

extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

using std::cout;
using std::endl;

void usage(char *cmd) {
    cout << "Usage\n";
    cout << "  " << cmd << " -W cat,hat,hit,pat,sat,sit,vat -L SAITHAT -P\n";
    cout << "Options\n";
    cout << "   -?            ! help\n";
    cout << "   -h            ! help\n";
    cout << "   -d            ! debug\n";
    cout << "   -D ##         ! dimension\n";
    cout << "   -n ##         ! draw n tiles\n";
    cout << "   -F ##         ! dictionary filename\n";
    cout << "   -W word,...   ! comma separated words\n";
    cout << "   -L letters    ! initial letters\n";
    cout << "   -M maxresults ! max results (default 1000)\n";
    cout << "   -P            ! preserve order of dictionary (otherwise sorts, and maybe shuffle)\n";
    cout << "   -p            ! do not preserve order or dictionary [default]\n";
    cout << "   -S            ! shuffle words in dictionary [default]\n";
    cout << "   -s            ! sort words in dictionary by length and alphabet\n";
    cout << "   -T            ! randomize tiles\n";
    cout << "   -t            ! do not randomize tiles\n";
    cout << "   -o #          ! output options (001=normal|010=debug|100=machine)\n";
}

int dim=256,
    tile_count=21,
    output_options = 1;

long long max_results = 1000;

bool debug = false,
    preserve_order = false,
    shuffle_words = false,
    shuffle_t = false;

string dict_filename,
    dict_words,      // IF,AFT,ALOES,TEAR,HIT,DO,FOE,BARD,DO,BASH,JOT
    initial_letters; // BASEDHABTEDIALFOROO

int parseargs(int argc, char * const argv[]) {
    int ch;
    while ((ch = getopt(argc, argv, "?hdD:n:F:L:M:o:pPsStTW:")) != -1) {
        switch(ch) {
            case '?':
            case 'h': usage(argv[0]);
                return 1;
            case 'd': debug = true;
                break;
            case 'D': dim=atoi(optarg);
                break;
            case 'n': tile_count=atoi(optarg);
                break;
            case 'F': dict_filename.assign(optarg);
                break;
            case 'L': initial_letters.assign(optarg);  //TENUEFMDWRIISTLPONEOI
                break;
            case 'M': max_results=atoll(optarg);
                break;
            case 'o': output_options=atoi(optarg);
                break;
            case 'P': preserve_order = true;
                break;
            case 'p': preserve_order = false;
                break;
            case 'S': shuffle_words = true;
                break;
            case 's': shuffle_words = false;
                break;
            case 'T': shuffle_t = true;
                break;
            case 't': shuffle_t = false;
                break;
            case 'W': dict_words.assign(optarg);
                break;
            default:  cout << "Use: '" << argv[0] << "-?' for help\n";
                return -1;
        }
    }
    return 0;
}

void split(const string& s, char c,
           vector<string>& v) {
    string::size_type i = 0;
    string::size_type j = s.find(c);
    
    while (j != string::npos) {
        v.push_back(s.substr(i, j-i));
        i = ++j;
        j = s.find(c, j);
        
        if (j == string::npos)
            v.push_back(s.substr(i, s.length()));
    }
}

vector<char> mk_tiles() {
    vector<char> tiles(initialize_tiles());
    if(shuffle_t) {
        cout << "Randomizing unplaced tiles\n";
        shuffle_tiles(tiles);
    } else
        cout << "Not randomizing unplaced tiles\n";
    return tiles;
}

Dictionary mk_dictionary() {
    Dictionary dictionary(preserve_order,shuffle_words);
    if(!dict_filename.empty())
        dictionary.load(dict_filename);
    if(!dict_words.empty()) {
        vector<string> words;
        split(dict_words,',',words);
        dictionary.add_words(words);
    }
    return dictionary;
}

Board mk_board() {
    Dictionary dictionary(mk_dictionary());
    vector<char> tiles(mk_tiles());
    Board board(dictionary, tiles, dim, tile_count);
    board.debug = debug;
    board.output_options=output_options;
    board.max_results = max_results;
    
    if(initial_letters.empty())  {
        board.peel(tile_count);
    } else {
        board.peel(tochar(initial_letters));
    }
    board.enable_playable_words();
    
    return board;
}

void describe_rslt(const Board &board, const string& word, long long numunique, long long numresults) {
    numunique = board.numunique - numunique;
    numresults = board.numresults - numresults;
    string cnts(numresults==0?"none":(to_string(numunique)+"/"+to_string(numresults)));
    cout << "(unique/count)=" << left << setw(10) << cnts << " " << word << "\n";
}

int main(int argc,  char * const argv[]) {
    showargs(argc, argv);
    int rc = parseargs(argc, argv);
    if (rc!=0) return rc;
    cout << "debug=" << boolalpha << debug
        << "\ndictionary=" << dict_filename
        << "; dim=" << dim
        << "; initial_letters=" << initial_letters
        << "; max_results=" << max_results
        << "; output_options=" << output_options
        << "; preserve_order=" << preserve_order
        << "; shuffle_tiles=" << shuffle_t
        << "; shuffle_words=" << shuffle_words
        << "; tile_count=" << tile_count
        << endl;
    
    Board board(mk_board());
    string start_condition = board.show_unplaced();

    vector<string> top_words(board.dictionary.words.cbegin(), board.dictionary.words.cend());
    std::sort(top_words.begin(), top_words.end(), board.dictionary.cmp_longest);
    cout << "Use " << board.dictionary.words.size() << " of " << board.dictionary.all_words.size() <<  " words\n"
    << "Starting condition: " << start_condition << endl;
    
    int numanswers = 0;
    vector<const CharAtPos> uses;
    const Place start = Place(board.dim/2, board.dim/2, Place::Direction::horizontal);
    
    Clocker clock;
    cout << clock.readResetDetailed("NOTE: EXECUTION BEGINS ") << "\n\n";
    
    for(string & word : top_words) {
        if(board.insert_word(word, start, uses)) {
            long long numunique = board.numunique;
            long long numresults = board.numresults;
            deque<const Coord> expand_from;
            for(auto cap : uses)
                expand_from.push_back(cap.coord);
            bool normal = board.newsolve(expand_from, top_words);
            describe_rslt(board, word, numunique, numresults);
            if(!normal || board.numunique >= board.max_results)
                break;
            numanswers++;
            board.revert(uses);
        }
    }

    for(auto kv : board.board_counts) {
        cout << kv.second << " : " << kv.first;
    }
    
    cout << "#requested=" << board.max_results
    << "; #unique=" << board.boards_seen.size()
    << "; #computed=" << board.numresults
    << "; #starts=" << numanswers
    << " for " << start_condition << endl;
    
    cout << clock.readResetDetailed("NOTE: EXECUTION COMPLETE ") << endl;
    
    return 0;
}

