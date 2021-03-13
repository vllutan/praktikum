#include <iostream>
#include <cstring>
#include <cstdlib>

using namespace std;

class Book {
  string name;
  string author;
  int num_page;
  string release_year;
 public:
  Book() {
    name = "-";
    author = "-";
    num_page = -1;
    release_year = "-";
  };

  Book(string& b_name, string& b_author, int b_num_page, string& b_release_year){
    name = b_name;
    author = b_author;
    num_page = b_num_page;
    release_year = b_release_year;
  }

  Book(const Book &b){
    name = b.get_name();
    author = b.get_author();
    num_page = b.get_page();
    release_year = b.get_year();
  }

  const string get_name() const {return name;}
  const string get_author() const {return author;}
  const int get_page() const {return num_page;}
  const string get_year() const {return release_year;}

  Book& operator=(const Book &b){
    name = b.get_name();
    author = b.get_author();
    num_page = b.get_page();
    release_year = b.get_year();
    return *this;
  }

  ~Book(){}

};

class Library {
  int size;
  Book *book_array;
 public:
  Library() {size = 0; book_array = new Book [1];}
  Library(const Library &l) {
    size = l.get_size();
    book_array = new Book [size];
    for(int i=0; i<size; ++i){
      book_array[i] = l.get_n(i);
    }
  }

  const int get_size() const {return size;}
  const Book get_n(const int n) const {
    if (n < size) return book_array[n];
    else {
      cerr << "Size error in get_n: n bigger than size" << endl;
      Book b;
      return b;
    }
  }

  int* find_by_name(const string& needed_name){
    int *index_arr = new int [1];
    int arr_size = 0;
    for (int i=0; i<size; ++i){
      if(book_array[i].get_name() == needed_name){
        ++arr_size;
        int *arr = new int[arr_size];
        for(int j = 0; j < arr_size - 1; ++j) {arr[j] = index_arr[j];}
        arr[arr_size - 1] = i;
        delete[] index_arr;
        index_arr = arr;
      }
    }
    index_arr[arr_size] = -1;
    return index_arr;
  }

  int* find_by_author(const string& needed_author){
    int *index_arr = new int [1];
    int arr_size = 0;
    for (int i=0; i<size; ++i){
      if(book_array[i].get_author() == needed_author){
        ++arr_size;
        int *arr = new int[arr_size];
        for(int j = 0; j < arr_size - 1; ++j) {arr[j] = index_arr[j];}
        arr[arr_size - 1] = i;
        delete[] index_arr;
        index_arr = arr;
      }
    }
    index_arr[arr_size] = -1;
    return index_arr;
  }

  int* find_by_page(const int& needed_page){
    int *index_arr = new int [1];
    int arr_size = 0;
    for (int i=0; i<size; ++i){
      if(book_array[i].get_page() == needed_page){
        ++arr_size;
        int *arr = new int[arr_size];
        for(int j = 0; j < arr_size - 1; ++j) {arr[j] = index_arr[j];}
        arr[arr_size - 1] = i;
        delete[] index_arr;
        index_arr = arr;
      }
    }
    index_arr[arr_size] = -1;
    return index_arr;
  }

  int* find_by_year(const string& needed_year){
    int *index_arr = new int [1];
    int arr_size = 0;
    for (int i=0; i<size; ++i){
      if(book_array[i].get_year() == needed_year){
        ++arr_size;
        int *arr = new int[arr_size];
        for(int j = 0; j < arr_size - 1; ++j) {arr[j] = index_arr[j];}
        arr[arr_size - 1] = i;
        delete[] index_arr;
        index_arr = arr;
      }
    }
    index_arr[arr_size] = -1;
    return index_arr;
  }

  const int find_book(string& b_name, string& b_author, int& b_num_page, string& b_year) const {
    for (int i=0; i<size; ++i){
      if(book_array[i].get_name() == b_name && book_array[i].get_author() == b_author &&
         book_array[i].get_page() == b_num_page && book_array[i].get_year() == b_year){
        return i;
      }
    }
    return -1;
  }

  Library& add_book(const Book& new_book){
    size++;
    Book *arr = new Book [size];
    for(int j = 0; j < size - 1; ++j) {arr[j] = book_array[j];}
    arr[size - 1] = new_book;
    delete[] book_array;
    book_array = arr;
    return *this;
  }

  Library& delete_book(const int index){
    size--;
    Book *arr = new Book [size];
    for(int i=0; i<index; ++i){arr[i] = book_array[i];}
    for(int i=index; i<size; ++i){arr[i] = book_array[i+1];}
    delete [] book_array;
    book_array = arr;
    return *this;
  }

  Library& switch_book(Book& ind1, Book& ind2){
    Book temp = ind1;
    ind1 = ind2;
    ind2 = temp;
    return *this;
  }

  Library& sort_by_name(){
    int min;
    for(int i=0; i<(size-1); ++i){
      min = i;
      for(int j=i+1; j<size; ++j){
        if(book_array[min].get_name() > book_array[j].get_name()){ min = j; }
      }
      if(min != i){
        switch_book(book_array[i], book_array[min]);
      }
    }
    return *this;
  }

  ~Library(){
    delete [] book_array;
    book_array = nullptr;
  }

};

void Menu(){
  cout << "MENU:" << endl;
  cout << "1. Find books by name" << endl;
  cout << "2. Find books by author" << endl;
  cout << "3. Find books by number of pages" << endl;
  cout << "4. Find books by the year of release" << endl;
  cout << "5. Add book" << endl;
  cout << "6. Remove book" << endl;
  cout << "7. Sort by name (alphabetical order)" << endl;
  cout << "8. Print all" << endl;
  cout << "9. Exit" << endl;
  cout << "10. Print Menu" << endl;
}


int main() {
  Library L;
  int action;

  Menu();
  do {
    cout << "\n/print number (Menu - 10)/" << endl;
    cin >> action;
    //cin.ignore(10, '\n');
    switch (action) {
      case 1:{
        string name;
        cout << "Write down a name" << endl;
        cin >> name;
        int* arr = L.find_by_name(name);
        int i=0;
        while(arr[i] != -1) {cout << arr[i]+1 << " "; i++;}
        cout << endl;
        delete [] arr;

        break;
      }
      case 2:{
        string author;
        cout << "Write down the author" << endl;
        cin >> author;
        int* arr = L.find_by_author(author);
        int i=0;
        while(arr[i] != -1) {cout << arr[i]+1 << " "; i++;}
        cout << endl;
        delete [] arr;

        break;
      }
      case 3:{
        int pages;
        cout << "Write down a number of pages" << endl;
        cin >> pages;
        int* arr = L.find_by_page(pages);
        int i=0;
        while(arr[i] != -1) {cout << arr[i]+1 << " "; i++;}
        cout << endl;
        delete [] arr;

        break;
      }
      case 4:{
        string year;
        cout << "Write down a year" << endl;
        cin >> year;
        int* arr = L.find_by_year(year);
        int i=0;
        while(arr[i] != -1) {cout << arr[i]+1 << " "; i++;}
        cout << endl;
        delete [] arr;

        break;
      }
      case 5: {
        string name, author, num_pages, year;
        int num_pages_i;
        cout << "Write down a name of the book" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> name;
        cout << "Write down an author" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> author;
        cout << "Write down number of pages" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> num_pages;
        if (num_pages == "-") num_pages_i = -1;
        else num_pages_i = stoi(num_pages);
        cout << "Write down a year of release" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin>> year;

        if(L.find_book(name, author, num_pages_i, year) >= 0) {
          cout << "This book is already in the library" << endl;
        } else {
          Book new_b(name, author, num_pages_i, year);
          L.add_book(new_b);
        }
        break;
      }
      case 6:{
        string name, author, num_pages, year;
        int num_pages_i;
        cout << "Write down a name of the book" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> name;
        cout << "Write down an author" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> author;
        cout << "Write down number of pages" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin >> num_pages;
        if (num_pages == "-") num_pages_i = -1;
        else num_pages_i = stoi(num_pages);
        cout << "Write down a year of release" << endl;
        cout << "/if you want to skip any option, write ""-""/" << endl;
        cin>> year;

        int index;
        if ((index = L.find_book(name, author, num_pages_i, year)) < 0){
          cout << "This book doesn't exist in the library" << endl;
        } else {
          L.delete_book(index);
        }

        break;
      }
      case 7:{
        L.sort_by_name();
        for(int i=0; i<L.get_size(); ++i){
          cout << L.get_n(i).get_name() << " " << L.get_n(i).get_author() << " " <<
               L.get_n(i).get_page() << " " << L.get_n(i).get_year() << endl;
        }

        break;
      }
      case 8:{
        for(int i=0; i<L.get_size(); ++i){
          cout << i+1 << ".   " << L.get_n(i).get_name() << " " << L.get_n(i).get_author() << " " <<
          L.get_n(i).get_page() << " " << L.get_n(i).get_year() << endl;
        }
        break;
      }
      case 9: return 0;
      case 10: {Menu(); break;}
      //default: {
        //cout << "Try choosing again" << endl;
        //Menu();
        //continue;
      //}
    };
  } while(true);

  return 0;
}