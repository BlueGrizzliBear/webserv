# webserv

A HTTP server compliant to RFC 7230-7235 using socket programming, written in C++.
The server configuration file is by default ./configuration/default.conf.

Implemented methods: GET POST PUT HEAD

Additional implemented features: The server can execute CGI-based extension files, depending on the server configuration file.

The server manages multi-clients and can be configured for multiples servers.

## Context

This project is part of the webserv project of 42 Software Engineering program.

## Configuration

By default, the program takes the default.conf configuration file in the configuration folder.
This file's parsing was inspired from Nginx configuration files.

Notable features:

- Multiple servers can be configured simultaneously.
- Basic configuration errors will be displayed and user will be shown where to correct them.
- Keywords implemented:
  - root
  - server_name
  - error_page
  - location
  - allowed_methods (custom)
  - index
  - autoindex
  - rewrite (custom)
  - auth_basic
  - auth_basic_user_file
  - cgi
  - client_max_body_size
- CGI usage: download and set the path to your cgi executable in this configuration file.

Configuration remarks:

- Only one port per server can be configured
- Multiple servers can be configured with the same port

## Usage

### Launch the program

Use the Makefile to compile the project

```bash
make
```

Launch the project

```bash
./webserv
```

By default, the program takes the default.conf configuration file in the configuration folder

However, it can be given the path to another configuration file instead on the first argument.

```bash
./webserv ./my/path/to/this/other/configuration/file
```

### Testing

The project has a data/ folder used for personnal testing.

## Remarks

The program is not multithreaded regarding client request handling.