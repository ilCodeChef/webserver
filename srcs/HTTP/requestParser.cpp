#include "requestParser.hpp"
#include <string.h>
#include <stdlib.h>
#include <sstream> 
#include "colours.hpp"


RequestParser::RequestParser(VirtualServer &config, Request &request): _is_header_finish(false), _is_first_line(false),
    _is_chunked(false), _current_chunk_size(0), _is_reading_chunk_size(true), finished_request(request) {
	finished_request.error = 0;
}

void RequestParser::printHeader() const {
	std::cout << B_WHT << "Method: " << finished_request.method << RST << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = finished_request.headers.begin(); it != finished_request.headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

RequestParser::~RequestParser(void){
}


int st_check_method(std::string method){
	// std::cout << "method " << method << std::endl;
	if (method.compare("GET") == 0)
		return GET;
	if (method.compare("POST") == 0)
		return POST;
	if (method.compare("DELETE") == 0)
		return DELETE;
	return NOT_IMPLEMTED;
}

int RequestParser::set_body(void){
    if (_is_chunked)
        return handle_chunked_data();
    ssize_t total_body =  finished_request.body.size() + _buffer.size();
    if (total_body >= _body_size){
	ssize_t diff =  total_body - _body_size;
	finished_request.body.append(_buffer.substr(0, _buffer.size() - diff));
	return true;
    }
    else
    	finished_request.body.append(_buffer);
    _buffer.clear();
    return false;
}

bool RequestParser::handle_chunked_data(void){
    while (!_buffer.empty()){
        if (_is_reading_chunk_size){
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                return false;
            
            std::string chunk_size_str = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);
            std::stringstream ss;
            ss << std::hex << chunk_size_str;
            ss >> _current_chunk_size;
            
            if (_current_chunk_size == 0){
                // End of chunked data
                _buffer.clear();
                return true;
            }
            
            _is_reading_chunk_size = false;
        } else {
            if (_buffer.size() < _current_chunk_size + 2)
                return false;
            
            finished_request.body.append(_buffer.substr(0, _current_chunk_size));
            _buffer.erase(0, _current_chunk_size + 2);
            _is_reading_chunk_size = true;
        }
    }
    
    return false;
}

bool  RequestParser::parse_protocol(void){
	if (_buffer.find_first_of("HTTP/1.1")  == std::string::npos){
		finished_request.error = WRONG_PROTOCOL;
		return true;
	}
	_protocol = "HTTP/1.1";
	//std::cout << "&protocol " << _protocol << std::endl;
	_buffer.erase(0, _buffer.find_first_of('\n') + 1);
	return false;
}

bool  RequestParser::set_MetAddProt(void){
	finished_request.method  = _buffer.substr(0, _buffer.find_first_of(' '));
	if (st_check_method(finished_request.method) == NOT_IMPLEMTED){
		finished_request.error = NOT_IMPLEMTED;
	}
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	finished_request.uri = _buffer.substr(0, _buffer.find_first_of(' '));
	finished_request.url = finished_request.uri;

	if (finished_request.uri.find('?') != std::string::npos){
		finished_request.query_string = finished_request.uri.substr(finished_request.uri.find('?') + 1, finished_request.uri.length());
		finished_request.url = finished_request.uri.substr(0,finished_request.uri.find('?'));
	}
	finished_request.script_name = finished_request.url.substr(finished_request.url.find_last_of('/') + 1, finished_request.url.length());
	_buffer.erase(0, _buffer.find_first_of(' ') + 1);
	return(parse_protocol());
}

void RequestParser::set_map(void){
	std::string value;
	while (_buffer.size() && _buffer.find_first_of('\n') != 0) {
		if (_buffer.find_first_of("\r\n\r\n") == 0 || _buffer.find_first_of("\n\n") == 0){
			break;
		}

		//checking if part of previus key
		while (_buffer.find_first_of(':') == std::string::npos || _buffer.find_first_of(':') > _buffer.find_first_of('\n')){
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of("\r\n"));
			_buffer.erase(0 ,_buffer.find_first_of('\n') + 1);
			if((finished_request.headers[ _last_key]).find_last_of(',') != (finished_request.headers[ _last_key]).length())
				finished_request.headers[ _last_key].append(", ");
			finished_request.headers[ _last_key].append(value);
		}
		_last_key = _buffer.substr(0, _buffer.find_first_of(':'));
		_buffer.erase(0,_buffer.find_first_of(':') + 2);
		if (_buffer.find_first_of("\r\n") != std::string::npos)
			value = _buffer.substr(0, _buffer.find_first_of("\r\n"));
		else
			value = _buffer.substr(_buffer.find_first_not_of(" \t"), _buffer.find_first_of('\n'));
		_buffer.erase(0,_buffer.find_first_of('\n') + 1);
		finished_request.headers[ _last_key] =  value;
	}
	_is_header_finish = true;
	_buffer.erase(0, 2);

    if (finished_request.headers.find("Transfer-Encoding") != finished_request.headers.end() &&
        finished_request.headers["Transfer-Encoding"].find("chunked") != std::string::npos)
        _is_chunked = true;
}

bool RequestParser::feed(const char *chunk, ssize_t size)
{
	_buffer.append(chunk, size);
    
    if (!_is_header_finish) {
        if (_buffer.find("\r\n\r\n") == std::string::npos) {
            return false;
        }
		if (set_MetAddProt())
			return true;
		set_map();

		if (finished_request.method != "POST")
			return true;
		if (finished_request.headers.find("Content-Length") != finished_request.headers.end()){
			if (stoi(finished_request.headers["Content-Length"]) == 0 )
				return true;

			_body_size = stoi(finished_request.headers["Content-Length"]);
		}
    }

	if (set_body()){
		return true;
	}
	if (_is_chunked){
		if (_current_chunk_size == 0 && _is_reading_chunk_size)
			return true;
	}
    return false;
}



