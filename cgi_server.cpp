#include "cgi_server.hpp"

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;

        server s(io_context, std::atoi(argv[1]));

        io_context.run();
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

/*******************************
 *   User_Info_Table
 **********************************/

void User_Info_Table::parsing(std::string query_str) {

    std::cerr << "Parsing: query_string = " << query_str << "\n";

    std::stringstream ss(query_str);

    std::string tmp = "";
    for (int i = 0; i < 5; ++i) {
        // processing host
        std::getline(ss, tmp, '&');
        size_t equal_symbol_pos = tmp.find('=');
        if (equal_symbol_pos != std::string::npos) {
            user_info_table[i].URL = tmp.substr(equal_symbol_pos + 1);
        } else {
            std::cerr << "Equal sign not found , when processing user-" << i
                      << " host" << std::endl;
        }

        // processing port
        std::getline(ss, tmp, '&');
        equal_symbol_pos = tmp.find('=');
        if (equal_symbol_pos != std::string::npos) {
            user_info_table[i].port = tmp.substr(equal_symbol_pos + 1);
        } else {
            std::cerr << "Equal sign not found , when processing user-" << i
                      << " port" << std::endl;
        }

        // processing file_path
        std::getline(ss, tmp, '&');
        equal_symbol_pos = tmp.find('=');
        if (equal_symbol_pos != std::string::npos) {
            user_info_table[i].file_path = tmp.substr(equal_symbol_pos + 1);
        } else {
            std::cerr << "Equal sign not found , when processing user-" << i
                      << " file_path" << std::endl;
        }

        if (!user_info_table[i].URL.empty() &&
            !user_info_table[i].port.empty() &&
            !user_info_table[i].file_path.empty()) {
            user_count++;
        }
    }

    std::cerr << "After parsing: user count = " << user_count << "\n";
    for (int i = 0; i < MAX_USERS; i++) {
        std::cerr << "URL: " << user_info_table[i].URL << "\n";
        std::cerr << "port: " << user_info_table[i].port << "\n";
        std::cerr << "file_path: " << user_info_table[i].file_path << "\n";
    }
}

/*******************************
 *   Client Session
 *********************************/

void client_session::do_read() {
    auto self(shared_from_this());
    socket_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string request(data_, length);
                std::cout << "request: " << request << std::endl;
                memset(data_, 0, max_length);

                HTTP_request_parser(request);

                do_CGI();
            }
        });
}

void client_session::do_CGI() {
    std::string exec = get_exec();
    if (exec == "./panel.cgi") {
        show_panel();
    } else if (exec == "./console.cgi") {
        // TODO: implement the console.cgi
        std::cout << "exec = console.cgi" << std::endl;

        User_Info_Table::getInstance().parsing(env_map["QUERY_STRING"]);
        show_console();

    } else {
        std::cerr << "Unknown CGI: " << exec << std::endl;
        socket_.close();
    }

}

void client_session::HTTP_request_parser(std::string req) {
    std::istringstream iss(req);
    std::string line = "";

    /**********************
     *   Split the first line
     ***********************/
    std::getline(iss, line); // Read the first line of the request
    std::istringstream line_stream(line);
    std::string method, uri, protocol;
    line_stream >> method >> uri >> protocol;

    // Update the env_map with the parsed values
    env_map["REQUEST_METHOD"] = method;
    env_map["SERVER_PROTOCOL"] = protocol;
    env_map["REQUEST_URI"] = uri;

    if (uri.find("?") != std::string::npos) {
        std::string query_string = uri.substr(uri.find("?") + 1);
        env_map["QUERY_STRING"] = query_string;
    }

    /**********************
     *   Split the 2-nd line
     ***********************/
    std::getline(iss, line); // Read the second line of the request
    std::istringstream line_stream2(line);
    std::string host, _trash;
    line_stream2 >> _trash >> host;
    env_map["HTTP_HOST"] = host;
    env_map["REMOTE_ADDR"] = socket_.remote_endpoint().address().to_string();
    env_map["REMOTE_PORT"] = std::to_string(socket_.remote_endpoint().port());

    // Split the host into two parts
    size_t colon_pos = host.find(":");
    if (colon_pos != std::string::npos) {
        std::string server_addr = host.substr(0, colon_pos);
        std::string server_port = host.substr(colon_pos + 1);
        env_map["SERVER_ADDR"] = server_addr;
        env_map["SERVER_PORT"] = server_port;
    }

    print_env_map();
}

void client_session::show_panel() {
    auto self(shared_from_this());
    std::string panel_page = get_panel_page();
    boost::asio::async_write(
        socket_, boost::asio::buffer(panel_page, panel_page.length()),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::cout << "panel page sent successfully" << std::endl;
            }
        });
}

void client_session::show_console() {
    auto self(shared_from_this());
    std::string console_page = get_console_basic_framwork();
    boost::asio::async_write(
        socket_, boost::asio::buffer(console_page, console_page.length()),
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::cout << "console page sent successfully" << std::endl;
            }
    });
}

/*******************************
 *   Helper Functions
 *********************************/

std::string get_panel_page() {
    using namespace std;
    string panel_part1 = R"(
	<!DOCTYPE html>
	<html lang="en">
	  <head>
	    <title>NP Project 3 Panel</title>
	    <link
	      rel="stylesheet"
	      href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
	      integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
	      crossorigin="anonymous"
	    />
	    <link
	      href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
	      rel="stylesheet"
	    />
	    <link
	      rel="icon"
	      type="image/png"
	      href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
	    />
	    <style>
	      * {
	        font-family: 'Source Code Pro', monospace;
	      }
	    </style>
	  </head>
	  <body class="bg-secondary pt-5">
	  	<form action="console.cgi" method="GET">
	      <table class="table mx-auto bg-light" style="width: inherit">
	        <thead class="thead-dark">
	          <tr>
	            <th scope="col">#</th>
	            <th scope="col">Host</th>
	            <th scope="col">Port</th>
	            <th scope="col">Input File</th>
	          </tr>
	        </thead>
	        <tbody>
	)";

    string host_menu;
    for (int i = 0; i < 12; i++) {
        host_menu += "<option value=\"nplinux" + to_string(i + 1) +
                     ".cs.nycu.edu.tw\">nplinux" + to_string(i + 1) +
                     "</option>";
    }

    string panel_part2;
    int N_SERVERS = 5;
    for (int i = 0; i < N_SERVERS; i++) {
        panel_part2 += (boost::format(R"(
			<tr>
	          <th scope="row" class="align-middle">Session %1%</th>
	          <td>
	            <div class="input-group">
	              <select name="h%2%" class="custom-select">
	                <option></option>"%3%"
	              </select>
	              <div class="input-group-append">
	                <span class="input-group-text">.cs.nycu.edu.tw</span>
	              </div>
	            </div>
	          </td>
	          <td>
	            <input name="p%2%" type="text" class="form-control" size="5" />
	          </td>
	          <td>
	            <select name="f%2%" class="custom-select">
	              <option></option>
	              <option value="t1.txt">t1.txt</option>
	              <option value="t2.txt">t2.txt</option>
	              <option value="t3.txt">t3.txt</option>
	              <option value="t4.txt">t4.txt</option>
	              <option value="t5.txt">t5.txt</option>
	            </select>
	          </td>
	        </tr>
			)") % to_string(i + 1) %
                        to_string(i) % host_menu)
                           .str();
    }
    string panel_part3 = R"(
			<tr>
	          <td colspan="3"></td>
	          <td>
	            <button type="submit" class="btn btn-info btn-block">Run</button>
	          </td>
	        </tr>
	      </tbody>
	    </table>
	  </form>
	  </body>
	  </html>
	)";
    return "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n" + panel_part1 + panel_part2 + panel_part3 + "\r\n\r\n";
}

std::string get_console_basic_framwork() {
    std::string ret;
    ret += "HTTP/1.1 200 OK\r\n";
    ret += "Content-type: text/html\r\n\r\n";
    ret += R"(
    <!DOCTYPE html>
    <html lang="en">
        <head>
            <meta charset="UTF-8" />
            <title>NP Project 3 Console</title>
            <link
                rel="stylesheet"
                href="https://cdn.jsdelivr.net/npm/bootstrap@4.5.3/dist/css/bootstrap.min.css"
                integrity="sha384-TX8t27EcRE3e/ihU7zmQxVncDAy5uIKz4rEkgIXeMed4M0jlfIDPvg6uqKI2xXr2"
                crossorigin="anonymous"
            />
            <link
                href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
                rel="stylesheet"
            />
            <link
                rel="icon"
                type="image/png"
                href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
            />
            <style>
            * {
                font-family: 'Source Code Pro', monospace;
                font-size: 1rem !important;
            }
            body {
                background-color: #212529;
            }
            pre {
                color: #cccccc;
            }
            b {
                color: #FFC249;
            }
            </style>
        </head>
        <body>
            <table class="table table-dark table-bordered">
                <thead>
                    <tr id="table_head">
                    </tr>
                </thead>
                <tbody>
                    <tr id="table_body">
                    </tr>
                </tbody>
            </table>
        </body>
    </html>
    )";
    
    // construct specific user info
    for (int i = 0; i < User_Info_Table::getInstance().user_count; i++) {
        std::string URL = User_Info_Table::getInstance().user_info_table[i].URL;
        std::string port =
            User_Info_Table::getInstance().user_info_table[i].port;
        ret += "<script>document.querySelector('#table_head').innerHTML += '";
        ret += R"(<th scope=\"col\">)" + URL + ":" + port + "</th>";
        ret += "';</script>";
        
        // table body
        std::string msg = R"(<td><pre id="user_)" + std::to_string(i) + R"(" class="mb-0"></pre></td>)";
        ret += "<script>document.querySelector('#table_body').innerHTML += '";
        ret += msg;
        ret += "';</script>";
    }
    
    return ret;
}
