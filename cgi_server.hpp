#include <boost/asio/buffer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/format.hpp>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unistd.h>
#include <utility>
#include <vector>
#include <string>
#include <map>

#define MAX_USERS 5
#define MAX_MESSAGE_LEN 4096
using namespace boost::asio;
using namespace boost::asio::ip;

extern boost::asio::io_context IOContext;


/*******************************
 *   User_Info & User_Info_Table
 **********************************/
typedef struct _ { // the user info disignated by the client in browser => aim
                   // to connect to "np_golden"
    std::string URL;
    std::string port;
    std::string file_path;
} User_Info;

class User_Info_Table {
  public:
    User_Info user_info_table[MAX_USERS];
    int user_count = 0;
    static User_Info_Table instance;

    static User_Info_Table &getInstance() {
        static User_Info_Table instance;
        return instance;
    }
    void parsing(std::string query_string); // parse the query string
  private:
};


/*******************************
 *   Shell_Connector
 **********************************/
class Shell_Connector : public std::enable_shared_from_this<Shell_Connector> {
  public:
    int user_id;

    Shell_Connector(int id, std::shared_ptr<tcp::socket> client_session_socket);
    void start();

  private:
    tcp::resolver resolver;
    std::ifstream file_in;
    tcp::socket my_socket;
    std::shared_ptr<tcp::socket> client_socket;
    char data[MAX_MESSAGE_LEN];

    void resolve_handler();
    void connect_handler(boost::asio::ip::tcp::resolver::results_type endpoints);
    void do_read();
    void do_write(std::string msg);
    void open_file(std::string file_name);
    void send_shell_output(int user_id, std::string content);
    void send_command_from_file(int user_id, std::string content);
};

/**********************
 *  client_session
***********************/

class client_session : public std::enable_shared_from_this<client_session> {
  public:
    client_session(tcp::socket socket) : socket_(std::move(socket)) { init_env_map(); }
    void start() { do_read(); }
    void print_env_map() {
        for (auto &pair : env_map) {
            std::cout << pair.first << ":  " << pair.second << std::endl;
        }
    }

  private:
    void do_read() ;
    void do_CGI() ;
    void HTTP_request_parser(std::string req);
    void init_env_map() {
        env_map["REQUEST_METHOD"] = "";
        env_map["REQUEST_URI"] = "";
        env_map["QUERY_STRING"] = "";
        env_map["SERVER_PROTOCOL"] = "";
        env_map["HTTP_HOST"] = "";
        env_map["SERVER_ADDR"] = "";
        env_map["SERVER_PORT"] = "";
        env_map["REMOTE_ADDR"] = "";
        env_map["REMOTE_PORT"] = "";
    }
    std::string get_exec() {
        std::string request_uri = env_map["REQUEST_URI"];
        request_uri += "?";
        size_t question_mark_pos = request_uri.find("?");
        std::string left = request_uri.substr(0, question_mark_pos);
        return "." + left;
    }
    void show_panel() ;
    void show_console() ;


    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];
    std::map<std::string, std::string> env_map;
};

/**********************
 *  Server
***********************/
class server {
  public:
    server(short port)
        : acceptor_(IOContext, tcp::endpoint(tcp::v4(), port)) {
        do_accept(); // accpet the connection when the server construct
    }

  private:
    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                std::cout << "accept a connection" << std::endl;
                if (!ec) {
                    std::make_shared<client_session>(std::move(socket))->start();
                }
                do_accept();
            });
    }

    tcp::acceptor acceptor_;
};

/**********************
 *  Helper Function
***********************/

std::string get_panel_page() ;
std::string get_console_basic_framwork() ;