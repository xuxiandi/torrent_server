//
// http_server.h
// ~~~~~~~~~~~~~
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
// * $Id: http_server.h 98 2011-08-19 16:08:52Z jack $
//

#ifndef __HTTP_SERVER_H__
#define __HTTP_SERVER_H__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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
#include <boost/date_time/posix_time/posix_time.hpp>


namespace libtorrent {
namespace http {
	namespace server {

		namespace status_strings {
			const std::string ok =
				"HTTP/1.1 200 OK\r\n";
			const std::string created =
				"HTTP/1.1 201 Created\r\n";
			const std::string accepted =
				"HTTP/1.1 202 Accepted\r\n";
			const std::string no_content =
				"HTTP/1.1 204 No Content\r\n";
			const std::string partial_content =
				"HTTP/1.1 206 Partial Content\r\n";
			const std::string multiple_choices =
				"HTTP/1.1 300 Multiple Choices\r\n";
			const std::string moved_permanently =
				"HTTP/1.1 301 Moved Permanently\r\n";
			const std::string moved_temporarily =
				"HTTP/1.1 302 Moved Temporarily\r\n";
			const std::string not_modified =
				"HTTP/1.1 304 Not Modified\r\n";
			const std::string bad_request =
				"HTTP/1.1 400 Bad Request\r\n";
			const std::string unauthorized =
				"HTTP/1.1 401 Unauthorized\r\n";
			const std::string forbidden =
				"HTTP/1.1 403 Forbidden\r\n";
			const std::string not_found =
				"HTTP/1.1 404 Not Found\r\n";
			const std::string internal_server_error =
				"HTTP/1.1 500 Internal Server Error\r\n";
			const std::string not_implemented =
				"HTTP/1.1 501 Not Implemented\r\n";
			const std::string bad_gateway =
				"HTTP/1.1 502 Bad Gateway\r\n";
			const std::string service_unavailable =
				"HTTP/1.1 503 Service Unavailable\r\n";
		} // namespace status_strings

		namespace misc_strings {

			const char name_value_separator[] = { ':', ' ' };
			const char crlf[] = { '\r', '\n' };

		} // namespace misc_strings

		namespace stock_replies {

			const char ok[] = "";
			const char created[] =
				"<html>"
				"<head><title>Created</title></head>"
				"<body><h1>201 Created</h1></body>"
				"</html>";
			const char accepted[] =
				"<html>"
				"<head><title>Accepted</title></head>"
				"<body><h1>202 Accepted</h1></body>"
				"</html>";
			const char no_content[] =
				"<html>"
				"<head><title>No Content</title></head>"
				"<body><h1>204 Content</h1></body>"
				"</html>";
			const char partial_content[] =
				"<html>"
				"<head><title>No Partial Content</title></head>"
				"<body><h1>206 Partial Content</h1></body>"
				"</html>";
			const char multiple_choices[] =
				"<html>"
				"<head><title>Multiple Choices</title></head>"
				"<body><h1>300 Multiple Choices</h1></body>"
				"</html>";
			const char moved_permanently[] =
				"<html>"
				"<head><title>Moved Permanently</title></head>"
				"<body><h1>301 Moved Permanently</h1></body>"
				"</html>";
			const char moved_temporarily[] =
				"<html>"
				"<head><title>Moved Temporarily</title></head>"
				"<body><h1>302 Moved Temporarily</h1></body>"
				"</html>";
			const char not_modified[] =
				"<html>"
				"<head><title>Not Modified</title></head>"
				"<body><h1>304 Not Modified</h1></body>"
				"</html>";
			const char bad_request[] =
				"<html>"
				"<head><title>Bad Request</title></head>"
				"<body><h1>400 Bad Request</h1></body>"
				"</html>";
			const char unauthorized[] =
				"<html>"
				"<head><title>Unauthorized</title></head>"
				"<body><h1>401 Unauthorized</h1></body>"
				"</html>";
			const char forbidden[] =
				"<html>"
				"<head><title>Forbidden</title></head>"
				"<body><h1>403 Forbidden</h1></body>"
				"</html>";
			const char not_found[] =
				"<html>"
				"<head><title>Not Found</title></head>"
				"<body><h1>404 Not Found</h1></body>"
				"</html>";
			const char internal_server_error[] =
				"<html>"
				"<head><title>Internal Server Error</title></head>"
				"<body><h1>500 Internal Server Error</h1></body>"
				"</html>";
			const char not_implemented[] =
				"<html>"
				"<head><title>Not Implemented</title></head>"
				"<body><h1>501 Not Implemented</h1></body>"
				"</html>";
			const char bad_gateway[] =
				"<html>"
				"<head><title>Bad Gateway</title></head>"
				"<body><h1>502 Bad Gateway</h1></body>"
				"</html>";
			const char service_unavailable[] =
				"<html>"
				"<head><title>Service Unavailable</title></head>"
				"<body><h1>503 Service Unavailable</h1></body>"
				"</html>";

		} // namespace stock_replies

		namespace mime_types {

			struct mapping
			{
				const char* extension;
				const char* mime_type;
			} mappings[] =
			{
				{ "htm",   "text/html" },
				{ "html",  "text/html" },
				{ "txt",   "text/plain" },
				{ "xml",   "text/xml" },
				{ "dtd",   "text/dtd" },
				{ "css",   "text/css" },

				/* image mime */
				{ "gif",   "image/gif" },
				{ "jpe",   "image/jpeg" },
				{ "jpg",   "image/jpeg" },
				{ "jpeg",  "image/jpeg" },
				{ "png",   "image/png" },
				/* same as modules/mux/mpjpeg.c here: */
				{ ".mpjpeg","multipart/x-mixed-replace; boundary=7b3cc56e5f51db803f790dad720ed50a" },

				/* media mime */
				{ "flv", "video/flv" },
				{ "rmvb", "video/x-pn-realvideo" },
				{ "mp4", "video/mp4" },
				{ "3gp", "video/3gpp" },
				{ "divx", "video/divx" },
				{ "avi",   "video/avi" },
				{ "asf",   "video/x-ms-asf" },
				{ "m1a",   "audio/mpeg" },
				{ "m2a",   "audio/mpeg" },
				{ "m1v",   "video/mpeg" },
				{ "m2v",   "video/mpeg" },
				{ "mp2",   "audio/mpeg" },
				{ "mp3",   "audio/mpeg" },
				{ "mpa",   "audio/mpeg" },
				{ "mpg",   "video/mpeg" },
				{ "mpeg",  "video/mpeg" },
				{ "mpe",   "video/mpeg" },
				{ "mov",   "video/quicktime" },
				{ "moov",  "video/quicktime" },
				{ "oga",   "audio/ogg" },
				{ "ogg",   "application/ogg" },
				{ "ogm",   "application/ogg" },
				{ "ogv",   "video/ogg" },
				{ "ogx",   "application/ogg" },
				{ "opus",  "audio/ogg; codecs=opus" },
				{ "spx",   "audio/ogg" },
				{ "wav",   "audio/wav" },
				{ "wma",   "audio/x-ms-wma" },
				{ "wmv",   "video/x-ms-wmv" },
				{ "webm",  "video/webm" },

				{ 0, 0 } // Marks end of list.
			};

			/// Convert a file extension into a MIME type.
			std::string extension_to_type(const std::string& extension)
			{
				for (mapping* m = mappings; m->extension; ++m)
				{
					if (m->extension == extension)
					{
						return m->mime_type;
					}
				}

				return "application/octet-stream";
			}

		} // namespace mime_types


		struct reply;
		struct request;

		//////////////////////////////////////////////////////////////////////////
		/// struct header.
		struct header
		{
			std::string name;
			std::string value;
		};

		//////////////////////////////////////////////////////////////////////////
		/// 用于客户端请求时的回调, 参数(uri[in], file_size[out], index[out])
		typedef boost::function<bool (const std::string&, boost::int64_t&, int&)> request_callback;

		//////////////////////////////////////////////////////////////////////////
		/// 用于向客户端回复数据的回调, 参数(index[in], offset[in], buffer[out], read_size[in/out]).
		typedef boost::function<bool (int, boost::int64_t, char*, int&)> read_callback;

		//////////////////////////////////////////////////////////////////////////
		/// A request received from a client.
		struct request
		{
			std::string method;
			std::string uri;
			boost::int64_t body_size;
			boost::int64_t offset;
			bool keep_alive;
			int video_index;
			int http_server_port;
			int http_version_major;
			int http_version_minor;
			std::vector<header> headers;
		};

		//////////////////////////////////////////////////////////////////////////
		/// A reply to be sent to a client.
		struct reply
		{
			/// The status of the reply.
			enum status_type
			{
				ok = 200,
				created = 201,
				accepted = 202,
				no_content = 204,
				partial_content = 206,
				multiple_choices = 300,
				moved_permanently = 301,
				moved_temporarily = 302,
				not_modified = 304,
				bad_request = 400,
				unauthorized = 401,
				forbidden = 403,
				not_found = 404,
				internal_server_error = 500,
				not_implemented = 501,
				bad_gateway = 502,
				service_unavailable = 503
			} status;

			/// The headers to be included in the reply.
			std::vector<header> headers;

			/// The content to be sent in the reply.
			std::string content;

			std::size_t send_bytes;

			static boost::asio::const_buffer status_to_buffer(status_type status)
			{
				switch (status)
				{
				case ok:
					return boost::asio::buffer(status_strings::ok);
				case created:
					return boost::asio::buffer(status_strings::created);
				case accepted:
					return boost::asio::buffer(status_strings::accepted);
				case no_content:
					return boost::asio::buffer(status_strings::no_content);
				case partial_content:
					return boost::asio::buffer(status_strings::partial_content);
				case multiple_choices:
					return boost::asio::buffer(status_strings::multiple_choices);
				case moved_permanently:
					return boost::asio::buffer(status_strings::moved_permanently);
				case moved_temporarily:
					return boost::asio::buffer(status_strings::moved_temporarily);
				case not_modified:
					return boost::asio::buffer(status_strings::not_modified);
				case bad_request:
					return boost::asio::buffer(status_strings::bad_request);
				case unauthorized:
					return boost::asio::buffer(status_strings::unauthorized);
				case forbidden:
					return boost::asio::buffer(status_strings::forbidden);
				case not_found:
					return boost::asio::buffer(status_strings::not_found);
				case internal_server_error:
					return boost::asio::buffer(status_strings::internal_server_error);
				case not_implemented:
					return boost::asio::buffer(status_strings::not_implemented);
				case bad_gateway:
					return boost::asio::buffer(status_strings::bad_gateway);
				case service_unavailable:
					return boost::asio::buffer(status_strings::service_unavailable);
				default:
					return boost::asio::buffer(status_strings::internal_server_error);
				}
			}

			static std::string status_to_string(reply::status_type status)
			{
				switch (status)
				{
				case reply::ok:
					return status_strings::ok;
				case reply::created:
					return status_strings::created;
				case reply::accepted:
					return status_strings::accepted;
				case reply::no_content:
					return status_strings::no_content;
				case reply::partial_content:
					return status_strings::partial_content;
				case reply::multiple_choices:
					return status_strings::multiple_choices;
				case reply::moved_permanently:
					return status_strings::moved_permanently;
				case reply::moved_temporarily:
					return status_strings::moved_temporarily;
				case reply::not_modified:
					return status_strings::not_modified;
				case reply::bad_request:
					return status_strings::bad_request;
				case reply::unauthorized:
					return status_strings::unauthorized;
				case reply::forbidden:
					return status_strings::forbidden;
				case reply::not_found:
					return status_strings::not_found;
				case reply::internal_server_error:
					return status_strings::internal_server_error;
				case reply::not_implemented:
					return status_strings::not_implemented;
				case reply::bad_gateway:
					return status_strings::bad_gateway;
				case reply::service_unavailable:
					return status_strings::service_unavailable;
				default:
					return status_strings::internal_server_error;
				}
			}

			/// Convert the reply into a vector of buffers. The buffers do not own the
			/// underlying memory blocks, therefore the reply object must remain valid and
			/// not be changed until the write operation has completed.
			std::string to_buffers()
			{
				std::string reply_string;
				reply_string = status_to_string(status);
				for (std::size_t i = 0; i < headers.size(); ++i)
				{
					header& h = headers[i];
					reply_string += h.name + 
						std::string(misc_strings::name_value_separator,2) + 
						h.value +
						std::string(misc_strings::crlf,2);
				}
				reply_string += std::string(misc_strings::crlf,2);

				return reply_string;
			}

			/// Get a stock reply.
			static reply stock_reply(status_type status)
			{
				reply rep;
				rep.status = status;
				rep.content = status_to_string(status);
				rep.headers.resize(2);
				rep.headers[0].name = "Content-Length";
				rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
				rep.headers[1].name = "Content-Type";
				rep.headers[1].value = "text/html";
				return rep;
			}
		};

		//////////////////////////////////////////////////////////////////////////
		/// Parser for incoming requests.
		class request_parser
		{
		public:
			/// Construct ready to parse the request method.
			request_parser()
				: state_(method_start)
			{

			}

			/// Reset to initial parser state.
			void reset()
			{
				state_ = method_start;
			}

			/// Parse some data. The tribool return value is true when a complete request
			/// has been parsed, false if the data is invalid, indeterminate when more
			/// data is required. The InputIterator return value indicates how much of the
			/// input has been consumed.
			template <typename InputIterator>
			boost::tuple<boost::tribool, InputIterator> parse(request& req,
				InputIterator begin, InputIterator end)
			{
				while (begin != end)
				{
					boost::tribool result = consume(req, *begin++);
					if (result || !result)
						return boost::make_tuple(result, begin);
				}
				boost::tribool result = boost::indeterminate;
				return boost::make_tuple(result, begin);
			}

		private:
			/// Handle the next character of input.
			boost::tribool consume(request& req, char input)
			{
				switch (state_)
				{
				case method_start:
					if (!is_char(input) || is_ctl(input) || is_tspecial(input))
					{
						return false;
					}
					else
					{
						state_ = method;
						req.method.push_back(input);
						return boost::indeterminate;
					}
				case method:
					if (input == ' ')
					{
						state_ = uri;
						return boost::indeterminate;
					}
					else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
					{
						return false;
					}
					else
					{
						req.method.push_back(input);
						return boost::indeterminate;
					}
				case uri_start:
					if (is_ctl(input))
					{
						return false;
					}
					else
					{
						state_ = uri;
						req.uri.push_back(input);
						return boost::indeterminate;
					}
				case uri:
					if (input == ' ')
					{
						state_ = http_version_h;
						return boost::indeterminate;
					}
					else if (is_ctl(input))
					{
						return false;
					}
					else
					{
						req.uri.push_back(input);
						return boost::indeterminate;
					}
				case http_version_h:
					if (input == 'H')
					{
						state_ = http_version_t_1;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_t_1:
					if (input == 'T')
					{
						state_ = http_version_t_2;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_t_2:
					if (input == 'T')
					{
						state_ = http_version_p;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_p:
					if (input == 'P')
					{
						state_ = http_version_slash;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_slash:
					if (input == '/')
					{
						req.http_version_major = 0;
						req.http_version_minor = 0;
						state_ = http_version_major_start;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_major_start:
					if (is_digit(input))
					{
						req.http_version_major = req.http_version_major * 10 + input - '0';
						state_ = http_version_major;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_major:
					if (input == '.')
					{
						state_ = http_version_minor_start;
						return boost::indeterminate;
					}
					else if (is_digit(input))
					{
						req.http_version_major = req.http_version_major * 10 + input - '0';
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_minor_start:
					if (is_digit(input))
					{
						req.http_version_minor = req.http_version_minor * 10 + input - '0';
						state_ = http_version_minor;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case http_version_minor:
					if (input == '\r')
					{
						state_ = expecting_newline_1;
						return boost::indeterminate;
					}
					else if (is_digit(input))
					{
						req.http_version_minor = req.http_version_minor * 10 + input - '0';
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case expecting_newline_1:
					if (input == '\n')
					{
						state_ = header_line_start;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case header_line_start:
					if (input == '\r')
					{
						state_ = expecting_newline_3;
						return boost::indeterminate;
					}
					else if (!req.headers.empty() && (input == ' ' || input == '\t'))
					{
						state_ = header_lws;
						return boost::indeterminate;
					}
					else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
					{
						return false;
					}
					else
					{
						req.headers.push_back(header());
						req.headers.back().name.push_back(input);
						state_ = header_name;
						return boost::indeterminate;
					}
				case header_lws:
					if (input == '\r')
					{
						state_ = expecting_newline_2;
						return boost::indeterminate;
					}
					else if (input == ' ' || input == '\t')
					{
						return boost::indeterminate;
					}
					else if (is_ctl(input))
					{
						return false;
					}
					else
					{
						state_ = header_value;
						req.headers.back().value.push_back(input);
						return boost::indeterminate;
					}
				case header_name:
					if (input == ':')
					{
						state_ = space_before_header_value;
						return boost::indeterminate;
					}
					else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
					{
						return false;
					}
					else
					{
						req.headers.back().name.push_back(input);
						return boost::indeterminate;
					}
				case space_before_header_value:
					if (input == ' ')
					{
						state_ = header_value;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case header_value:
					if (input == '\r')
					{
						state_ = expecting_newline_2;
						return boost::indeterminate;
					}
					else if (is_ctl(input))
					{
						return false;
					}
					else
					{
						req.headers.back().value.push_back(input);
						return boost::indeterminate;
					}
				case expecting_newline_2:
					if (input == '\n')
					{
						state_ = header_line_start;
						return boost::indeterminate;
					}
					else
					{
						return false;
					}
				case expecting_newline_3:
					return (input == '\n');
				default:
					return false;
				}
			}

			/// Check if a byte is an HTTP character.
			static bool is_char(int c)
			{
				return c >= 0 && c <= 127;
			}

			/// Check if a byte is an HTTP control character.
			static bool is_ctl(int c)
			{
				return (c >= 0 && c <= 31) || (c == 127);
			}

			/// Check if a byte is defined as an HTTP tspecial character.
			static bool is_tspecial(int c)
			{
				switch (c)
				{
				case '(': case ')': case '<': case '>': case '@':
				case ',': case ';': case ':': case '\\': case '"':
				case '/': case '[': case ']': case '?': case '=':
				case '{': case '}': case ' ': case '\t':
					return true;
				default:
					return false;
				}
			}

			/// Check if a byte is a digit.
			static bool is_digit(int c)
			{
				return c >= '0' && c <= '9';
			}

			/// The current state of the parser.
			enum state
			{
				method_start,
				method,
				uri_start,
				uri,
				http_version_h,
				http_version_t_1,
				http_version_t_2,
				http_version_p,
				http_version_slash,
				http_version_major_start,
				http_version_major,
				http_version_minor_start,
				http_version_minor,
				expecting_newline_1,
				header_line_start,
				header_lws,
				header_name,
				space_before_header_value,
				header_value,
				expecting_newline_2,
				expecting_newline_3
			} state_;
		};

		//////////////////////////////////////////////////////////////////////////
		/// The common handler for all incoming requests.
		class request_handler
			: private boost::noncopyable
		{
		public:
			/// Construct with a directory containing files to be served.
			explicit request_handler(const std::string &doc_root, const request_callback &callback)
				: doc_root_(doc_root)
				, m_request_callback(callback)
			{
			}

			/// Handle a request and produce a reply.
			void handle_request(request& req, reply& rep)
			{
				// Decode url to path.
				std::string request_path;
				if (!url_decode(req.uri, request_path))
				{
					rep = reply::stock_reply(reply::bad_request);
					return;
				}

				// Request path must be absolute and not contain "..".
				if (request_path.empty() || request_path[0] != '/'
					|| request_path.find("..") != std::string::npos)
				{
					rep = reply::stock_reply(reply::bad_request);
					return;
				}

				// If path ends in slash (i.e. is a directory) then add "0".
				if (request_path[request_path.size() - 1] == '/')
				{
					request_path += "0";
				}

				// Determine the file extension.
				std::size_t last_slash_pos = request_path.find_last_of("/");
				std::size_t last_dot_pos = request_path.find_last_of(".");
				std::string extension;
				if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
				{
					extension = request_path.substr(last_dot_pos + 1);
				}

				boost::int64_t range_start = 0, range_end = 0;
				bool is_keep_alive = false;

				// headers request process.
				std::string header_name, header_value;
				bool is_range_request = false;

				for (std::vector<header>::const_iterator i = req.headers.begin();
					i != req.headers.end(); i++)
				{
					header_name = i->name;
					boost::to_lower(header_name);
					if (header_name == "range")
					{
						header_value = i->value;
						is_range_request = true;
						boost::to_lower(header_value);
						bool success = true;
						char const* ptr = header_value.c_str();
						if (string_begins_no_case("bytes=", ptr))
							ptr += 6;
						char* end;
						range_start = strtoll(ptr, &end, 10);
						if (end == ptr)
							success = false;
						else if (*end != '-') 
							success = false;
						else
						{
							ptr = end + 1;
							if (*ptr == 0) {
								range_end = -1;
								continue;
							}
							range_end = strtoll(ptr, &end, 10);
							if (end == ptr) success = false;
						}

						if (!success || range_end < range_start) {
							range_start = range_end = 0;
							continue;
						}
					}

					if (header_name == "connection")
					{
						header_value = i->value;
						boost::to_lower(header_value);
						if (header_value == "keep-alive")
							is_keep_alive = true;
						else if (header_value == "close")
							is_keep_alive = false;
						else
							is_keep_alive = false;
					}
				}

				last_slash_pos = req.uri.find_last_of("/");
				if (last_slash_pos != std::string::npos)
				{
					std::string tmp = req.uri.substr(last_slash_pos + 1);
					req.video_index = atoi(tmp.c_str());
				}
				else
				{
					req.video_index = 0;
				}

				if (!is_range_request)
					rep.status = reply::ok;
				else
					rep.status = reply::partial_content;

				boost::int64_t file_size = 0;
				boost::int64_t body_size = 0;

				/// 回调函数, 处理用户请求.
				if (m_request_callback)
				{
					if (!m_request_callback(boost::cref(req.uri),
						boost::ref(file_size), boost::ref(req.video_index)))
					{
						rep = reply::stock_reply(reply::bad_request);
						return;
					}
				}

				if (file_size != 0)
				{
					if ((range_start == 0 && range_end == 0) ||
						(range_start == 0 && range_end == -1))
					{
						body_size = file_size;
					}
					else
					{
						if (range_end == -1)
							body_size = file_size - range_start;
						else
							body_size = range_end - range_start + 1;
					}
				}

				req.body_size = range_start + body_size;
				req.keep_alive = is_keep_alive;
				req.offset = range_start;
				range_end = req.body_size - 1;

#if defined(_DEBUG) && defined(WIN32)
				std::cout << "request: offset: " << range_start << ", end offset: "
					<< range_end <<  ", body_size: " << req.body_size << ", file size: "
					<< file_size << std::endl;
#endif

				rep.headers.resize(6);
				rep.headers[0].name = "Content-Length";
				rep.headers[0].value = boost::lexical_cast<std::string>(body_size);
				rep.headers[1].name = "Server";
				rep.headers[1].value = "TorrentServer/1.0";
				rep.headers[2].name = "Content-Range";
				rep.headers[2].value = "bytes " + boost::lexical_cast<std::string>(range_start)
					+ "-" + boost::lexical_cast<std::string>(range_end) 
					+ "/" + boost::lexical_cast<std::string>(file_size);
				rep.headers[3].name = "Content-Type";
				rep.headers[3].value = mime_types::extension_to_type(extension);
				rep.headers[4].name = "Connection";
				if (req.keep_alive)
					rep.headers[4].value = "keep-alive";
				else
					rep.headers[4].value = "close";
				rep.headers[5].name = "Accept-Ranges";
				rep.headers[5].value = "bytes";
			}

		private:
			/// The directory containing the files to be served.
			std::string doc_root_;

			/// 用于客户端请求时的回调.
			request_callback m_request_callback;

			/// Perform URL-decoding on a string. Returns false if the encoding was
			/// invalid.
			static bool url_decode(const std::string& in, std::string& out)
			{
				out.clear();
				out.reserve(in.size());
				for (std::size_t i = 0; i < in.size(); ++i)
				{
					if (in[i] == '%')
					{
						if (i + 3 <= in.size())
						{
							int value = 0;
							std::istringstream is(in.substr(i + 1, 2));
							if (is >> std::hex >> value)
							{
								out += static_cast<char>(value);
								i += 2;
							}
							else
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					}
					else if (in[i] == '+')
					{
						out += ' ';
					}
					else
					{
						out += in[i];
					}
				}
				return true;
			}
		};


		class connection;
		class connection_manager;

		//////////////////////////////////////////////////////////////////////////
		/// Represents a single connection from a client.
		const static int io_buffer_size = 512 * 1024;
		class connection
			: public boost::enable_shared_from_this<connection>,
			private boost::noncopyable
		{
		public:
			/// Construct a connection with the given io_service.
			explicit connection(boost::asio::io_service &io_service,
				connection_manager *manager, request_handler &handler, read_callback &read_cb)
				: m_socket(io_service)
				, m_timer(io_service)
				, m_connection_manager(manager)
				, m_request_handler(handler)
				, m_read_callback(read_cb)
				, m_keep_alive(false)
				, m_abort(false)
			{

			}

			/// Get the socket associated with the connection.
			boost::asio::ip::tcp::socket& socket()
			{
				return m_socket;
			}

			/// Start the first asynchronous operation for the connection.
			void start()
			{
				boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n", 
					boost::bind(&connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
			}

			/// Stop all asynchronous operations associated with the connection.
			void stop()
			{
				m_abort = true;

				boost::system::error_code ignore;
				m_timer.cancel(ignore);
				m_socket.close(ignore);
			}

		private:
			/// Read from libtorrent.
			inline int read_from_torrent(request &req, reply &rep);

			/// Handle completion of a read operation.
			void handle_read(const boost::system::error_code &e, std::size_t bytes_transferred);

			/// Handle completion of a write operation.
			void handle_write(boost::shared_ptr<request> request_, 
				boost::shared_ptr<reply> reply_, std::size_t bytes_transferred, 
				const boost::system::error_code& e);

			void handle_timer(const boost::system::error_code &e);

			/// Socket for the connection.
			boost::asio::ip::tcp::socket m_socket;

			/// deadline timer.
			boost::asio::deadline_timer m_timer;

			/// The manager for this connection.
			connection_manager *m_connection_manager;

			/// The handler used to process the incoming request.
			request_handler &m_request_handler;

			/// 从上层读取数据的回调.
			read_callback m_read_callback;

			/// Buffer for torrent incoming data.
			boost::array<char, io_buffer_size> m_buffer;

			/// Buffer for incoming data.
			boost::asio::streambuf m_response;

			/// The incoming request.
			boost::shared_ptr<request> m_request;

			/// The reply to be sent back to the client.
			boost::shared_ptr<reply> m_reply;

			/// Is keep alive.
			bool m_keep_alive;

			/// The parser for the incoming request.
			request_parser m_request_parser;

			/// abort.
			bool m_abort;
		};

		typedef boost::shared_ptr<connection> connection_ptr;

		//////////////////////////////////////////////////////////////////////////
		/// Manages open connections so that they may be cleanly stopped when the server
		/// needs to shut down.
		class connection_manager
			: private boost::noncopyable
		{
		public:
			/// Add the specified connection to the manager and start it.
			void start(connection_ptr c)
			{
				connections_.insert(c);
				c->start();
			}

			/// Stop the specified connection.
			void stop(connection_ptr c)
			{
				connections_.erase(c);
				c->stop();
			}

			/// Stop all connections.
			void stop_all()
			{
				std::for_each(connections_.begin(), connections_.end(),
					boost::bind(&connection::stop, _1));
				connections_.clear();
			}

		private:
			/// The managed connections.
			std::set<connection_ptr> connections_;
		};

		int connection::read_from_torrent(request &req, reply &rep)
		{
			// 使用最小的size, 避免缓冲泄露.
			int read_bytes = 
				std::min<boost::uint64_t>(m_buffer.size(), req.body_size);

			if (m_read_callback)
			{
				do
				{
					int bytes_transferred = read_bytes;
					// 读取数据.
					if (m_read_callback(req.video_index, req.offset, m_buffer.data(), boost::ref(bytes_transferred)))
					{
						if (bytes_transferred != 0)
						{
							req.offset += bytes_transferred;
							return bytes_transferred;
						}
						else
						{
							if (req.offset >= req.body_size)
								return 0;
							boost::this_thread::sleep(boost::posix_time::millisec(100));
							return 0;
						}
					}
					else
					{
						return 0;
					}
				} while (true);
			}

			return 0;
		}

		void connection::handle_read(const boost::system::error_code& e,
			std::size_t bytes_transferred)
		{
			if (!e)
			{
				m_request.reset(new request);
				m_reply.reset(new reply);

				std::vector<char> buffer;
				buffer.resize(bytes_transferred + 1);
				buffer[bytes_transferred] = 0;
				m_response.sgetn(&buffer[0], m_response.size());

				boost::tribool result;
				boost::tie(result, boost::tuples::ignore) = m_request_parser.parse(
					*m_request, buffer.begin(), buffer.end());

				if (result)
				{
					boost::system::error_code ignore_ec;
					// Get current local port.
					m_request->http_server_port = m_socket.local_endpoint(ignore_ec).port();
					// handle request.
					m_request_handler.handle_request(*m_request, *m_reply);
					if (m_request->keep_alive && !m_keep_alive)
						m_keep_alive = true;
					// copy rep to buffer.
					std::string rep = m_reply->to_buffers();
					std::copy(rep.begin(), rep.end(), m_buffer.begin());
					m_reply->send_bytes = rep.length();
					if (m_reply->send_bytes == 0)
					{
						m_timer.expires_from_now(boost::posix_time::seconds(1));
						m_timer.async_wait(boost::bind(&connection::handle_timer, shared_from_this(), boost::asio::placeholders::error));
						return ;
					}
					// send buffer to client.
					boost::asio::async_write(m_socket, boost::asio::buffer(m_buffer, rep.length()),
						boost::bind(&connection::handle_write, shared_from_this(), m_request, m_reply,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));
					// async read http header.
					if (m_reply->status != reply::bad_request)
					{
						boost::asio::async_read_until(m_socket, m_response, "\r\n\r\n", 
							boost::bind(&connection::handle_read, shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred));
					}

					return ;
				}
				else
				{
					std::cout << "receiver a bad request!\n" << std::endl;
					*m_reply = reply::stock_reply(reply::bad_request);
					boost::asio::async_write(m_socket, boost::asio::buffer(m_buffer),
						boost::bind(&connection::handle_write, shared_from_this(), m_request, m_reply,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));
				}
			}
			else
			{
				// stop this connection.
				m_connection_manager->stop(shared_from_this());
			}
		}

		void connection::handle_write(boost::shared_ptr<request> request_,
			boost::shared_ptr<reply> reply_, std::size_t bytes_transferred, 
			const boost::system::error_code& e)
		{
			if (!e)
			{
				if ((request_ && reply_) && 
					(request_->body_size > 0 && reply_->status != reply::bad_request) &&
					(request_->offset < request_->body_size))
				{
					if (reply_->send_bytes - bytes_transferred != 0)
					{
						reply_->send_bytes -= bytes_transferred;
						boost::asio::async_write(m_socket, 
							boost::asio::buffer(m_buffer.data() + bytes_transferred, reply_->send_bytes),
							boost::bind(&connection::handle_write, shared_from_this(), request_, reply_,
							boost::asio::placeholders::bytes_transferred,
							boost::asio::placeholders::error));
						return ;
					}
					// read data to send.
					reply_->content.clear();
					int read_bytes = read_from_torrent(*request_, *reply_);
					reply_->send_bytes = read_bytes;
					if (m_reply->send_bytes == 0)
					{
						m_timer.expires_from_now(boost::posix_time::seconds(1));
						m_timer.async_wait(boost::bind(&connection::handle_timer, shared_from_this(), boost::asio::placeholders::error));
						return ;
					}
					boost::asio::async_write(m_socket, boost::asio::buffer(m_buffer, read_bytes),
						boost::bind(&connection::handle_write, shared_from_this(), request_, reply_,
						boost::asio::placeholders::bytes_transferred,
						boost::asio::placeholders::error));
					return ;
				}
				else
				{
					// for keep alive and not bad request.
					if (request_ && request_->keep_alive)
					{
						if (m_reply->status == reply::bad_request)
							m_connection_manager->stop(shared_from_this());
						return ;
					}
					if (!request_)
					{
						m_connection_manager->stop(shared_from_this());
						return ;
					}
					// Initiate graceful connection closure.
					boost::system::error_code ignored_ec;
					m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
					return ;
				}
			}

			if (e != boost::asio::error::operation_aborted)
			{
				m_connection_manager->stop(shared_from_this());
			}
		}

		void connection::handle_timer(const boost::system::error_code &e)
		{
			if (m_abort || e)
				return;

			// read data to send.
			m_reply->content.clear();
			int read_bytes = read_from_torrent(*m_request, *m_reply);
			if (read_bytes == 0)
			{
				// one second retry.
				m_timer.expires_from_now(boost::posix_time::seconds(1));
				m_timer.async_wait(boost::bind(&connection::handle_timer, shared_from_this(), boost::asio::placeholders::error));
			}
			else
			{
				m_reply->send_bytes = read_bytes;
				boost::asio::async_write(m_socket, boost::asio::buffer(m_buffer, read_bytes),
					boost::bind(&connection::handle_write, shared_from_this(), m_request, m_reply,
					boost::asio::placeholders::bytes_transferred,
					boost::asio::placeholders::error));
			}
		}
		//////////////////////////////////////////////////////////////////////////
		/// The top-level class of the HTTP server.
		class server
			: private boost::noncopyable
		{
		public:
			/// Construct the server to listen on the specified TCP address and port, and
			/// serve up files from the given directory.
			explicit server(const std::string &address, const std::string &port,
				const std::string &doc_root,
				const request_callback request_cb,
				read_callback read_cb)
				: m_io_service()
				, m_signals(m_io_service)
				, m_acceptor(m_io_service)
				, m_connection_manager()
				, m_read_callback(read_cb)
				, m_new_connection()
				, m_request_handler(doc_root, request_cb)
			{
				// Register to handle the signals that indicate when the server should exit.
				// It is safe to register for the same signal multiple times in a program,
				// provided all registration for the specified signal is made through Asio.
				m_signals.add(SIGINT);
				m_signals.add(SIGTERM);
#if defined(SIGQUIT)
				m_signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
				m_signals.async_wait(boost::bind(&server::handle_stop, this));

				// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
				boost::asio::ip::tcp::resolver resolver(m_io_service);
				boost::asio::ip::tcp::resolver::query query(address, port);
				boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
				m_acceptor.open(endpoint.protocol());
				m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
				m_acceptor.bind(endpoint);
				m_acceptor.listen();

				start_accept();
			}

			/// Run the server's io_service loop.
			void run()
			{
				// The io_service::run() call will block until all asynchronous operations
				// have finished. While the server is running, there is always at least one
				// asynchronous operation outstanding: the asynchronous accept call waiting
				// for new incoming connections.
				m_io_service.run();
			}

			/// Stop the server's io_service loop.
			void stop()
			{
				m_io_service.stop();
			}

		private:
			/// Initiate an asynchronous accept operation.
			void start_accept()
			{
				m_new_connection.reset(new connection(m_io_service,
					&m_connection_manager, m_request_handler, m_read_callback));
				m_acceptor.async_accept(m_new_connection->socket(),
					boost::bind(&server::handle_accept, this,
					boost::asio::placeholders::error));
			}

			/// Handle completion of an asynchronous accept operation.
			void handle_accept(const boost::system::error_code& e)
			{
				// Check whether the server was stopped by a signal before this completion
				// handler had a chance to run.
				if (!m_acceptor.is_open())
				{
					return;
				}

				if (!e)
				{
					m_connection_manager.start(m_new_connection);
				}

				start_accept();
			}

			/// Handle a request to stop the server.
			void handle_stop()
			{
				// The server is stopped by cancelling all outstanding asynchronous
				// operations. Once all operations have finished the io_service::run() call
				// will exit.
				m_acceptor.close();
				m_connection_manager.stop_all();
			}

			/// The io_service used to perform asynchronous operations.
			boost::asio::io_service m_io_service;

			/// The signal_set is used to register for process termination notifications.
			boost::asio::signal_set m_signals;

			/// Acceptor used to listen for incoming connections.
			boost::asio::ip::tcp::acceptor m_acceptor;

			/// The connection manager which owns all live connections.
			connection_manager m_connection_manager;

			/// 用connection读取数据回调.
			read_callback m_read_callback;

			/// The next connection to be accepted.
			connection_ptr m_new_connection;

			/// The handler for all incoming requests.
			request_handler m_request_handler;
		};

	} // namespace server
} // namespace http
} // namespace libtorrent

#endif // __HTTP_SERVER_H__
