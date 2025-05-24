# Webserv
Webserv is a virtual webserver coded in C++. 

## Compiling
**Linux**  
To compile on linux, run "make" at the root of the repository

```bash	
make
```

**Non-Linux**  
To compile on other systems, use Docker and the provided Dockerfile. If you don't have Docker yet, you will have to install and run it first. To compile the project via docker, run (after having started up Docker):

```bash
docker build -t container .
```

## Running
This webserver takes one argument: a configuration file. Sample configuration files can be found in the "configurations" repository.

**Linux**docker run --rm -it -p 127.0.0.1:8080:8080 container  
To run the project on Linux, execute the program with the desired configuration via the command line, like so:

```bash
./webserv path_to_config_file
```

**Non-linux**  
To run the project on a non-Linux system, run the following command in your command line.

```bash
docker run --rm -it -p 127.0.0.1:8080:8080 container
```

This will run the container in interactive mode, opening a new shell on the command line. This shell is now running in Linux, execute the program like you can see in the Linux example above
