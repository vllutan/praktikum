#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <vector>
#include <unistd.h>
#include <ctime>
#include <fstream>
#include <sys/stat.h>

using namespace std;

const int BACKLOG = 5;

class SocketAddress {     // IP and port number
  struct sockaddr_in saddr;
 public:
  SocketAddress(){}
  SocketAddress(const char* ip, short port) {
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(ip);
  }

  SocketAddress(short port){
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }

  struct sockaddr* get_addr() const {return (sockaddr*)&saddr;}
};

class Socket{
 protected:
  int sd_;
  explicit Socket(int sd) : sd_(sd) {}
 public:
  Socket(){
    sd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (sd_ < 0) throw "Incorrect sd_";
  }

  const int get_sd_() const {return sd_;}

  void shut_down() {shutdown(sd_, 2);}

  ~Socket() {close(sd_);}
};

class ConnectedSocket : public Socket {
 public:
  ConnectedSocket() : Socket() {}
  explicit ConnectedSocket(int sd) : Socket(sd) {}

  void write__(const string& s) {                                                 //send
    if( send(sd_, s.c_str(), s.length(), 0) < 0 ) throw "send failed";
  }
  void read__(string& s) {                                                        //recv
    int len = 1024;
    char buf[len];
    if (recv(sd_, buf, len, 0) < 0) throw "read failed";
    s = buf;
  }
  void write__(const std::vector<uint8_t> &bytes){                                //send
    if( send(sd_, bytes.data(), bytes.size(), 0) < 0 ) throw "write(vector) failed";
  }

  ~ConnectedSocket() = default;
};


class ServerSocket : public Socket {
 public:
  ServerSocket() : Socket() {}

  void bind__(const SocketAddress& ipAddr) {                                      //bind
    if( bind(sd_, ipAddr.get_addr(), sizeof(ipAddr)) < 0 ) throw "bind failed";
  }
  int accept__(SocketAddress& clAddr){                                            //accept
    int len = sizeof(clAddr);
    int new_sd = accept(sd_, clAddr.get_addr(), (socklen_t*)&len);
    if (new_sd < 0) throw "accept failed";
    return new_sd;
  }
  void listen__(int backlog){                                                      //listen
    if( listen(sd_, backlog) < 0 ) throw "listen failed";
  }

  ~ServerSocket() = default;
};

class ClientSocket : public ConnectedSocket {
 public:
  ClientSocket() : ConnectedSocket() {}

  void connect__(const SocketAddress& addr){                                       //connect
    if (connect(sd_, addr.get_addr(), sizeof(addr)) < 0) throw "connect failed";
  }

  ~ClientSocket() = default;
};

class HttpHeader{
  string name;
  string value;
 public:
  HttpHeader(const string& n, const string& v) : name(n), value(v) {}
  HttpHeader(const HttpHeader& h) {
    name = h.name;
    value = h.value;
  }

  static HttpHeader parse_header(const string& line){
    int i = 0;
    string new_name, new_value;
    if(!line.empty()){
      while(line[i] != ':'){
        new_name += line[i];
        i++;
      }
      new_name += '\0';

      i++;
      while(i < line.length()){
        new_value += line[i];
        i++;
      }
    } else {
      new_name = " ";
      new_value = " ";
    }

    return HttpHeader(new_name, new_value);
  }

  string to_string() const {
    return name + ":" + value;
  }

};

/*class HttpRequest{
  std::vector<string> lines;
  // [0] - method, uri, version (starting line)
  // [i] - headers

 public:
  HttpRequest(){
    lines = {"GET /tset.txt HTTP/1.1\n"};
  }

  string to_string(){
    string res;
    for (int i=0; i < lines.size(); ++i){
      res += lines[i];
    }
    return res;
  }
};

class HttpResponse{
  string status;
 public:
// finish
}; */

std::vector<string> split_lines(const string& line){
  std::vector<string> res;
  string tmp;
  size_t begin_pos=0, end_pos=0;

  while( (end_pos = line.find("\r\n", begin_pos)) != -1){
    tmp = line.substr(begin_pos, end_pos + 2 - begin_pos);
    res.push_back(tmp);
    begin_pos = end_pos + 2;
  }

  res.push_back("");
  return res;
}

string parse_path(string &line){
  size_t beg;
  string path = "";

  for(beg=0; line[beg] != '/'; ++beg){}

  for(int i=1; (line[beg + i] != ' ') && (line[i] != '\n'); ++i){
    path += line[beg + i];
  }

  return path;
}


void ProcessConnection(int cd, const SocketAddress &clAddr){
  ConnectedSocket cs(cd);
  string request, response, method="", header;
  int code;

  cs.read__(request);
  int fd_log = open("log.txt", O_RDWR | O_CREAT | O_APPEND, 0644);
  write(fd_log, request.c_str(), request.length());
  close(fd_log);

  std::vector<string> lines = split_lines(request);
  for(int i=0; lines[0][i] != ' '; ++i) method += lines[0][i];
  string path = parse_path(lines[0]);

  int fd = open(path.c_str(), O_RDONLY);                         //default_connection (later with cgi choice)
  if(fd < 0) {
    response = "HTTP/1.1 404 NotOk"; code = 404;
    fd = open("404.html", O_RDONLY);
    if (fd < 0) throw "no 404 error file";
  }
  else {response = "HTTP/1.1 200 Ok"; code = 200; }
  cs.write__(response);

  std::vector<uint8_t> body;
  char c;
  while( read(fd, &c, 1) ) body.push_back(c);             // file to vector

  time_t rawtime;                                         // get Date
  struct tm * timeinfo;
  char buffer[40];
  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (buffer,40,"%a, %e %h %G %T GMT",timeinfo);    //  end get Date

  header = "Version: HTTP/1.1\r\nDate: " + (string)buffer + "\r\nServer: Apache/2.4.41 (Ubuntu)\r\n";

  string content_type="";                                 // get Content-Type
  if((path.rfind("jpg", path.length())!=-1) || (path.rfind("jpeg", path.length())!=-1) || (path.rfind("png", path.length())!=-1)){
    content_type = "image/jpeg";
  } else if(path.rfind("html", path.length())!=-1){
    content_type = "text/html";
  } else content_type = "text/plain";


  if(code == 200) {
    struct stat buff;                                        // get Last-Modified
    stat(path.c_str(), &buff);
    struct tm * timeinfo1;
    char buffer1[40];
    time (&buff.st_mtime);
    timeinfo1 = localtime (&buff.st_mtime);
    strftime (buffer1,40,"%a, %e %h %G %T GMT",timeinfo1);   // end get Last-Modified

    header += "Last-Modified: " + (string)buffer1;

    if(method == "GET"){
      header += "\r\nContent-Type: " + content_type + ";charset=utf-8\r\nContent-Length: " + to_string(body.size()) + "\r\n\r\n";

      cs.write__(header);
      cs.write__(body);
    } else if (method == "HEAD") {
      header += "\r\n\r\n";

      cs.write__(header);
    } else throw "code error";
  } else if (code == 404) {
    header += "Content-Type: " + content_type + ";charset=utf-8\r\nContent-Length: " + to_string(body.size()) + "\r\n\r\n";

    cs.write__(header);
    cs.write__(body);
  }

  cout << header << endl;

  close(fd);
  cs.shut_down();
}

void ServerLoop(){                                                  //server_main cycle
  try {
    int fd_log = open("log.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd_log < 0) throw "cannot open log";
    close(fd_log);

    SocketAddress serAddr("127.0.0.1", 1234);
    ServerSocket ss;
    ss.bind__(serAddr);
    ss.listen__(BACKLOG);

    for (;;) {
      SocketAddress clAddr;
      int cd = ss.accept__(clAddr);
      ProcessConnection(cd, clAddr);
    }
  }
  catch(const char* s){cout << "\nmy error: " << s << endl;}
}


// int main() { cout << "Hello, World!" << endl; return 0; }