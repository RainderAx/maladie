#pragma once
#include <memory>
#include <string>

class CharacterClass {
public:
    virtual ~CharacterClass() = default;
    virtual int id() const = 0;
    virtual std::string name() const = 0;
    virtual std::string skill() const = 0;
    virtual int attackBonus() const { return 0; }
    virtual int luckBonus() const { return 0; }
    virtual int skillDamage(int level) const { return 8 + level * 2; }
    virtual int skillHeal(int) const { return 0; }
};

class Guerrier : public CharacterClass { public: int id()const override{return 1;} std::string name()const override{return "Guerrier";} std::string skill()const override{return "Frappe tellurique";} int attackBonus()const override{return 2;} int skillDamage(int l)const override{return 16+l*3;} };
class Berserker : public Guerrier { public: int id()const override{return 2;} std::string name()const override{return "Berserker";} std::string skill()const override{return "Rage des braises";} int attackBonus()const override{return 5;} int skillDamage(int l)const override{return 24+l*4;} };
class Paladin : public Berserker { public: int id()const override{return 3;} std::string name()const override{return "Paladin";} std::string skill()const override{return "Égide radieuse";} int attackBonus()const override{return 7;} int skillDamage(int l)const override{return 28+l*4;} int skillHeal(int m)const override{return m/5;} };
class Priest : public CharacterClass { public: int id()const override{return 4;} std::string name()const override{return "Prêtre";} std::string skill()const override{return "Lumière réparatrice";} int attackBonus()const override{return 1;} int skillDamage(int l)const override{return 12+l*2;} int skillHeal(int m)const override{return m/3;} };
class Oracle : public Priest { public: int id()const override{return 5;} std::string name()const override{return "Oracle";} std::string skill()const override{return "Présage astral";} int attackBonus()const override{return 3;} int skillDamage(int l)const override{return 22+l*3;} int skillHeal(int m)const override{return m/4;} };
class Templier : public Oracle { public: int id()const override{return 6;} std::string name()const override{return "Templier";} std::string skill()const override{return "Sceau du jugement";} int attackBonus()const override{return 6;} int skillDamage(int l)const override{return 30+l*4;} int skillHeal(int m)const override{return m/5;} };
class Vampire : public CharacterClass { public: int id()const override{return 7;} std::string name()const override{return "Vampire";} std::string skill()const override{return "Morsure sanguine";} int attackBonus()const override{return 3;} int skillDamage(int l)const override{return 18+l*3;} int skillHeal(int m)const override{return m/6;} };
class Sanguinaire : public Vampire { public: int id()const override{return 8;} std::string name()const override{return "Sanguinaire";} std::string skill()const override{return "Marée écarlate";} int attackBonus()const override{return 6;} int skillDamage(int l)const override{return 27+l*4;} int skillHeal(int m)const override{return m/4;} };
class SeigneurNocturne : public Sanguinaire { public: int id()const override{return 9;} std::string name()const override{return "Seigneur Nocturne";} std::string skill()const override{return "Éclipse prédatrice";} int attackBonus()const override{return 9;} int skillDamage(int l)const override{return 36+l*5;} int skillHeal(int m)const override{return m/3;} };
class PileFace : public CharacterClass { public: int id()const override{return 10;} std::string name()const override{return "Pile Face";} std::string skill()const override{return "Destin hasardeux";} int attackBonus()const override{return 1;} int luckBonus()const override{return 35;} int skillDamage(int l)const override{return 12+l*3;} };
class BossDeFin : public SeigneurNocturne { public: int id()const override{return 11;} std::string name()const override{return "BossDeFin";} std::string skill()const override{return "Anéantissement de la Faille";} int attackBonus()const override{return 18;} int skillDamage(int l)const override{return 55+l*7;} int skillHeal(int m)const override{return m/2;} };

std::shared_ptr<CharacterClass> makeCharacterClass(int id);
