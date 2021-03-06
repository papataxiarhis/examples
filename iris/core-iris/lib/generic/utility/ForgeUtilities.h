#ifndef FORGEUTILITIES_H_
#define FORGEUTILITIES_H_

#include "ControllerUtilities.h"

#include <iostream>
#include <string>
#include <set>
#include <list>
#include <complex>
#include <iterator>
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread/mutex.hpp>

#include "../../../build/lib/generic/utility/anvil_pkt.pb.h"
#include "ProtobufSerializationUtilities.h"


//class AnvilProtoPacket;

namespace iris {

using namespace std;
namespace bip = boost::asio::ip;
typedef complex<float> Cplx;

namespace tcp_helper {
using namespace bip;

void send_n_bytes(tcp::socket &socket, const char *_buf, std::size_t n);
void send_n_bytes(tcp::socket &socket, const string &str);
int recv_n_bytes(tcp::socket &socket, char *_buf, std::size_t n);
int recv_endline(tcp::socket &socket, char *_buf, int size);

class TcpSocket {
protected:
	boost::asio::io_service ioService;
	tcp::socket* socket;

public:
	template <typename Iterator> void write(Iterator begin, Iterator end);
	int write(const char *buf, int bufSize);
	template <typename Iterator> int read(Iterator begin, Iterator end);
	int read(char *buf, int bufSize);
	int read_line(string &buf);
};

class TcpSocketClient : public TcpSocket {
    bool open;
public:
	TcpSocketClient(std::string _address, int _port);
	~TcpSocketClient();
        inline bool is_open() {
            return open;
        }
};

class TcpConnection
  : public boost::enable_shared_from_this<TcpConnection>
{
public:
  typedef boost::shared_ptr<TcpConnection> pointer;
  typedef boost::function<void(std::string &)> msg_ready_callback;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new TcpConnection(io_service));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start_read(msg_ready_callback);
  void start_write(const std::string &message);

private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::streambuf input_buffer_;

  TcpConnection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }


  //void handle_read(const boost::system::error_code &, size_t bytes_transferred, msg_ready_callback);
  void handle_read(size_t bytes_transferred, msg_ready_callback);
  void handle_write(const boost::system::error_code &ec, size_t bytes_transferred) {
	std::cout << "Transfer " << bytes_transferred << " bytes" << std::endl;
  };
  void close_connection();

};

class TcpSocketServer : public TcpSocket {
private:
	tcp::acceptor *acceptor;

public:
	TcpSocketServer(int _port);
	~TcpSocketServer();
};
}

namespace socket_command {
using namespace bip;

std::string create_forgecommand_string(const std::string &command_name, const std::string &param_value = "");

/**
 * Class that sends an configuration event with the format "nameofevent:value" through a socket and receives "ok" as confirmation event
 * The connections are not persistent. So after the "ok" the receiver closes the socket
 */
class TCPEventTransmitter {
	int port_x;
	std::string address_x;
	tcp_helper::TcpSocketClient *socket;
	char ack_buf[4];

public:
	TCPEventTransmitter(int _port_x, const std::string &_address_x);

	void create_socket();
	void erase_connection() {
		delete socket;
		socket = NULL;
	}
	void close_connection() {
		delete socket;
		//socket->close();
		//erase_connection();
	}
	void send_event(const std::string &event_name, double value);
	void send_event(const std::string &event_name, const std::string &value);
	void send_event(const std::string &event_string);
};

// OLD implementation of the TCPEventReceiver
#if 0
class TCPEventReceiver {
	int port_x;
	tcp_helper::TcpSocketServer *socket;

public:
	TCPEventReceiver(int _port_x);
	void start_server();
	void close_connection() {
		delete socket;
		socket = NULL;
	}

	bool receive_command(std::list<std::pair<std::string, std::string> > &event_list);
};
#endif 

class TCPEventReceiver {
public:
	TCPEventReceiver(boost::asio::io_service &ios, int _port_x);
	void start_server();
	void close_connection() {
		//delete socket;
		//socket = NULL;
	}

	void receive_command(std::string &receive_cmd);
	bool get_events(std::list<std::pair<std::string, std::string> > &ev);

private:
	int port_x;
	boost::asio::io_service &io_service;

	std::list<std::pair<std::string, std::string> > event_list_;
	tcp::acceptor *acceptor_;

	void start_accept();
	void handle_accept(tcp_helper::TcpConnection::pointer new_connection, const boost::system::error_code& error);

	TCPEventReceiver& operator=(TCPEventReceiver arg){ return *this;};
	TCPEventReceiver( const TCPEventReceiver &obj):io_service(obj.io_service){};
};

}

namespace anvil_interface {
using namespace bip;
using namespace tcp_helper;

enum DATA_TYPE {CPLX_DATA, FLOAT_DATA, PSD_DATA};
//enum STREAM_TYPE {FIXED_SIZE, PSD};

// Deprecated. Now we use protobuf
/*
class AnvilTcpTx {
	std::string address_x;
	int port_x;
	TcpSocketClient* tx_socket;
	map<char, DATA_TYPE> sessions;
	vector<char> buffer;
public:
	AnvilTcpTx(std::string, int);
	void initiate_session(char tag, DATA_TYPE type);
	void initiate_psd_session(char tag, float min_freq, float max_freq, int Nbins);
	//template <typename Iterator> void write(Iterator begin, Iterator end, char tag);
	void write(const char *, const int, const char);
	~AnvilTcpTx() {
		delete tx_socket;
	}
};*/

void anvil_add_protopsdheader(AnvilProtoPacket &pkt, int Nbins, float fmax, float fmin);

class AnvilProtobufTcpTx {
private:
	std::string address_x;
	int port_x;
	TcpSocketClient* tx_socket;
	map<AnvilTag, DATA_TYPE> sessions;

public:
	AnvilProtobufTcpTx(std::string, int);
	void initiate_session(AnvilTag tag, DATA_TYPE type);
	void initiate_psd_session(AnvilTag tag, float min_freq, float max_freq, int Nbins);
	void set_psd_param(float min_freq, float max_freq, int Nbins);
	//template <typename Iterator> void write(Iterator begin, Iterator end, char tag);
	bool write(const AnvilProtoPacket& message);
	bool write_pkt(const vector<float> &data, AnvilTag tag);
	bool write_pkt(const vector<Cplx> &data, AnvilTag tag); // Comment: I run into some problems with templates...
	bool write_pkt(const vector<boost::any> &data, AnvilTag tag);
        void close_socket() {
            if(is_open()) {
		delete tx_socket;
                tx_socket = NULL;
            }
        }
	~AnvilProtobufTcpTx() {
            close_socket();
	}
        inline bool is_open() {
            return tx_socket != NULL;
        }
};

class ctrl_data {
public:
	int Nbins;
	float min_freq, max_freq;
	ctrl_data(float _min_freq, float _max_freq, int _Nbins) : min_freq(_min_freq), max_freq(_max_freq), Nbins(_Nbins) {}
};

class session_data {
public:
	vector<string> frames;
	int n_frames;
	DATA_TYPE dt;
	ctrl_data *ctrl_ptr;
	inline int n_rcv_frames() {int i, count = 0; for(i = 0; i < n_frames; i++) if(frames[i] != "") count++; return count;}
	inline bool is_complete() { return n_rcv_frames() == n_frames; }
	session_data(DATA_TYPE _dt) : dt(_dt), ctrl_ptr(NULL) {}
};

/* Deprecated
class AnvilTcpRx {
	int port_x;
	TcpSocketServer* rx_socket;
	map<char, session_data> sessions;
	char buffer[1024];
public:
	AnvilTcpRx(int);
	void read(char *buf, int size);
	DATA_TYPE guess_data_type(const char tag);
	std::string read_pkt();
	char read_frame();
};*/

/*class AnvilUdpTx {
	boost::scoped_ptr<UdpSocketTransmitter> tx_socket;
public:
	AnvilUdpTx(std::string address_x, int port_x) {
		tx_socket.reset(new UdpSocketTransmitter(address_x, port_x));
	}

	template <typename Iterator>
	void write(Iterator begin, Iterator end) {
		send_n(begin, end, tx_socket);
	}
};

class AnvilPsdUdpTx {
	boost::scoped_ptr<UdpSocketTransmitter> tx_socket;
	float min_freq, max_freq;
	int Nbins;
	bool first_time;

public:
	AnvilPsdUdpTx(std::string address_x, int port_x, float _min_freq, float _max_freq, int _Nbins): min_freq(_min_freq), max_freq(_max_freq), Nbins(_Nbins) {
	    tx_socket.reset(new UdpSocketTransmitter(address_x, port_x));
	    first_time = true;
	}

	template <typename Iterator>
	void write(Iterator begin, Iterator end) {
		if(first_time) {
			std::string str;
			union {
				float a;
				char bytes[4];
			} thing;
			thing.a = min_freq;
			str.assign(thing.bytes, 4);
			tx_socket->write(str.begin(), str.end());
			thing.a = max_freq;
			str.assign(thing.bytes, 4);
			tx_socket->write(str.begin(), str.end());
			numToString(str, Nbins);
			tx_socket->write(str.begin(), str.end());
			first_time = false;
		}
		tx_socket->write(begin, end);
	}
};*/

}

}

#endif
