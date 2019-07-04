#include<iostream>
#include<string>
#include"Input.h"
using namespace std;

char charInput(string allowedInput){
    string inputString;
    while(true){
        cin.clear();
        getline(cin, inputString);
        if(cin.good() && inputString.length() == 1 && allowedInput.find(inputString) != string::npos){
            return inputString.at(0);
        }
        cout<<"Wrong input, try again."<<endl;
    }
}
