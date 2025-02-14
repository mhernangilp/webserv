<h1 align="center">
	🌐 webserv
</h1>

<p align="center">
	<b><i>This is when you finally understand why a URL starts with HTTP</i></b><br>
</p>

<h3 align="center">
	<a href="#%EF%B8%8F-about">About</a>
	<span> · </span>
	<a href="#%EF%B8%8F-usage">Usage</a>
	<span> · </span>
	<a href="#-testing">Testing</a>
</h3>

---

## 💡 About the project

> _This project consists of building your own HTTP server from scratch using C++.  
> You will learn the internals of the HTTP protocol and how to implement essential server features._  

For more detailed information, look at the [**subject of this project**](https://github.com/mhernangilp/webserv/blob/main/en.subject.webserv.pdf).

## 🛠️ Usage

### Requirements

The program is written in C++98 and needs a **`gcc` or `clang` compiler** to run, along with standard **C++ libraries**.

### Instructions

**1. Compiling**

To compile the project, go to the project path and run:

```shell
$ make
```

**2. Running**

Run the server with a configuration file:

```shell
$ ./webserv [configuration_file]
```

If no configuration file is provided, the server will use a default one.

## 📋 Testing

To test the server, you can use `telnet`:

```shell
$ telnet localhost 8002
```

But the best way to test it, is with your browser so you can see the default html.

You can also use `curl`:

```shell
$ curl localhost:8002
```
