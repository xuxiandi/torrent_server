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

#include "libtorrent/extensions/metadata_transfer.hpp"
#include "libtorrent/extensions/ut_metadata.hpp"
#include "libtorrent/extensions/ut_pex.hpp"
#include "libtorrent/extensions/smart_ban.hpp"

#include "libtorrent/bencode.hpp"
#include "libtorrent/alert_types.hpp"
#include "libtorrent/file.hpp"
#include "libtorrent/entry.hpp"

#include "libtorrent/session.hpp"
#include "libtorrent/torrent_server.hpp"

using namespace libtorrent;

int num_outstanding_resume_data = 0;

bool yes(libtorrent::torrent_status const&)
{ return true; }

int save_file(std::string const& filename, std::vector<char>& v)
{
	file f;
	error_code ec;
	if (!f.open(filename, file::write_only, ec)) return -1;
	if (ec) return -1;
	file::iovec_t b = {&v[0], v.size()};
	size_type written = f.writev(0, &b, 1, ec);
	if (written != int(v.size())) return -3;
	if (ec) return -3;
	return 0;
}

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
		std::string filename;

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

		std::vector<char> in;
		if (load_file(".ses_state", in, ec) == 0)
		{
			lazy_entry e;
			if (lazy_bdecode(&in[0], &in[0] + in.size(), e, ec) == 0)
				s.load_state(e);
		}

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

		session_obj->listen_on(std::make_pair(6881, 6881), ec, "");

		// 设置缓冲.
		session_settings settings = session_obj->settings();
		// settings.use_read_cache = false;
		// settings.disk_io_read_mode = session_settings::disable_os_cache;
		settings.broadcast_lsd = true;
		settings.allow_multiple_connections_per_ip = true;
		settings.local_service_announce_interval = 15;
		settings.min_announce_interval = 20;
		session_obj->set_settings(settings);

		lazy_entry resume_data;
		filename = combine_path(".", combine_path(".resume", to_hex(p.ti->info_hash().to_string()) + ".resume"));
		std::vector<char> buf;
		if (load_file(filename.c_str(), buf, ec) == 0)
			p.resume_data = &buf;

		// 添加torrent.
		torrent_handle h = s.add_torrent(p, ec);

		// 设置为新的下载策略.
		h.set_user_defined_download(true);

		// 创建torrent服务器, 将session作为参数传入.
		torrent_server serv(s);

		// 创建listen端口.
		serv.create_server(8889);

		// 这里一直处于等待状态, 或做其它事.
		getchar();

		// 如果需要退出, 请调用destory_server即可.
		serv.destory_server();

		// 保存resume.
		int num_paused = 0;
		int num_failed = 0;

		printf("saving resume data\n");

		s.pause();
		std::vector<torrent_status> temp;
		s.get_torrent_status(&temp, &yes, 0);
		for (std::vector<torrent_status>::iterator i = temp.begin();
			i != temp.end(); ++i)
		{
			torrent_status& st = *i;
			if (!st.handle.is_valid())
			{
				printf("  skipping, invalid handle\n");
				continue;
			}
			if (!st.has_metadata)
			{
				printf("  skipping %s, no metadata\n", st.name.c_str());
				continue;
			}
			if (!st.need_save_resume)
			{
				printf("  skipping %s, resume file up-to-date\n", st.name.c_str());
				continue;
			}

			// save_resume_data will generate an alert when it's done
			st.handle.save_resume_data();
			++num_outstanding_resume_data;
			printf("\r%d  ", num_outstanding_resume_data);
		}
		printf("\nwaiting for resume data [%d]\n", num_outstanding_resume_data);

		boost::filesystem::create_directory(".resume", ec);
		while (num_outstanding_resume_data > 0)
		{
			alert const* a = s.wait_for_alert(seconds(10));
			if (a == 0) continue;

			std::deque<alert*> alerts;
			s.pop_alerts(&alerts);
			std::string now = time_now_string();
			for (std::deque<alert*>::iterator i = alerts.begin()
				, end(alerts.end()); i != end; ++i)
			{
				// make sure to delete each alert
				std::auto_ptr<alert> a(*i);

				torrent_paused_alert const* tp = alert_cast<torrent_paused_alert>(*i);
				if (tp)
				{
					++num_paused;
					printf("\rleft: %d failed: %d pause: %d "
						, num_outstanding_resume_data, num_failed, num_paused);
					continue;
				}

				if (alert_cast<save_resume_data_failed_alert>(*i))
				{
					++num_failed;
					--num_outstanding_resume_data;
					printf("\rleft: %d failed: %d pause: %d "
						, num_outstanding_resume_data, num_failed, num_paused);
					continue;
				}

				save_resume_data_alert const* rd = alert_cast<save_resume_data_alert>(*i);
				if (!rd) continue;
				--num_outstanding_resume_data;
				printf("\rleft: %d failed: %d pause: %d "
					, num_outstanding_resume_data, num_failed, num_paused);

				if (!rd->resume_data) continue;

				torrent_handle h = rd->handle;
				torrent_status st = h.status(torrent_handle::query_save_path);
				std::vector<char> out;
				bencode(std::back_inserter(out), *rd->resume_data);
				save_file(combine_path(st.save_path, combine_path(".resume", to_hex(st.info_hash.to_string()) + ".resume")), out);
			}
		}

		printf("\nsaving session state\n");
		{
			entry session_state;
			s.save_state(session_state);

			std::vector<char> out;
			bencode(std::back_inserter(out), session_state);
			save_file(".ses_state", out);
		}

		printf("closing session");

		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
