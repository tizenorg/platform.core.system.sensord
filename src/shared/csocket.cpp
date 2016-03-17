/*
 * sensord
 *
 * Copyright (c) 2013 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <sensor_log.h>
#include <csocket.h>
#include <attr/xattr.h>
#include <sys/stat.h>
#include <stdint.h>

csocket::csocket()
: m_sock_fd(-1)
, m_sock_type(SOCK_STREAM)
, m_send_flags(MSG_NOSIGNAL)
, m_recv_flags(MSG_NOSIGNAL)
{
	memset(&m_addr, 0, sizeof(m_addr));
}


csocket::csocket(int sock_fd)
: m_sock_fd(-1)
, m_sock_type(SOCK_STREAM)
, m_send_flags(MSG_NOSIGNAL)
, m_recv_flags(MSG_NOSIGNAL)
{
	m_sock_fd = sock_fd;
	set_sock_type();
	memset(&m_addr, 0, sizeof(m_addr));
}


csocket::csocket(const csocket &sock)
: m_sock_fd(-1)
, m_sock_type(SOCK_STREAM)
, m_send_flags(MSG_NOSIGNAL)
, m_recv_flags(MSG_NOSIGNAL)
{
	if (this == &sock)
		return;

	m_sock_fd = sock.m_sock_fd;
	m_sock_type = sock.m_sock_type;
	m_send_flags = sock.m_send_flags;
	m_recv_flags = sock.m_recv_flags;

	memcpy(&m_addr, &sock.m_addr, sizeof(sockaddr_un));
}

csocket::~csocket() { }

bool csocket::create(int sock_type)
{
	m_sock_fd = socket(AF_UNIX, sock_type, 0);

	if (!is_valid()) {
		_E("Failed to create socket for %s", get_client_name());
		_ERRNO(errno);
		return false;
	}

	m_sock_type = sock_type;

	return true;
}

bool csocket::bind (const char *sock_path)
{
	int length;
	mode_t socket_mode;

	if (!is_valid()) {
		_E("%s's socket is invalid", get_client_name());
		return false;
	}

	if (!access(sock_path, F_OK)) {
		unlink(sock_path);
	}

	m_addr.sun_family = AF_UNIX;

	strncpy(m_addr.sun_path, sock_path, strlen(sock_path));

	length = strlen(m_addr.sun_path) + sizeof(m_addr.sun_family);

	if (::bind(m_sock_fd, (struct sockaddr *)&m_addr, length) < 0) {
		_E("Binding failed for socket(%d)", m_sock_fd);
		_ERRNO(errno);
		close();
		return false;
	}

	socket_mode = ( S_IRWXU | S_IRWXG | S_IRWXO );
	if (chmod(sock_path, socket_mode) < 0) {
		_E("chmod failed for socket(%d)", m_sock_fd);
		_ERRNO(errno);
		close();
		return false;
	}

	return true;
}

bool csocket::listen(const int max_connections)
{
	if (!is_valid()) {
		_E("Socket(%d) is invalid", m_sock_fd);
		return false;
	}

	if (::listen(m_sock_fd, max_connections) < 0) {
		_E("Listening failed for socket(%d)", m_sock_fd);
		_ERRNO(errno);
		close();
		return false;
	}

	return true;
}

bool csocket::accept(csocket& client_socket) const
{
	int addr_length = sizeof(m_addr);
	int err = 0;

	do {
		client_socket.m_sock_fd = ::accept(m_sock_fd, (sockaddr *)&m_addr, (socklen_t *)&addr_length);
		if (!client_socket.is_valid()) {
			err = errno;
			::close(client_socket.m_sock_fd);
		}
	} while (err == EINTR);

	if (!client_socket.is_valid()) {
		_E("Accept failed for socket(%d)", m_sock_fd);
		_ERRNO(errno);
		return false;
	}

	return true;
}

ssize_t csocket::send_for_seqpacket(const void *buffer, size_t size) const
{
	ssize_t err, len;

	do {
		len = ::send(m_sock_fd, buffer, size, m_send_flags);
		err = len < 0 ? errno : 0;
	} while (err == EINTR);

	if (err) {
		_E("send(%d, 0x%x, %d, 0x%x) = %d",
			m_sock_fd, buffer, size, m_send_flags, len);
		_ERRNO(errno);
	}

	return err == 0 ? len : -err;
}

ssize_t csocket::recv_for_seqpacket(void* buffer, size_t size) const
{
    ssize_t err, len;

	do {
        len = ::recv(m_sock_fd, buffer, size, m_recv_flags);

		if (len > 0) {
			err = 0;
		} else if (len == 0) {
			_E("recv(%d, 0x%p , %d) = %d, because the peer performed shutdown!",
				m_sock_fd, buffer, size, len);
			err = 1;
		} else {
			err = errno;
		}
    } while (err == EINTR);

	if ((err == EAGAIN) || (err == EWOULDBLOCK)) {
		_D("recv(%d, 0x%x, %d, 0x%x) = %d",
			m_sock_fd, buffer, size, m_recv_flags, len);
		_ERRNO(errno);
		return 0;
	}

	if (err) {
		_E("recv(%d, 0x%x, %d, 0x%x) = %d",
			m_sock_fd, buffer, size, m_recv_flags, len);
		_ERRNO(errno);
	}

    return err == 0 ? len : -err;
}


ssize_t csocket::send_for_stream(const void *buffer, size_t size) const
{
	ssize_t len;
	ssize_t err = 0;
	size_t total_sent_size = 0;

	do {
		len = ::send(m_sock_fd, (const void *)((uint8_t *)buffer + total_sent_size), size - total_sent_size, m_send_flags);

		if (len >= 0) {
			total_sent_size += len;
			err = 0;
		} else {
			_E("send(%d, 0x%p + %d, %d - %d) = %d for %s",
				m_sock_fd, buffer, total_sent_size, size, total_sent_size,
				len, get_client_name());
			_ERRNO(errno);

			if (errno != EINTR) {
				err = errno;
				break;
			}
		}
	} while (total_sent_size < size);

	return err == 0 ? total_sent_size : -err;
}

ssize_t csocket::recv_for_stream(void* buffer, size_t size) const
{
	ssize_t len;
	ssize_t err = 0;
	size_t total_recv_size = 0;

	do {
		len = ::recv(m_sock_fd, (void *)((uint8_t *)buffer + total_recv_size), size - total_recv_size, m_recv_flags);

		if (len > 0) {
			total_recv_size += len;
		} else if (len == 0) {
			_E("recv(%d, 0x%p + %d, %d - %d) = %d, because the peer of %s performed shutdown!",
				m_sock_fd, buffer, total_recv_size, size, total_recv_size, len, get_client_name());
			err = 1;
			break;
		} else {
			_E("recv(%d, 0x%p + %d, %d - %d) = %d for %s",
				m_sock_fd, buffer, total_recv_size, size, total_recv_size,
				len, get_client_name());
			_ERRNO(errno);

			if (errno != EINTR) {
				err = errno;
				break;
			}
		}
	} while (total_recv_size < size);

	return err == 0 ? total_recv_size : -err;
}


ssize_t csocket::send(const void *buffer, size_t size) const
{
	if (m_sock_type == SOCK_STREAM)
		return send_for_stream(buffer, size);

	return send_for_seqpacket(buffer, size);
}

ssize_t csocket::recv(void* buffer, size_t size) const
{
	if (m_sock_type == SOCK_STREAM)
		return recv_for_stream(buffer, size);

	return recv_for_seqpacket(buffer, size);
}

bool csocket::connect(const char *sock_path)
{
	const int TIMEOUT = 5;
	fd_set write_fds;
	struct timeval tv;
	int addr_len;
	bool prev_blocking_mode;

	if (!is_valid()) {
		_E("%s's socket is invalid", get_client_name());
		return false;
	}

	prev_blocking_mode = is_blocking_mode();

	set_blocking_mode(false);

	m_addr.sun_family = AF_UNIX;

	strncpy(m_addr.sun_path, sock_path, strlen(sock_path));

	addr_len = strlen(m_addr.sun_path) + sizeof(m_addr.sun_family);

	if (::connect(m_sock_fd,(sockaddr *) &m_addr, addr_len) < 0) {
		_E("connect error: sock_fd: %d\n for %s", m_sock_fd, get_client_name());
		_ERRNO(errno);
		return false;
	}

	FD_ZERO(&write_fds);
	FD_SET(m_sock_fd, &write_fds);
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	int ret;

	ret = select(m_sock_fd + 1, NULL, &write_fds, NULL, &tv);

	if (ret == -1) {
		_E("select error: sock_fd: %d\n for %s", m_sock_fd, get_client_name());
		_ERRNO(errno);
		close();
		return false;
	} else if (!ret) {
		_E("select timeout: %d seconds elapsed for %s", tv.tv_sec, get_client_name());
		close();
		return false;
	}

	if (!FD_ISSET(m_sock_fd, &write_fds)) {
		_E("select failed for %s, nothing to write, m_sock_fd : %d", get_client_name(), m_sock_fd);
		close();
		return false;
	}

	int so_error;
	socklen_t len = sizeof(so_error);

	if (getsockopt(m_sock_fd, SOL_SOCKET, SO_ERROR, &so_error, &len) == -1) {
		_E("getsockopt failed for %s, m_sock_fd : %d", get_client_name(), m_sock_fd);
		_ERRNO(errno);
		close();
		return false;
	}

	if (so_error) {
		_E("SO_ERROR occurred for %s, m_sock_fd : %d, so_error : %d",
			get_client_name(), m_sock_fd, so_error);
		close();
		return false;
	}

	if (prev_blocking_mode)
		set_blocking_mode(true);

	return true;
}

bool csocket::set_blocking_mode(bool blocking)
{
	int flags;

	flags = fcntl(m_sock_fd, F_GETFL);

	if (flags == -1) {
		_E("fcntl(F_GETFL) failed for %s, m_sock_fd: %d", get_client_name(), m_sock_fd);
		_ERRNO(errno);
		return false;
	}

	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);

	flags = fcntl(m_sock_fd, F_SETFL, flags);

	if (flags == -1) {
		_E("fcntl(F_SETFL) failed for %s, m_sock_fd: %d", get_client_name(), m_sock_fd);
		_ERRNO(errno);
		return false;
	}

	return true;
}


bool csocket::set_sock_type(void)
{
	socklen_t opt_len;
	int sock_type;

	opt_len = sizeof(sock_type);

	if (getsockopt(m_sock_fd, SOL_SOCKET, SO_TYPE, &sock_type, &opt_len) < 0) {
	   _E("getsockopt(SOL_SOCKET, SO_TYPE) failed for %s, m_sock_fd: %d",
		  get_client_name(), m_sock_fd);
	   _ERRNO(errno);
	   return false;
	}

	m_sock_type = sock_type;
	return true;
}

bool csocket::set_connection_mode(void)
{
	struct timeval tv;
	const int TIMEOUT = 5;

	set_blocking_mode(true);

	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;

	if(setsockopt(m_sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
		_E("Set SO_RCVTIMEO failed for %s, m_sock_fd : %d", get_client_name(), m_sock_fd);
		_ERRNO(errno);
		close();
		return false;
	}

	m_send_flags = MSG_NOSIGNAL;
	m_recv_flags = MSG_NOSIGNAL;

	return true;
}

bool csocket::set_transfer_mode(void)
{
	set_blocking_mode(false);


	m_send_flags = MSG_DONTWAIT | MSG_NOSIGNAL;
	m_recv_flags = MSG_DONTWAIT | MSG_NOSIGNAL;

	return true;
}

bool csocket::is_blocking_mode(void)
{
	int flags;

	flags = fcntl(m_sock_fd, F_GETFL);

	if (flags == -1) {
		_E("fcntl(F_GETFL) failed for %s, m_sock_fd: %d", get_client_name(), m_sock_fd);
		_ERRNO(errno);
		return false;
	}

	return !(flags & O_NONBLOCK);
}


bool csocket::is_valid(void) const
{
	return (m_sock_fd >= 0);
}

int csocket::get_socket_fd(void) const
{
	return m_sock_fd;
}

bool csocket::close(void)
{
	if (m_sock_fd >= 0) {
		if (::close(m_sock_fd) < 0) {
			_E("Socket(%d) close failed", m_sock_fd);
			_ERRNO(errno);
			return false;
		}
		m_sock_fd = -1;
	}

	return true;
}
