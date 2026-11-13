#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;

namespace Art {
const string cyan="\033[36m", purple="\033[35m", red="\033[31m", green="\033[32m", yellow="\033[33m", reset="\033[0m";
void title(const string& t){ cout<<"\n"<<purple<<"╔══════════════════════════════════════════════════════════════╗\n║"<<setw(34+(int)t.size()/2)<<t<<setw(30-(int)t.size()/2)<<"║\n╚══════════════════════════════════════════════════════════════╝"<<reset<<"\n"; }
}
struct Item{string name,description;int attack=0,defense=0,heal=0,value=0;bool equipment=false;};
struct Quest{string name,objective,reward;int goal=0,progress=0,xp=0,gold=0;bool completed=false,claimed=false;};
enum class Element { Eau, Feu, Plante, Ombre };
string elementName(Element e){switch(e){case Element::Eau:return "Eau";case Element::Feu:return "Feu";case Element::Plante:return "Plante";default:return "Ombre";}}
struct Enemy{string name;int hp,maxHp,attack,defense,chance,xp,gold;Element element;bool boss=false;string className;};

// Hiérarchie de classes : chaque branche respecte l'héritage demandé.
class Classe {
public:
 virtual ~Classe()=default;
 virtual int id()const=0; virtual string name()const=0; virtual string skill()const=0;
 virtual int attackBonus()const{return 0;} virtual int skillDamage(int level)const{return 8+level*2;}
 virtual int skillHeal(int)const{return 0;} virtual int luckBonus()const{return 0;} virtual int criticalChance()const{return 8;} virtual bool isBoss()const{return false;}
};
class Guerrier:public Classe{public:int id()const override{return 1;}string name()const override{return "Guerrier";}string skill()const override{return "Frappe tellurique";}int attackBonus()const override{return 2;}int skillDamage(int l)const override{return 16+l*3;}};
class Berserker:public Guerrier{public:int id()const override{return 2;}string name()const override{return "Berserker";}string skill()const override{return "Rage des braises";}int attackBonus()const override{return 5;}int skillDamage(int l)const override{return 24+l*4;}};
class Paladin:public Berserker{public:int id()const override{return 3;}string name()const override{return "Paladin";}string skill()const override{return "Égide radieuse";}int attackBonus()const override{return 7;}int skillDamage(int l)const override{return 28+l*4;}int skillHeal(int m)const override{return m/5;}};
class Priest:public Classe{public:int id()const override{return 4;}string name()const override{return "Prêtre";}string skill()const override{return "Lumière réparatrice";}int attackBonus()const override{return 1;}int skillDamage(int l)const override{return 12+l*2;}int skillHeal(int m)const override{return m/3;}};
class Oracle:public Priest{public:int id()const override{return 5;}string name()const override{return "Oracle";}string skill()const override{return "Présage astral";}int attackBonus()const override{return 3;}int skillDamage(int l)const override{return 22+l*3;}int skillHeal(int m)const override{return m/4;}};
class Templier:public Oracle{public:int id()const override{return 6;}string name()const override{return "Templier";}string skill()const override{return "Sceau du jugement";}int attackBonus()const override{return 6;}int skillDamage(int l)const override{return 30+l*4;}int skillHeal(int m)const override{return m/5;}};
class Vampire:public Classe{public:int id()const override{return 7;}string name()const override{return "Vampire";}string skill()const override{return "Morsure sanguine";}int attackBonus()const override{return 3;}int skillDamage(int l)const override{return 18+l*3;}int skillHeal(int m)const override{return m/6;}};
class Sanguinaire:public Vampire{public:int id()const override{return 8;}string name()const override{return "Sanguinaire";}string skill()const override{return "Marée écarlate";}int attackBonus()const override{return 6;}int skillDamage(int l)const override{return 27+l*4;}int skillHeal(int m)const override{return m/4;}};
class SeigneurNocturne:public Sanguinaire{public:int id()const override{return 9;}string name()const override{return "Seigneur Nocturne";}string skill()const override{return "Éclipse prédatrice";}int attackBonus()const override{return 9;}int skillDamage(int l)const override{return 36+l*5;}int skillHeal(int m)const override{return m/3;}};
class PileFace:public Classe{public:int id()const override{return 10;}string name()const override{return "Pile Face";}string skill()const override{return "Destin hasardeux";}int attackBonus()const override{return 1;}int luckBonus()const override{return 35;}int criticalChance()const override{return 30;}};
class BossDeFin:public SeigneurNocturne{public:int id()const override{return 10;}string name()const override{return "BossDeFin";}string skill()const override{return "Anéantissement de la Faille";}int attackBonus()const override{return 18;}int skillDamage(int l)const override{return 55+l*7;}int skillHeal(int m)const override{return m/2;}bool isBoss()const override{return true;}};
shared_ptr<Classe> makeClass(int id){switch(id){case 1:return make_shared<Guerrier>();case 2:return make_shared<Berserker>();case 3:return make_shared<Paladin>();case 4:return make_shared<Priest>();case 5:return make_shared<Oracle>();case 6:return make_shared<Templier>();case 7:return make_shared<Vampire>();case 8:return make_shared<Sanguinaire>();case 9:return make_shared<SeigneurNocturne>();case 10:return make_shared<PileFace>();case 11:return make_shared<BossDeFin>();default:return make_shared<Guerrier>();}}
class Player{
public:
 string name,className;shared_ptr<Classe> classe=makeClass(1);int level=1,xp=0,gold=80,hp=100,maxHp=100,baseAttack=12,baseDefense=4,chance=10;vector<Item> inventory;Item weapon{"Lame rouillée","Une arme de fortune.",2,0,0,0,true};Item armor{"Manteau de voyageur","Une protection légère.",0,2,0,0,true};vector<Quest> quests;
 int attack()const{return baseAttack+weapon.attack+classe->attackBonus();} int defense()const{return baseDefense+armor.defense;} int criticalChance()const{return min(60,chance+classe->luckBonus());} int xpNext()const{return 100+(level-1)*75;}
 void showStats()const{Art::title("FICHE DU HÉROS");cout<<Art::yellow<<name<<Art::reset<<" — "<<className<<" — Niveau "<<level<<" — Classe : "<<classe->name()<<"\nCompétence : "<<classe->skill()<<"\nPV : "<<hp<<"/"<<maxHp<<"   ATK : "<<attack()<<"   DEF : "<<defense()<<"   Chance : "<<criticalChance()<<"%   Or : "<<gold<<"\nXP : [";int bars=20*xp/xpNext();for(int i=0;i<20;i++)cout<<(i<bars?'#':'-');cout<<"] "<<xp<<"/"<<xpNext()<<"\nArme : "<<weapon.name<<" (ATK +"<<weapon.attack<<") | Armure : "<<armor.name<<" (DEF +"<<armor.defense<<")\n";}
 void gainXp(int amount){xp+=amount;cout<<Art::green<<"+"<<amount<<" XP"<<Art::reset<<"\n";while(xp>=xpNext()){xp-=xpNext();level++;maxHp+=18;baseAttack+=4;baseDefense+=2;chance+=1;hp=maxHp;cout<<Art::yellow<<"★ NIVEAU "<<level<<" ! Vos forces se renouvellent."<<Art::reset<<"\n";}}
 void addItem(const Item&i){inventory.push_back(i);cout<<Art::green<<"Objet obtenu : "<<i.name<<Art::reset<<"\n";}
};
mt19937 rng((unsigned)time(nullptr));
int roll(int a,int b){return uniform_int_distribution<int>(a,b)(rng);}
const string SAVE_FILE="savegame.txt";
bool saveD1=false,saveD2=false,saveD3=false,saveD4=false; int arenaBestWave=0;
void saveGame(const Player&p,bool d1,bool d2,bool d3){
 ofstream out(SAVE_FILE); if(!out)return;
 out<<"CHRONIQUES_SAVE_V1\n"<<quoted(p.name)<<' '<<quoted(p.className)<<' '<<p.classe->id()<<' '<<p.level<<' '<<p.xp<<' '<<p.gold<<' '<<p.hp<<' '<<p.maxHp<<' '<<p.baseAttack<<' '<<p.baseDefense<<' '<<p.chance<<'\n';
 auto writeItem=[&](const Item&i){out<<quoted(i.name)<<' '<<quoted(i.description)<<' '<<i.attack<<' '<<i.defense<<' '<<i.heal<<' '<<i.value<<' '<<i.equipment<<' ';};
 writeItem(p.weapon);writeItem(p.armor);out<<'\n'<<p.inventory.size()<<'\n'; for(const auto&i:p.inventory){writeItem(i);out<<'\n';}
 out<<p.quests.size()<<'\n';for(const auto&q:p.quests)out<<quoted(q.name)<<' '<<quoted(q.objective)<<' '<<quoted(q.reward)<<' '<<q.goal<<' '<<q.progress<<' '<<q.xp<<' '<<q.gold<<' '<<q.completed<<' '<<q.claimed<<'\n';
 out<<d1<<' '<<d2<<' '<<d3<<'\n'<<saveD4<<' '<<arenaBestWave<<'\n';
}
bool loadGame(Player&p,bool&d1,bool&d2,bool&d3){
 ifstream in(SAVE_FILE);string marker;if(!(in>>marker)||marker!="CHRONIQUES_SAVE_V1")return false;
 int classId;if(!(in>>quoted(p.name)>>quoted(p.className)>>classId>>p.level>>p.xp>>p.gold>>p.hp>>p.maxHp>>p.baseAttack>>p.baseDefense>>p.chance))return false;p.classe=makeClass(classId);
 auto readItem=[&](Item&i){return (bool)(in>>quoted(i.name)>>quoted(i.description)>>i.attack>>i.defense>>i.heal>>i.value>>i.equipment);};
 if(!readItem(p.weapon)||!readItem(p.armor))return false;
 size_t count;
 if(!(in>>count))return false;
 p.inventory.clear();
 for(size_t i=0;i<count;i++){Item x;if(!readItem(x))return false;p.inventory.push_back(x);}
 if(!(in>>count))return false;
 p.quests.clear();
 for(size_t i=0;i<count;i++){Quest q;if(!(in>>quoted(q.name)>>quoted(q.objective)>>quoted(q.reward)>>q.goal>>q.progress>>q.xp>>q.gold>>q.completed>>q.claimed))return false;p.quests.push_back(q);}
 if(!(in>>d1>>d2>>d3))return false;
 if(!(in>>saveD4>>arenaBestWave)){in.clear();saveD4=false;arenaBestWave=0;}
 return true;
}
int ask(int min,int max){int n;while(true){if(cin>>n&&n>=min&&n<=max){cin.ignore(numeric_limits<streamsize>::max(),'\n');return n;}cin.clear();cin.ignore(numeric_limits<streamsize>::max(),'\n');cout<<"Choix invalide. Recommencez : ";}}
Enemy makeEnemy(int d,bool boss=false){
 if(d==1){if(boss)return Enemy{"Gardien des Cendres",180,180,22,8,15,220,140,Element::Feu,true,"Guerrier"};return Enemy{"Rat d'obsidienne",45,45,9,2,8,45,20,Element::Feu,false,"Bête"};}
 if(d==2){if(boss)return Enemy{"Oracle des Marées",270,270,30,12,20,360,220,Element::Eau,true,"Oracle"};return Enemy{"Spectre noyé",82,82,16,5,12,80,35,Element::Eau,false,"Spectre"};}
 if(d==3){if(boss)return Enemy{"Roi Sans-Visage",390,390,38,16,25,550,350,Element::Ombre,true,"Seigneur Nocturne"};return Enemy{"Chevalier de la Faille",125,125,23,9,18,125,60,Element::Plante,false,"Chevalier"};}
 if(d==4){if(boss)return Enemy{"Archonte du Néant",620,620,52,24,32,1100,750,Element::Ombre,true,"Templier"};return Enemy{"Mimiqueur de Cristal",170,170,29,12,22,180,90,Element::Eau,false,"Paladin"};}
 return Enemy{"Reine aux Milles-Visages",520,520,46,20,35,900,600,Element::Ombre,true,"BossDeFin"};
}
vector<Enemy> arenaRoster(int wave){
 vector<Enemy> v={
  {"Garde de Fer",90,90,18,6,10,70,30,Element::Feu,false,"Guerrier"},{"Brisecrâne",115,115,24,5,13,85,38,Element::Ombre,false,"Berserker"},{"Chevalier Prismatique",145,145,22,12,11,100,45,Element::Eau,false,"Paladin"},{"Pénitent Cendré",100,100,17,7,15,95,42,Element::Feu,false,"Prêtre"},{"Voyante du Fil",110,110,20,8,18,110,48,Element::Eau,false,"Oracle"},{"Sceau Vivant",155,155,25,14,14,125,55,Element::Plante,false,"Templier"},{"Néophyte Sanguin",105,105,23,5,16,105,46,Element::Ombre,false,"Vampire"},{"Dévoreur Rouge",135,135,28,7,19,130,58,Element::Feu,false,"Sanguinaire"},{"Duc de la Nuit",180,180,32,11,22,160,72,Element::Ombre,false,"Seigneur Nocturne"},{"Lame du Quartier Noir",125,125,27,8,12,115,50,Element::Feu,false,"Guerrier"},{"Furie Fendue",150,150,35,6,20,145,65,Element::Feu,false,"Berserker"},{"Héraut d'Azur",190,190,28,16,15,155,70,Element::Eau,false,"Paladin"},{"Sœur des Braises",135,135,24,9,21,140,62,Element::Feu,false,"Prêtre"},{"Oracle Aveugle",160,160,31,10,24,175,78,Element::Eau,false,"Oracle"},{"Templier de Bronze",210,210,34,18,16,190,85,Element::Plante,false,"Templier"},{"Comte des Veines",175,175,36,9,23,180,80,Element::Ombre,false,"Vampire"},{"Sanguinaire Royal",220,220,42,10,25,215,95,Element::Feu,false,"Sanguinaire"},{"Marquis Sans-Lune",250,250,45,14,27,250,110,Element::Ombre,false,"Seigneur Nocturne"},{"Paladin du Miroir",240,240,37,20,18,230,100,Element::Plante,false,"Paladin"},{"Avatar de la Faille",300,300,50,17,30,300,140,Element::Ombre,false,"Seigneur Nocturne"}
 };
 for(auto &e:v){int hpBonus=(wave-1)*12; e.hp+=hpBonus;e.maxHp+=hpBonus;e.attack+=(wave-1)*3;e.defense+=(wave-1)*2;e.chance=min(60,e.chance+(wave-1));e.xp+=wave*12;e.gold+=wave*7;}
 return v;
}

void updateQuests(Player&p,const string&kind){for(auto&q:p.quests)if(!q.completed&&q.objective==kind){q.progress=min(q.goal,q.progress+1);if(q.progress>=q.goal){q.completed=true;cout<<Art::yellow<<"Quête accomplie : "<<q.name<<Art::reset<<"\n";}}}
void showQuests(Player&p){Art::title("QUÊTES");if(p.quests.empty())cout<<"Aucune quête active.\n";for(auto&q:p.quests)cout<<(q.claimed?"[RÉCOMPENSÉE] ":q.completed?"[TERMINÉE] ":"[EN COURS] ")<<q.name<<" — "<<q.progress<<"/"<<q.goal<<" — "<<q.reward<<"\n";}
void claimQuests(Player&p){for(auto&q:p.quests)if(q.completed&&!q.claimed){q.claimed=true;p.gold+=q.gold;p.gainXp(q.xp);cout<<"Récompense reçue : "<<q.gold<<" or.\n";}}
bool combat(Player&p,Enemy e){Art::title(e.boss?"BOSS — "+e.name:"RENCONTRE — "+e.name);while(p.hp>0&&e.hp>0){cout<<"\n"<<Art::red<<e.name<<Art::reset<<" PV "<<e.hp<<"/"<<e.maxHp<<" | ATK "<<e.attack<<" | Chance "<<e.chance<<"% | Élément "<<elementName(e.element)<<" | Vous "<<p.hp<<"/"<<p.maxHp<<"\n1. Frappe   2. Technique   3. Potion   4. Fuir\n> ";int c=ask(1,4);if(c==4&&!e.boss){cout<<"Vous vous repliez dans l'ombre.\n";return false;}if(c==3){auto it=find_if(p.inventory.begin(),p.inventory.end(),[](const Item&i){return i.heal>0;});if(it==p.inventory.end()){cout<<"Vous n'avez plus de potion.\n";continue;}p.hp=min(p.maxHp,p.hp+it->heal);cout<<"Vous récupérez "<<it->heal<<" PV.\n";p.inventory.erase(it);}else{int dmg=c==2?max(1,p.attack()+p.classe->skillDamage(p.level)+roll(0,4)-e.defense):max(1,p.attack()+roll(-3,6)-e.defense);if(c==2){cout<<Art::cyan<<p.classe->skill()<<" !"<<Art::reset<<"\n";int heal=p.classe->skillHeal(p.maxHp);if(heal>0){p.hp=min(p.maxHp,p.hp+heal);cout<<"La compétence restaure "<<heal<<" PV.\n";}}if(roll(1,100)<=p.criticalChance()){dmg*=2;cout<<Art::yellow<<"Coup critique !"<<Art::reset<<"\n";}e.hp-=dmg;cout<<"Vous infligez "<<dmg<<" dégâts.\n";updateQuests(p,"vaincre");}if(e.hp<=0)break;int taken=max(1,e.attack+roll(-3,5)-p.defense());if(roll(1,100)<=e.chance){taken*=2;cout<<Art::red<<"Coup critique ennemi !"<<Art::reset<<"\n";}p.hp-=taken;cout<<e.name<<" vous inflige "<<taken<<" dégâts.\n";}if(p.hp<=0){cout<<Art::red<<"La Faille vous a vaincu. Votre légende s'arrête ici."<<Art::reset<<"\n";return false;}cout<<Art::green<<"Victoire ! "<<e.name<<" est vaincu."<<Art::reset<<"\n";p.gold+=e.gold;p.gainXp(e.xp);updateQuests(p,"vaincre");if(e.boss)updateQuests(p,"boss");return true;}
void inventory(Player&p){while(true){Art::title("ARSENAL ET SACOCHE");cout<<"Équipé — Arme : "<<p.weapon.name<<" | Armure : "<<p.armor.name<<"\n";if(p.inventory.empty())cout<<"Sac vide.\n";for(size_t i=0;i<p.inventory.size();i++)cout<<i+1<<". "<<p.inventory[i].name<<" — "<<p.inventory[i].description<<"\n";cout<<"0. Retour\n> ";int c=ask(0,(int)p.inventory.size());if(c==0)return;Item item=p.inventory[c-1];if(item.equipment){if(item.attack>0)p.weapon=item;else p.armor=item;cout<<"Équipement changé.\n";p.inventory.erase(p.inventory.begin()+c-1);}else if(item.heal>0){p.hp=min(p.maxHp,p.hp+item.heal);p.inventory.erase(p.inventory.begin()+c-1);cout<<"Potion utilisée.\n";}}}
void shop(Player&p){Art::title("ÉCHOPPE DE LA PLACE NOIRE");vector<Item>s={{"Potion solaire","+45 PV",0,0,45,25,false},{"Épée turquoise","Une lame chargée d'énergie.",9,0,0,120,true},{"Plastron forestier","Une armure souple et dense.",0,8,0,120,true}};cout<<"Or : "<<p.gold<<"\n";for(size_t i=0;i<s.size();i++)cout<<i+1<<". "<<s[i].name<<" — "<<s[i].description<<" — "<<s[i].value<<" or\n";cout<<"0. Retour\n> ";int c=ask(0,(int)s.size());if(c&&p.gold>=s[c-1].value){p.gold-=s[c-1].value;p.addItem(s[c-1]);}else if(c)cout<<"Votre bourse ne suffit pas.\n";}
bool dungeon(Player&p,int d,const string&name){Art::title("DONJON : "+name);cout<<"Les murs murmurent. Deux salles vous séparent du gardien.\n";for(int i=1;i<=2;i++){cout<<"\nSalle "<<i<<"/2 — une présence se manifeste.\n";if(!combat(p,makeEnemy(d)))return false;if(roll(1,100)<=65)p.addItem({"Potion trouvée","Une lueur liquide.",0,0,35,0,false});}cout<<"\nLa salle du trône s'ouvre.\n";if(!combat(p,makeEnemy(d,true)))return false;if(d==3){cout<<Art::red<<"\nUne seconde présence descend du trône : la Reine aux Milles-Visages."<<Art::reset<<"\n";if(!combat(p,makeEnemy(4,true)))return false;}cout<<Art::yellow<<"Le donjon est purifié. Un nouveau chemin s'éveille."<<Art::reset<<"\n";return true;}

bool arena(Player&p){
 Art::title("ARÈNE INFINIE");cout<<"Vingt adversaires peuvent surgir à chaque cycle. La difficulté augmente à chaque vague.\nMeilleur score actuel : vague "<<arenaBestWave<<".\n";
 int wave=1;
 while(p.hp>0){auto roster=arenaRoster(wave);Enemy enemy=roster[roll(0,(int)roster.size()-1)];cout<<"\n"<<Art::yellow<<"VAGUE "<<wave<<" — "<<enemy.name<<" ["<<enemy.className<<"]"<<Art::reset<<"\n";if(!combat(p,enemy))break;int reward=wave*25;p.gold+=reward;p.gainXp(wave*30);cout<<Art::green<<"Récompense d'arène : "<<reward<<" or."<<Art::reset<<"\n";arenaBestWave=max(arenaBestWave,wave);p.hp=min(p.maxHp,p.hp+p.maxHp/5);cout<<"Récupération entre les vagues : "<<p.hp<<"/"<<p.maxHp<<" PV.\n";cout<<"1. Vague suivante   2. Quitter l'arène\n> ";if(ask(1,2)==2)break;wave++;}
 if(wave>arenaBestWave)arenaBestWave=wave;
 cout<<"Meilleure vague : "<<arenaBestWave<<".\n";
 return p.hp>0;
}
void addStoryQuest(Player&p,int d){Quest q;if(d==1)q={"Les braises de la mémoire","vaincre","Purifier les Cendres",3,0,120,50};if(d==2)q={"Le chant sous les eaux","boss","Réveiller l'Oracle",1,0,220,100};if(d==3)q={"La dernière mue","boss","Briser le Roi Sans-Visage",1,0,350,200};if(d==4)q={"Le sanctuaire inversé","boss","Vaincre l'Archonte du Néant",1,0,500,300};p.quests.push_back(q);cout<<Art::yellow<<"Nouvelle quête : "<<q.name<<" — "<<q.reward<<Art::reset<<"\n";}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);Art::title("CHRONIQUES DE LA FAILLE");Player p;bool d1=false,d2=false,d3=false;ifstream existing(SAVE_FILE);bool loaded=false;if(existing.good()){cout<<"Une sauvegarde automatique a été trouvée.\n1. Reprendre la partie\n2. Nouvelle partie\n> ";int choice=ask(1,2);if(choice==1){loaded=loadGame(p,d1,d2,d3);if(loaded)cout<<Art::green<<"Progression restaurée depuis "<<SAVE_FILE<<"."<<Art::reset<<"\n";else cout<<"Sauvegarde illisible, création d'une nouvelle partie.\n";}}if(!loaded){cout<<"Nom du héros : ";string n;getline(cin,n);if(n.empty())n="Astra";p.name=n;Art::title("CHOIX DE CLASSE");cout<<"1. Guerrier — Frappe tellurique\n2. Berserker — Rage des braises\n3. Paladin — Égide radieuse\n4. Prêtre — Lumière réparatrice\n5. Oracle — Présage astral\n6. Templier — Sceau du jugement\n7. Vampire — Morsure sanguine\n8. Sanguinaire — Marée écarlate\n9. Seigneur Nocturne — Éclipse prédatrice\n10. Pile Face — Destin hasardeux\n> ";int c=ask(1,10);p.classe=makeClass(c);p.className=p.classe->name();if(c==1){p.maxHp=125;p.hp=125;p.baseDefense=7;}else if(c==2||c==3){p.maxHp=115;p.hp=115;p.baseDefense=6;}else if(c==4||c==5||c==6){p.maxHp=105;p.hp=105;p.baseDefense=5;}else if(c==7||c==8||c==9){p.maxHp=110;p.hp=110;p.baseDefense=4;}else{p.maxHp=100;p.hp=100;p.baseDefense=3;p.chance=35;}p.inventory={{"Potion solaire","+45 PV",0,0,45,25,false},{"Potion solaire","+45 PV",0,0,45,25,false}};p.quests.push_back({"Premier éclat","vaincre","Vaincre votre premier adversaire",1,0,60,30});saveGame(p,d1,d2,d3);cout<<Art::green<<"Sauvegarde automatique créée."<<Art::reset<<"\n";}while(p.hp>0){Art::title("PLACE NOIRE — HUB");cout<<"1. Explorer les donjons\n2. Fiche du héros\n3. Sac et équipement\n4. Voir et réclamer les quêtes\n5. Échoppe\n6. Repos\n7. Arène infinie\n0. Quitter\n> ";int m=ask(0,7);if(m==0){saveGame(p,d1,d2,d3);break;}if(m==2)p.showStats();else if(m==3)inventory(p);else if(m==4){showQuests(p);claimQuests(p);}else if(m==5)shop(p);else if(m==6){p.hp=p.maxHp;cout<<"Vous vous reposez. Les blessures se referment.\n";}else if(m==7){arena(p);}else if(m==1){cout<<"1. Cathédrale des Cendres"<<(d1?" [PURIFIÉE]":"")<<"\n2. Jardins Engloutis"<<(d2?" [PURIFIÉE]":"")<<"\n3. Faille du Roi"<<(d3?" [PURIFIÉE]":"")<<"\n4. Sanctuaire inversé (optionnel)"<<(saveD4?" [PURIFIÉ]":"")<<"\n0. Retour\n> ";int d=ask(0,4);if(d==1&&!d1){addStoryQuest(p,1);d1=dungeon(p,1,"Cathédrale des Cendres");}else if(d==2&&!d2&&d1){addStoryQuest(p,2);d2=dungeon(p,2,"Jardins Engloutis");}else if(d==3&&!d3&&d2){addStoryQuest(p,3);d3=dungeon(p,3,"Faille du Roi");}else if(d==4&&!saveD4&&d3){addStoryQuest(p,4);saveD4=dungeon(p,4,"Sanctuaire inversé");}else if(d&&((d==2&&!d1)||(d==3&&!d2)))cout<<"Le chemin est scellé. Purifiez le donjon précédent.\n";else if(d)cout<<"Ce lieu a déjà été purifié.\n";if(d1&&d2&&d3){Art::title("FIN — L'AUBE SUR LA FAILLE");cout<<"Les trois gardiens sont tombés. Votre stratégie a transformé la nuit en passage.\n";p.showStats();saveGame(p,d1,d2,d3);}}saveGame(p,d1,d2,d3);}cout<<"\nMerci d'avoir écrit cette légende.\n";return 0;}
