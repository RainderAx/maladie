#pragma once
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
namespace Terminal {
inline const std::string cyan="\033[36m", purple="\033[35m", red="\033[31m", green="\033[32m", yellow="\033[33m", reset="\033[0m";
inline void title(const std::string& text){std::cout<<"\n"<<purple<<"╔══════════════════════════════════════════════════════════════╗\n║"<<std::setw(34+(int)text.size()/2)<<text<<std::setw(30-(int)text.size()/2)<<"║\n╚══════════════════════════════════════════════════════════════╝"<<reset<<"\n";}
inline int ask(int min,int max){int n;while(true){if(std::cin>>n&&n>=min&&n<=max){std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');return n;}std::cin.clear();std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');std::cout<<"Choix invalide. Recommencez : ";}}
}
