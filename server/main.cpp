#include <stdio.h>
#include <iostream>

#include <iostream>
#include <fstream>
#include <iterator>
#include <exception>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/progress.hpp>
#include <boost/tokenizer.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/cstdint.hpp>

#include "libtorrent/session.hpp"
#include "libtorrent/torrent_server.hpp"

using namespace libtorrent;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "usage: " << argv[0] << " <torrent file>" << std::endl;
		return -1;
	}

	try
	{
		add_torrent_params p;
		boost::system::error_code ec;

		// open torrent file for download.
		{
			p.ti = new torrent_info(argv[1], ec);
			if (ec) {
				std::cout << ec.message() << std::endl;
				return -1;
			}
		}

		// 创建libtorrent::session对象.
		session s;
		session *session_obj = &s;
		session_obj->add_dht_router(std::make_pair(
			std::string("router.bittorrent.com"), 6881));
		session_obj->add_dht_router(std::make_pair(
			std::string("router.utorrent.com"), 6881));
		session_obj->add_dht_router(std::make_pair(
			std::string("router.bitcomet.com"), 6881));
		session_obj->start_dht();

		// session_obj->load_asnum_db("GeoIPASNum.dat");
		// session_obj->load_country_db("GeoIP.dat");

		session_obj->listen_on(std::make_pair(6881, 6889));

		// 设置缓冲.
		session_settings settings = session_obj->settings();
		// settings.use_read_cache = false;
		// settings.disk_io_read_mode = session_settings::disable_os_cache;
		// settings.broadcast_lsd = true;
		settings.allow_multiple_connections_per_ip = true;
		settings.local_service_announce_interval = 15;
		settings.min_announce_interval = 20;
		session_obj->set_settings(settings);

		// 添加torrent.
		torrent_handle h = s.add_torrent(p, ec);

		// 设置为新的下载策略.
		h.set_user_defined_download(true);

		// 创建torrent服务器, 将session作为参数传入.
		torrent_server serv(s);

		// 创建listen端口.
		serv.create_server(8889);

		// 这里一直处于等待状态, 或做其它事.
		Sleep(INFINITE);

		// 如果需要退出, 请调用destory_server即可.
		serv.destory_server();

		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
