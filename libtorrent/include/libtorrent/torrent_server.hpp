//
// torrent_server.h
// ~~~~~~~~~~~~~~~~
//
// Copyright (c) 2011 Jack (jack.wgm@gmail.com)
//
// This file is part of Libavplayer.
//
// Libavplayer is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// Libavplayer is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with Libavplayer; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
// * $Id: torrent_server.h 98 2011-08-19 16:08:52Z jack $
//

#ifndef __TORRENT_SERVER_H__
#define __TORRENT_SERVER_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

// 声明libtorrent中的实现.
namespace libtorrent {

	class session;
	struct torrent_handle;
	class extern_read_op;

	// 声明http server的实现.
	namespace http {
		namespace server {
			class server;
		}
	}
}


namespace libtorrent {

// 视频信息.
struct video_info
{
	std::string file_name;			// 文件名.
	std::string hash;				// hash.
	int index;						// 在libtorrent中的index.
	boost::int64_t base_offset;		// 在libtorrent中的偏移.
	boost::int64_t file_size;		// 文件大小.
	std::string url;				// 外部访问的url.
	// 访问API.
	boost::shared_ptr<extern_read_op> read_op;
};

// 具体的server实现.
// 使用方法, 示范如下
// 创建:
//  libtorrent::session s;
//  ...
//  torrent_server serv(s);
//  // 创建侦听8889端口.
//  serv.create_server(8889);
//  ...
// 销毁:
//  serv.destory_server();
//  s.stop();
// 注意: 创建和销毁libtorrent::session, torrent_server的顺序不能弄错了.
class torrent_server : boost::noncopyable
{
public:
	torrent_server(session &ses);
	~torrent_server(void);

	// 创建http的服务器端口.
	bool create_server(short port);

	// 销毁Http服务.
	void destory_server();

protected:
	bool request_handle(const std::string &uri, boost::int64_t &file_size, int &index);
	bool read_handle(int index, boost::int64_t offset, char *buffer, int &read_size);

private:
	libtorrent::session &m_ses;
	boost::mutex m_mutex;
	std::vector<video_info> m_video_infos;
	std::vector<torrent_handle> m_torrents;
	boost::shared_ptr<http::server::server> m_server;
	boost::thread m_server_thread;
};

}

#endif // __TORRENT_SERVER_H__
