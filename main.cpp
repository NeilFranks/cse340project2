/*
 * Copyright (C) Mohsen Zohrevandi, 2017
 *
 * Do not share this file with anyone
 */
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <set>
#include <algorithm>
#include "lexer.h"

using namespace std;

LexicalAnalyzer lex;

struct RHS{
    string tok;
    RHS * next;
};

struct rule{
    string non;
    RHS * rhs;
    rule * next;
};

//method to parse RHS using recursion. ends by consuming a HASH
bool parseRHS(RHS * r, Token t){
    if(t.token_type == ID){
        r->tok.assign(t.lexeme);
        //r = temp;
        return parseRHS(r->next = new RHS, lex.GetToken());
    }
    else if(t.token_type == HASH){
        return true;
    }
    else
        return false; //encountered invalid char; error
}

bool parseRule(rule * r, Token t){
    if(t.token_type == ID){
        r->non = t.lexeme;
        t = lex.GetToken();
        if(t.token_type == ARROW){
            if(!parseRHS(r->rhs = new RHS, lex.GetToken())){
                cout << "Error: tokens on RHS must either be ID or a HASH\n";
                return 1;
            }
        }else{
            cout << "Error: rules must have an arrow as their second token\n";
            return 1;
        }

        return parseRule(r->next = new rule, lex.GetToken());
    }
    else if(t.token_type == DOUBLEHASH){
        return true;
    }
    else
        return false;
}

pair<vector<string>, vector<string> > sortTokens(rule * r){
    vector<string> non;
    vector<string> term;

    //get set of non terminals
    set<string> nonT;

    rule * rIt = r;
    while(rIt != NULL && rIt->next != NULL){
        //add non if not already added
        nonT.insert(rIt->non);

        rIt = rIt->next;
    }

    //get ordered set of non terminals and of everything in RHS's
    rule * rIt2 = r;
    while(rIt2 != NULL && rIt2->next != NULL){
        if(find(nonT.begin(), nonT.end(), rIt2->non) != nonT.end() && find(non.begin(), non.end(), rIt2->non) == non.end())
        //token is a nonterminal and not already added
            non.push_back(rIt2->non);

        RHS * rhsIt = rIt2->rhs;
        while(rhsIt != NULL && rhsIt->next != NULL){
            //add token to non vector
            string symbol = rhsIt->tok;
            if(find(nonT.begin(), nonT.end(),symbol) != nonT.end() && find(non.begin(), non.end(), symbol) == non.end())
                //token is a nonterminal and not already added
                non.push_back(symbol);

            else if(find(nonT.begin(), nonT.end(),symbol) == nonT.end() && find(term.begin(), term.end(), symbol) == term.end())
                //token is a non terminal and not already added
                term.push_back(symbol);

            rhsIt = rhsIt->next;
        }
        rIt2 = rIt2->next;
    }


    return make_pair(non, term);
}

void printAll(rule * r){ //prints nonterminals and terminals in order
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(r);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    //print non terms in order
    vector<string>::iterator nonIt;
    for(nonIt = non.begin(); nonIt != non.end(); nonIt++)
        cout << *nonIt << " ";

    //print terms in order
    vector<string>::iterator termIt;
    for(termIt = term.begin(); termIt != term.end(); termIt++)
        cout << *termIt << " ";

}
 vector<rule *> prune(rule * r){
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(r);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    //find generative symbols
    set<string> gen;
    gen.insert(term.begin(), term.end());

    vector<rule*> newRules; //will be new list of rules
    string startSym = r->non;

    bool changed = true;

    while(changed) {
        changed = false;

        rule * it = r;
        while (it != NULL && it->next != NULL) {
            bool allGen = true; //keep track of if RHS is all generative symbols
            RHS *rhsIt = it->rhs;
            while (rhsIt != NULL && rhsIt->next != NULL) {
                string symbol = rhsIt->tok;
                if(find(gen.begin(), gen.end(), symbol) == gen.end()){ //symbol is not generative
                    allGen = false;
                }
                rhsIt = rhsIt->next;
            }
            if (allGen){
                if(find(gen.begin(), gen.end(), it->non) == gen.end()){ //symbol has not been added already
                    gen.insert(it->non);
                    changed = true;
                }
            }

            it = it->next;
        }
    }

    //get rid of all rules with non-generative symbols
    rule * it1 = r;
    while(it1 != NULL && it1->next != NULL){
        bool allGen = true; //assume all symbols in rule are generative
        if(find(gen.begin(), gen.end(), it1->non) == gen.end())//LHS not generative
            allGen = false;
        else{
            RHS * rhsIt = it1->rhs;
            while(rhsIt != NULL && rhsIt->next != NULL){
                if(find(gen.begin(), gen.end(), rhsIt->tok) == gen.end())//RHS symbol not generative
                    allGen = false;
                rhsIt = rhsIt->next;
            }
        }

        if(allGen)
            newRules.push_back(it1);

        it1 = it1->next;
    }

    //find all reachable symbols
    set<string> reach;
    vector<rule *> newRules2;

    if(!newRules.empty() && newRules.at(0)->non.compare(startSym) == 0){ //make sure start symbol wasnt useless
         reach.insert(newRules.at(0)->non);
         newRules2.push_back(newRules.at(0)); //start symbol is reachable

         bool changed2 = true;
         while (changed2) {
             changed2 = false;
             //increment through all reachable rules
             for (int pos = 0; pos < newRules.size(); pos++) {
                 if (find(reach.begin(), reach.end(), newRules.at(pos)->non) != reach.end()) {
                     //rule is reachable
                     RHS *rhsIt = newRules.at(pos)->rhs; //all nonterms in RHS are reachable
                     while (rhsIt != NULL && rhsIt->next != NULL) {
                         if (find(non.begin(), non.end(), rhsIt->tok) != non.end() &&
                             find(reach.begin(), reach.end(), rhsIt->tok) == reach.end()) {
                             //token is a nonterminal and not already added
                             reach.insert(rhsIt->tok);
                             changed2 = true;
                         }
                         rhsIt = rhsIt->next;
                     }
                 }
             }
         }

         //get rid of all rules with non-reachable symbols
         for (int it3 = 1; it3 < newRules.size(); it3++) {
             if (find(reach.begin(), reach.end(), newRules.at(it3)->non) != reach.end())
                 //LHS is reachable
                 newRules2.push_back(newRules.at(it3));
         }


     }

     return newRules2;
}

void printVec(vector<rule *> v){
    if(!v.empty())
    for(int i = 0; i < v.size(); i++){
        cout << v.at(i)->non << " ->";
        RHS * rhsIt = v.at(i)->rhs;
        if(rhsIt->tok.empty())
            cout << " #";
        else
            while(rhsIt != NULL && rhsIt->next != NULL){
                cout << " " << rhsIt->tok;
                rhsIt = rhsIt->next;
            }
        cout << endl;
    }
}

vector<set <string> > first(rule * head){
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(head);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    vector<set<string> > firstSets = vector<set<string> >(
            non.size() + term.size()); //string will be concatenated sequence of first terminals. Nons then terms
    //fill all terms and epsilon sets with themselves
    for (int i = non.size(); i < firstSets.size(); i++) {
        firstSets.at(i).insert(term.at(i - non.size())); //mark all terminals' first sets with themselves
    }

    //fill up firstSets of nonterminals
    //increment through all rules repeatedly until FIRST sets dont change
    bool changed = true;
    while (changed) {
        changed = false;
        rule *r = head;
        while(r != NULL && r->next != NULL){

            RHS *rhs = r->rhs;

            //find lhs string in non, so you can adjust its first set
            ptrdiff_t lPos = distance(non.begin(), find(non.begin(), non.end(), r->non));

            set<string> startSet = firstSets.at(lPos); //save a copy of current state of set


            if (rhs->tok.empty()) { //
                firstSets.at(lPos).insert("#");
            } else {
                bool stop = false;
                while (!stop && rhs != NULL && rhs->next != NULL) {
                    //find rhs string in non, so you can adjust its first set
                    ptrdiff_t rPos = distance(non.begin(), find(non.begin(), non.end(), rhs->tok));
                    if(rPos == non.size())
                        //find the rhs string in terms
                        rPos = rPos + distance(term.begin(), find(term.begin(), term.end(), rhs->tok));

                    if (find(firstSets.at(rPos).begin(), firstSets.at(rPos).end(), "#") !=
                        firstSets.at(rPos).end()) { //if first set of RHS node contains epsilon

                        //add everything from first set of RHS node except espilon
                        set<string> temp = firstSets.at(rPos);
                        temp.erase("#");
                        firstSets.at(lPos).insert(temp.begin(), temp.end()); //first set of rhs to first set of lpos

                        if (rhs->next->tok.empty()) //if there are no more tokens on RHS
                            firstSets.at(lPos).insert("#"); //add in epsilon

                    } else { //first set of RHS node doesnt contain epsilon
                        firstSets.at(lPos).insert(firstSets.at(rPos).begin(),
                                                  firstSets.at(rPos).end()); //first set of rhs to first set of i

                        stop = true;
                    }

                    rhs = rhs->next;
                }
            }

            //check if set changed
            if (startSet != firstSets.at(lPos))
                changed = true;

            r = r->next;
        }
    }

    return firstSets;
}

void printFirst(rule * head) {
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(head);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    vector< set <string> > firstSets = first(head);

    //print non terms in order
    //only parse non-useless rules
    for(int j = 0; j < non.size(); j++){
        //find where in firstSets to look

        cout << "FIRST(" << non.at(j) << ") = { ";

        //print first first (every non has at least one
        //find rule in list of rules
        bool commas = false;
        vector<string>::iterator termIt;
        if(find(firstSets.at(j).begin(), firstSets.at(j).end(), "#") != firstSets.at(j).end()){ //epsilon is in first set of rule
            cout << "#" ;
            commas = true;
        }


        for(int i = 0; i < term.size(); i++) {

            if(find(firstSets.at(j).begin(), firstSets.at(j).end(), term.at(i)) != firstSets.at(j).end()){ //terminal is in first set of rule
                if(commas)
                    cout << ", ";

                cout << term.at(i);
                commas = true;
            }
        }
        cout<< " }\n";
    }
}

vector<set < string > > follow(rule * head){
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(head);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    //get first sets
    vector< set <string> > firstSets = first(head);

    vector<set<string> > followSets = vector<set<string> > (non.size());

    //fill start symbol with eof
    followSets.at(0).insert("$");

    //fill up followSets
    //increment through al rules repeatedly until FOLLOW sets dont change
    bool changed = true;
    while (changed) {
        changed = false;
        rule *r = head;
        vector< set<string> > startSets = followSets; //save a copy of current state of set

        while (r != NULL && r->next != NULL) {

            RHS *rhs = r->rhs;

            //find lhs string in non, so you can adjust its follow set
            ptrdiff_t lPos = distance(non.begin(), find(non.begin(), non.end(), r->non));

            bool stop = false;
            while (!stop && rhs != NULL && rhs->next != NULL) {
                //find rhs string in non, so you can adjust its first set
                ptrdiff_t rPos = distance(non.begin(), find(non.begin(), non.end(), rhs->tok));

                RHS * rhs2 = rhs->next;

                if(rPos < non.size()) {
                    if (!rhs->next->tok.empty()) { //rhs->next is a non term

                        //find next rhs string in non or term, so you can access its first set
                        ptrdiff_t r2Pos = distance(non.begin(), find(non.begin(), non.end(), rhs->next->tok));
                        if (r2Pos == non.size())
                            r2Pos = non.size() + distance(term.begin(), find(term.begin(), term.end(), rhs->next->tok));

                        if (find(firstSets.at(r2Pos).begin(), firstSets.at(r2Pos).end(), "#") !=
                            firstSets.at(r2Pos).end()) { //if first set of RHS2 node contains epsilon

                            //add everything from first set of RHS2 node except epsilon
                            set<string> temp = firstSets.at(r2Pos);
                            temp.erase("#");
                            followSets.at(rPos).insert(temp.begin(), temp.end()); //first set of rhs2 to follow set of rhs

                            followSets.at(rPos).insert(followSets.at(lPos).begin(), followSets.at(lPos).end());

                        } else { //first set of RHS node doesnt contain epsilon
                            followSets.at(rPos).insert(firstSets.at(r2Pos).begin(), firstSets.at(r2Pos).end()); //first set of rhs2 to first set of rhs
                            if(rhs->next->next->tok.empty() && r2Pos<followSets.size()) //r->next is last token in RHS
                                followSets.at(r2Pos).insert(followSets.at(lPos).begin(), followSets.at(lPos).end());

                            //stop = true;
                        }
                    } else { //rhs has no token after it
                        //add follow of lhs to follow of rhs
                        followSets.at(rPos).insert(followSets.at(lPos).begin(), followSets.at(lPos).end());

                        //add follow of rhs to follow of lhs
                        //followSets.at(lPos).insert(followSets.at(rPos).begin(), followSets.at(rPos).end());
                    }
                }


                rhs = rhs->next;
            }

            r = r->next;
        }

        //check if set changed
        if (startSets != followSets)
            changed = true;
    }

    return followSets;
}

void printFollow(rule * head){
    //get nonterminals and terminals
    pair<vector<string>, vector<string> > tokens = sortTokens(head);
    vector<string> non = tokens.first;
    vector<string> term = tokens.second;

    vector< set <string> > followSets = follow(head);

    //print non terms in order
    //only parse non-useless rules
    for(int j = 0; j < non.size(); j++){
        //find where in firstSets to look

        cout << "FOLLOW(" << non.at(j) << ") = { ";

        //print first first (every non has at least one
        //find rule in list of rules
        bool commas = false;
        vector<string>::iterator termIt;
        if(find(followSets.at(j).begin(), followSets.at(j).end(), "$") != followSets.at(j).end()){ //epsilon is in first set of rule
            cout << "$" ;
            commas = true;
        }


        for(int i = 0; i < term.size(); i++) {

            if(find(followSets.at(j).begin(), followSets.at(j).end(), term.at(i)) != followSets.at(j).end()){ //terminal is in first set of rule
                if(commas)
                    cout << ", ";

                cout << term.at(i);
                commas = true;
            }
        }
        cout<< " }\n";
    }
}

int main (int argc, char* argv[])
{
    int task;

    if (argc < 2)
    {
        cout << "Error: missing argument\n";
        return 1;
    }

    /*
       Note that by convention argv[0] is the name of your executable,
       and the first argument to your program is stored in argv[1]
     */

    task = atoi(argv[1]);

    //parse first rule
    Token t = lex.GetToken();
    rule *head = new rule;

    if(t.token_type == ID) {
        head->non = t.lexeme;
        //head->rhs = (RHS*) malloc(sizeof(RHS));
        //head->next = (rule*) malloc(sizeof(rule));
        t = lex.GetToken();
        if(t.token_type == ARROW){
            if(!parseRHS(head->rhs = new RHS, lex.GetToken())){
                cout << "Error: tokens on RHS must either be ID or a HASH\n";
                return 1;
            }

            //parse any additional rules
            if(!parseRule(head->next = new rule, lex.GetToken())){
                cout << "Error: invalid rules\n";
                return 1;
            }
        }else{
            cout << "Error: rules must have an arrow as their second token\n";
            return 1;
        }
    }else{
        cout << "Error: rules must start with an ID\n";
        return 1;
    }

    int a = 0;
    /*
       Hint: You can modify and use the lexer from previous project
       to read the input. Note that there are only 4 token types needed
       for reading the input in this project.

       WARNING: You will need to modify lexer.cc and lexer.h to only
       support the tokens needed for this project if you are going to
       use the lexer.
     */

    switch (task) {
        case 1:
            printAll(head);
            break;

        case 2:
            printVec(prune(head));
            break;

        case 3:
            printFirst(head);
            break;

        case 4:
            printFollow(head);
            break;

        case 5:
            // TODO: perform task 5.
            break;

        default:
            cout << "Error: unrecognized task number " << task << "\n";
            break;
    }
    return 0;
}

