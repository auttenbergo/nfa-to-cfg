#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>

using namespace std;

const string EPSILON = "EPSILON";
const string NFA_SAMPLES_FILE_PATH = "/NFA Samples/";

struct State {
    bool isRecepient;
    map<char, vector<State*>> transitions;
};

struct NFA {
    vector<State*> states;
};

struct Rule {
    string symbol;
    vector<string> terminals;
};

struct CFG {
    vector<Rule*> rules;
};

State* createEmptyState() {
	State* state = new State;
	state->isRecepient = false;
	state->transitions = map<char, vector<State*>>();
	return state;
}

NFA parseFile(ifstream& stream) {
	NFA nfa;

	string strStateCount; stream >> strStateCount;
	string strRecepientCount; stream >> strRecepientCount;
	string strTransitionCount; stream >> strTransitionCount;


	int stateCount = stoi(strStateCount);
	int recepientCount = stoi(strRecepientCount);
	int transitionCount = stoi(strTransitionCount);


	

	for (int i = 0; i < stateCount; i++) {
		State* state = createEmptyState();
		nfa.states.push_back(state);
	}

	for (int i = 0; i < recepientCount; i++) {
		string strIndex; stream >> strIndex;
		int index = stoi(strIndex);
		nfa.states[index]->isRecepient = true;
	}
	for (int i = 0; i < stateCount; i++) {
		State* current = nfa.states[i];
		string strCurrentTransitionCount; stream >> strCurrentTransitionCount;
		int currentTransitionCount = stoi(strCurrentTransitionCount);
		for (int j = 0; j < currentTransitionCount; j++) {
			string strSymbol; stream >> strSymbol;
			char symbol = strSymbol[0];
			string strIndex; stream >> strIndex;
			int index = stoi(strIndex);
			current->transitions[symbol].push_back(nfa.states[index]);
		}
	}
	return nfa;
}

State* createSimpleState(bool isRecepient, bool isLast) {
	State* state = new State;
	state->isRecepient = isRecepient;
	state->transitions = map<char, vector<State*>>();
	return state;
}

NFA createSimpleNFA(char symbol) {
	State* end = createSimpleState(true, true);
	State* start = createSimpleState(false, false);
	start->transitions[symbol].push_back(end);


	NFA rv;
	rv.states.push_back(start);
	rv.states.push_back(end);

	return rv;
}

int countRecepients(NFA nfa) {
	int rv = 0;
	for (int i = 0; i < nfa.states.size(); i++) {
		if (nfa.states[i]->isRecepient)
			rv++;
	}
	return rv;
}

int countStateTransitions(State* state) {
	int rv = 0;
	auto transitions = state->transitions;
	for (auto itr = transitions.begin(); itr != transitions.end(); ++itr) {
		rv += state->transitions[itr->first].size();
	}
	return rv;
}

int countTransitions(NFA nfa) {
	int rv = 0;
	for (int i = 0; i < nfa.states.size(); i++) {
		rv += countStateTransitions(nfa.states[i]);
	}
	return rv;
}

int findIndexOfStateInNFA(NFA& nfa, State* state) {
	for (int i = 0; i < nfa.states.size(); i++) {
		if (state == nfa.states[i])
			return i;
	}
	return -1;
}


void printNFA(NFA nfa) {
	/* States, Recepients, Transitions */
	cout << nfa.states.size() << " " << countRecepients(nfa) << " " << countTransitions(nfa) << endl;

	/* Recepient indexes */
	for (int i = 0; i < nfa.states.size(); i++) {
		if (nfa.states[i]->isRecepient) {
			cout << i << " ";
		}
	}
	cout << endl;

	/* For each state */
	for (int i = 0; i < nfa.states.size(); i++) {
		State* current = nfa.states[i];
		auto transitions = current->transitions;
		/* Print transition count */
		cout << countStateTransitions(current) << " ";

		/* For each transition */
		for (auto itr = transitions.begin(); itr != transitions.end(); ++itr) {
			char symbol = itr->first;
			vector<State*> transitionStates = itr->second;
			/* For each transitionState print transitions */
			for (int j = 0; j < transitionStates.size(); j++) {
				State* toState = transitionStates[j];
				cout << symbol << " " << findIndexOfStateInNFA(nfa, toState) << " ";
			}
		}
		cout << endl;
	}
}

void freeNFA(NFA& nfa) {
	for (int i = 0; i < nfa.states.size(); i++) {
		delete nfa.states[i];
	}
}

void freeCFG(CFG& cfg) {
	for (int i = 0; i < cfg.rules.size(); i++) {
		delete cfg.rules[i];
	}
}

Rule* createSimpleRule(string symbol) {
	Rule* rule = new Rule;
	rule->symbol = symbol;
	return rule;
}


void addTerminals(NFA& nfa,Rule* rule,int stateIndex,map<int,string>& stateNames) {
	State* state = nfa.states[stateIndex];
	map<char, vector<State*>> transitions = state->transitions;

	
	for (auto itr = transitions.begin(); itr != transitions.end(); ++itr) {
		string symbol(1, itr->first);
		vector<State*> transitionStates = itr->second;
		
		for (int i = 0; i < transitionStates.size(); i++) {
			string currentTerminal = "";
			State* currentTransitionState = transitionStates[i];
			currentTerminal += symbol + " " + stateNames[findIndexOfStateInNFA(nfa,currentTransitionState)];
			rule->terminals.push_back(currentTerminal);

		}
	}
	if (transitions.size() == 0 || state->isRecepient) {
		rule->terminals.push_back(EPSILON);
	}
	
}

map<int, string> getStateNames(NFA& nfa) {
	map<int, string> result;
	string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int counter = 1;
	for (int i = 0; i < nfa.states.size(); i++) {
		State* state = nfa.states[i];
		if (i == 0) {
			result[i] = "S";
			continue;
		}
		result[i] = "H" + to_string(counter);
		counter++;
	}
	return result;
}

/*
	HAS TO BE FINISHED 

*/
CFG convertNFAtoCFG(NFA& nfa) {
	CFG cfg;

	map<int, string> stateNames = getStateNames(nfa);

	for (int i = 0; i < nfa.states.size(); i++) {
		Rule* rule = createSimpleRule(stateNames[i]);
		addTerminals(nfa, rule,i,stateNames);
		cfg.rules.push_back(rule);
	}
	

	return cfg;
	
}

void printCFG(CFG& cfg) {
	/* For each rule */
	for (int i = 0; i < cfg.rules.size(); i++) {
		Rule* rule = cfg.rules[i];
		cout << rule->symbol << " -> ";
		/* For each terminal of current rule */
		for (int j = 0; j < rule->terminals.size(); j++) {
			cout << rule->terminals[j] << " ";
			if (j != rule->terminals.size() - 1)
				cout << "| ";

		}
		cout << endl;
	}
}
int main()
{
    for (int num = 0; num <= 19; num++) {
        string nfaFile = "in";
        nfaFile += (num / 10 == 0) ? "0" + to_string(num) : to_string(num);
		nfaFile += ".txt";

        ifstream nfaStream;
        nfaStream.open(NFA_SAMPLES_FILE_PATH + nfaFile);

        NFA nfa = parseFile(nfaStream);
		nfaStream.close();

		printNFA(nfa);
		cout << "----------------" << endl;
		CFG cfg = convertNFAtoCFG(nfa);
		printCFG(cfg);
		cout << "------------NEXT---------" << endl;

		freeNFA(nfa);
		freeCFG(cfg);
    }
    return 0;
}
