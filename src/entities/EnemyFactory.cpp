#include "../../include/entities/EnemyFactory.hpp"
#include <algorithm>

Enemy makeEnemy(int zone,bool boss){
    if(zone==1)return boss?Enemy{"Gardien des Cendres","Guerrier",180,180,22,8,15,220,140,Element::Feu,true}:Enemy{"Rat d'obsidienne","Bête",45,45,9,2,8,45,20,Element::Feu,false};
    if(zone==2)return boss?Enemy{"Oracle des Marées","Oracle",270,270,30,12,20,360,220,Element::Eau,true}:Enemy{"Spectre noyé","Spectre",82,82,16,5,12,80,35,Element::Eau,false};
    if(zone==3)return boss?Enemy{"Roi Sans-Visage","Seigneur Nocturne",390,390,38,16,25,550,350,Element::Ombre,true}:Enemy{"Chevalier de la Faille","Chevalier",125,125,23,9,18,125,60,Element::Plante,false};
    if(zone==4)return boss?Enemy{"Archonte du Néant","Templier",620,620,52,24,32,1100,750,Element::Ombre,true}:Enemy{"Mimiqueur de Cristal","Paladin",170,170,29,12,22,180,90,Element::Eau,false};
    return Enemy{"Reine aux Milles-Visages","BossDeFin",520,520,46,20,35,900,600,Element::Ombre,true};
}

std::vector<Enemy> arenaRoster(int wave){
    std::vector<Enemy> monsters={
      {"Garde de Fer","Guerrier",90,90,18,6,10,70,30,Element::Feu,false},{"Brisecrâne","Berserker",115,115,24,5,13,85,38,Element::Ombre,false},{"Chevalier Prismatique","Paladin",145,145,22,12,11,100,45,Element::Eau,false},{"Pénitent Cendré","Prêtre",100,100,17,7,15,95,42,Element::Feu,false},{"Voyante du Fil","Oracle",110,110,20,8,18,110,48,Element::Eau,false},{"Sceau Vivant","Templier",155,155,25,14,14,125,55,Element::Plante,false},{"Néophyte Sanguin","Vampire",105,105,23,5,16,105,46,Element::Ombre,false},{"Dévoreur Rouge","Sanguinaire",135,135,28,7,19,130,58,Element::Feu,false},{"Duc de la Nuit","Seigneur Nocturne",180,180,32,11,22,160,72,Element::Ombre,false},{"Lame du Quartier Noir","Guerrier",125,125,27,8,12,115,50,Element::Feu,false},{"Furie Fendue","Berserker",150,150,35,6,20,145,65,Element::Feu,false},{"Héraut d'Azur","Paladin",190,190,28,16,15,155,70,Element::Eau,false},{"Sœur des Braises","Prêtre",135,135,24,9,21,140,62,Element::Feu,false},{"Oracle Aveugle","Oracle",160,160,31,10,24,175,78,Element::Eau,false},{"Templier de Bronze","Templier",210,210,34,18,16,190,85,Element::Plante,false},{"Comte des Veines","Vampire",175,175,36,9,23,180,80,Element::Ombre,false},{"Sanguinaire Royal","Sanguinaire",220,220,42,10,25,215,95,Element::Feu,false},{"Marquis Sans-Lune","Seigneur Nocturne",250,250,45,14,27,250,110,Element::Ombre,false},{"Paladin du Miroir","Paladin",240,240,37,20,18,230,100,Element::Plante,false},{"Avatar de la Faille","Seigneur Nocturne",300,300,50,17,30,300,140,Element::Ombre,false}
    };
    for(auto& enemy:monsters){int bonus=(wave-1);enemy.hp+=bonus*12;enemy.maxHp+=bonus*12;enemy.attack+=bonus*3;enemy.defense+=bonus*2;enemy.chance=std::min(60,enemy.chance+bonus);enemy.xp+=wave*12;enemy.gold+=wave*7;}
    return monsters;
}

std::string elementName(Element element){switch(element){case Element::Eau:return "Eau";case Element::Feu:return "Feu";case Element::Plante:return "Plante";default:return "Ombre";}}

Element elementFromId(int id){switch(id){case 0:return Element::Eau;case 1:return Element::Feu;case 2:return Element::Plante;default:return Element::Ombre;}}
int elementId(Element element){return static_cast<int>(element);}
double elementMultiplier(Element attackElement, Element defenseElement){
    if((attackElement==Element::Feu&&defenseElement==Element::Plante)||(attackElement==Element::Plante&&defenseElement==Element::Eau)||(attackElement==Element::Eau&&defenseElement==Element::Feu))return 1.5;
    if((attackElement==Element::Plante&&defenseElement==Element::Feu)||(attackElement==Element::Eau&&defenseElement==Element::Plante)||(attackElement==Element::Feu&&defenseElement==Element::Eau))return 0.75;
    return 1.0;
}
