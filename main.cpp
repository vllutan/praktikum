#include <iostream>
#include <string>

using namespace std;

class Transport{
 protected:
  int cost;
  int duration;
  string company;
  string *route_stops;
  int stops_num;

 public:

  static int counter;

  Transport(){
    cost = -1;
    duration = -1;
    company = "-";
    route_stops = nullptr;
    stops_num = 0;
    cout << "transport basic made" << endl;
    ++counter;
  }

  Transport(const Transport &t){
    cost = t.get_cost();
    duration = t.get_duration();
    company = t.get_company();
    stops_num = t.get_stops_num();
    route_stops = new string [stops_num];
    for (int i=0; i<stops_num; ++i){
      route_stops[i] = t.get_route()[i];
    }
    cout << "transport from transport made" << endl;
    ++counter;
  }

  Transport(int n_cost, int n_duration, string& n_company, const string* n_route, int n_num_stops){
    cost = n_cost;
    duration = n_duration;
    company = n_company;
    stops_num = n_num_stops;

    route_stops = new string [stops_num];
    for (int i=0; i<stops_num; ++i){
      route_stops[i] = n_route[i];
    }

    cout << "transport from variables made" << endl;
    ++counter;
  }

  Transport& operator=(const Transport &t){
    cost = t.get_cost();
    duration = t.get_duration();
    company = t.get_company();
    stops_num = t.get_stops_num();

    route_stops = new string [stops_num];
    for (int i=0; i<stops_num; ++i){
      route_stops[i] = t.get_route()[i];
    }

    cout << "transport with = made" << endl;
    return *this;
  }

  int get_cost() const {return cost;}
  int get_duration() const {return duration;}
  string get_company() const {return company;}
  const string* get_route() const { return route_stops; }
  int get_stops_num() const {return stops_num;}
  int get_counter() const {return counter;}

  void PrintRoute(){
    for(int i=0; i<stops_num; ++i) cout << i+1 << ". " << route_stops[i] << endl;
  }

  virtual ostream& Print(ostream &out) const = 0;
  virtual void NeededDocuments() = 0;
  friend ostream& operator<<(ostream &out, const Transport &t) {
    return t.Print(out);
  }

  void operator() (int time_left) const {
    if(time_left > duration) cout << "You'll be late" << endl;
    else cout << "You'll get on time" << endl;
  }

  virtual ~Transport(){
    delete [] route_stops;
    counter--;
    cout << "transport destructed" << endl;
  }

};

int Transport::counter = 0;

class Bus : public Transport {
  int number;
 public:

  Bus() : Transport () {number = 0; cout << "bus basic made" << endl;}

  Bus(const Bus &b) : Transport(b) {
    number = b.get_number();
    cout << "bus from bus made" << endl;
  }

  Bus(int n_cost, int n_duration, string& n_company, const string* n_route, int n_num_stops, int n_number) :
  Transport (n_cost, n_duration, n_company, n_route, n_num_stops){
    number = n_number;
    cout << "bus from variable made" << endl;
  }

  Bus& operator=(const Bus& b) {
    if (this != &b) {
      number = b.get_number();
      cost = b.get_cost();
      duration = b.get_duration();
      company = b.get_company();
      stops_num = b.get_stops_num();

      delete[] route_stops;
      route_stops = new string[stops_num];
      for (int i = 0; i < stops_num; ++i) {
        route_stops[i] = b.get_route()[i];
      }
    }
    cout << "bus with = made" << endl;
    return *this;
  }

  int get_number() const {return number;}

  ostream& Print(ostream &out) const override {
    out << "Bus number " << number << " from " << this->get_company() << " rides for " << this->get_duration() <<
        " minutes for " << this->get_cost() << " rubles." << endl;
    return out;
  }

  void NeededDocuments() override {
    cout << "You'll need " << this->get_cost() << " rubles and maybe a social card" << endl;
  }

  ~Bus() override {
    cout << "bus destructed" << endl;
  }
};


class Plane : public Transport {
  string number;
 public:

  Plane() : Transport () {number = "-"; cout << "plane basic made" << endl;}

  Plane(const Plane &p) : Transport(p) {
          number = p.get_number();
          cout << "plane from plane made" << endl;
  }

  Plane(int n_cost, int n_duration, string& n_company, const string* n_route, int n_num_stops, string& n_number) :
          Transport (n_cost, n_duration, n_company, n_route, n_num_stops){
    number = n_number;
    cout << "plane from variable made" << endl;
  }

  Plane& operator=(const Plane& p) {
    if (this != &p) {
      number = p.get_number();
      cost = p.get_cost();
      duration = p.get_duration();
      company = p.get_company();
      stops_num = p.get_stops_num();

      delete[] route_stops;
      route_stops = new string[stops_num];
      for (int i = 0; i < stops_num; ++i) {
        route_stops[i] = p.get_route()[i];
      }
    }
    cout << "plane with = made" << endl;
    return *this;
  }

  string get_number() const {return number;}

  ostream& Print(ostream &out) const override {
    out << "Flight number " << number << " of " << this->get_company() << " from "
        << ((this->get_route() == nullptr)?(" - "):(this->get_route()[0]));
    out << " to " << ((this->get_route() == nullptr)? " - " : this->get_route()[1]) ;
    out << " will be on the go for " << this->get_duration()<< " minutes." << endl;
    return out;
  }

  void NeededDocuments() override {
    cout << "You'll need " << this->get_cost() << " rubles, passport and ID" << endl;
  }

  ~Plane() override {
    cout << "plane destructed" << endl;
  }
};

int main() {
  Transport::counter = 0;

  string * road = new string[2];
  auto * road2 = new string[2];
  road[0] = "a";
  road[1] = "b";
  road2[0] = "a";
  road2[1] = "b";
  string comp = "M";
  string numb = "A007";

  Bus b1(35, 20, comp, road, 2, 15);
  b1.NeededDocuments();
  b1.PrintRoute();
  b1(20);

  Bus b2 = b1;
  Bus b3, b4;
  b3 = b1;

  cout << Transport::counter << endl << b1 << b2 << b3 << b4;

  Plane p1(180, 20, comp, road2, 2, numb);
  Plane p2=p1, p3, p4;
  p3=p1;

  p1.NeededDocuments();
  p1.PrintRoute();
  p1(200);

  cout << Transport::counter << endl << p1 << p2 << p3<< p4 << endl;

  return 0;
}