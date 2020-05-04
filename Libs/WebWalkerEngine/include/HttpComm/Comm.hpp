#pragma once
/*
 * Copyright © 2018, Kasi. All rights reserved.
 */
#ifndef COMM_HPP_INCLUDED
#define COMM_HPP_INCLUDED

#include <string>
#include <map>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <mutex>
#include <atomic>
#include <sstream>

class Request
{
private:
    std::string Method;
    std::string URI;
    std::string Protocol;
    std::map<std::string, std::string> Headers;
    std::string Content;
public:

    Request(const std::string& Method, const std::string& URI, const std::string& Protocol) : Method(Method), URI(URI), Protocol(Protocol), Content("")
    {

    };

    void SetHeader(const std::string& Key, const std::string& Value)
    {
        this->Headers[Key] = Value;
    };

    void SetContent(const std::string& Content) // have to handle format yourself.
    {
        this->Content = Content;
    }

    operator std::string() const
    {
        std::string Result = this->Method + " " + this->URI + " " + this->Protocol + "\r\n";
        for (const auto& [Key, Value] : this->Headers)
            Result += Key + ": " + Value + "\r\n";
        if ((this->Headers.count("Content-Length") == 0) && this->Content.size() != 0)
            Result += "Content-Length: " + std::to_string(this->Content.size()) + "\r\n";
        Result += "\r\n";
        if (this->Content.size())
        {
            Result += this->Content;
            Result += "\r\n";
        }
        return Result;
    }

    ~Request()
    {

    }
};

class Response
{
private:
    std::string Protocol;
    std::int32_t Status;
    std::string StatusMessage;
    std::map<std::string, std::string> Headers;
    std::string Content;
public:

    Response(const std::string Data)
    {
        std::istringstream Parse(Data);
        std::string Line;
        std::string StatusLine = "";
        while (std::getline(Parse, Line))
        {
            if (Line.size() > 1)
            {
                std::uint32_t Pos = Line.find(": ");
                if (Pos != std::string::npos)
                {
                    this->Headers[Line.substr(0, Pos)] = Line.substr(Pos + 2, (Line.size() - 1) - (Pos + 2));
                } else
                {
                    Pos = Line.find(" ");
                    if (Pos != std::string::npos)
                    {
                        this->Protocol = Line.substr(0, Pos);
                        Line = Line.substr(Pos + 1);
                        Pos = Line.find(" ");
                        if (Pos != std::string::npos)
                        {
                            this->Status = std::stoi(Line.substr(0, Pos));
                            Line = Line.substr(Pos + 1);
                            this->StatusMessage = Line;
                        }
                    }
                }
            } else
                break;
        }
        this->Content = Parse.str().substr(Parse.tellg());
    };

    std::string GetProtocol() const
    {
        return this->Protocol;
    };

    std::int32_t GetStatus() const
    {
        return this->Status;
    }

    std::string GetStatusMessage() const
    {
        return this->StatusMessage;
    }

    std::map<std::string, std::string> GetHeaders() const
    {
        return this->Headers;
    }

    std::string GetContent() const
    {
        return this->Content;
    }

    operator std::string() const
    {
        std::string Result = this->Protocol + " " + std::to_string(this->Status) + " " + this->StatusMessage + "\r\n";
        for (const auto& [Key, Value] : this->Headers)
            Result += Key + ": " + Value + "\r\n";
        Result += "\r\n";
        if (this->Content.size())
            Result += this->Content;
        return Result;
    }

    ~Response()
    {

    }
};

typedef hostent HostEntry;
typedef sockaddr_in SocketAddress;

class Comm
{
private:
    std::string URL;
    std::uint16_t Port;
    std::uint32_t Socket;
    HostEntry* HostInfo;
    SocketAddress SocketInfo;
    std::string Buffer;

    static WSAData WSA;
    static std::atomic<std::int32_t> InstanceCount; // does this need to be atomic?
    static std::mutex InstanceMutex;

public:
    Comm(const std::string& URL, std::uint16_t Port) : URL(URL), Port(Port)
    {
        Comm::InstanceMutex.lock();
        {
            if (Comm::InstanceCount == 0)
                if (WSAStartup(MAKEWORD(2, 2), &(Comm::WSA)) != 0)
                    throw std::runtime_error("WSAStartup failed: " + std::to_string(WSAGetLastError()));

            Comm::InstanceCount++;
        }
        Comm::InstanceMutex.unlock();
        this->HostInfo = gethostbyname(URL.c_str());
        this->SocketInfo.sin_port = htons(this->Port);
        this->SocketInfo.sin_family = AF_INET;
        this->SocketInfo.sin_addr.s_addr = *((unsigned long*)(this->HostInfo->h_addr));
        this->Buffer.resize(4096);
    };

    Response Send(const Request& R)
    {
        this->Socket = socket(this->SocketInfo.sin_family, SOCK_STREAM, IPPROTO_TCP);
        if (connect(this->Socket, (SOCKADDR*)&(this->SocketInfo), sizeof(this->SocketInfo)) != 0)
        {
            std::int32_t ConnectError = WSAGetLastError();
            if (ConnectError != WSAEISCONN)
                throw std::runtime_error("Connect failed: " + std::to_string(ConnectError));
        }
        Request CR = R; // maybe change.
        CR.SetHeader("Host", this->URL);
        std::string RawRequest = CR;
        std::int32_t SendResult = send(this->Socket, RawRequest.c_str(), RawRequest.size(), 0);
        if (SendResult == SOCKET_ERROR || !(SendResult <= (std::int32_t)RawRequest.size()))
            throw std::runtime_error("Socket error: " + std::to_string(WSAGetLastError()));
        std::int32_t BuffSize = 0;
        std::string Res;
        do
        {
            BuffSize = recv(this->Socket, &this->Buffer[0], this->Buffer.size(), 0);
            if (BuffSize > 0)
                Res += this->Buffer.substr(0, BuffSize);
        } while (BuffSize > 0);
        if (this->Socket)
        {
            closesocket(this->Socket);
            this->Socket = 0;
        }
        return Response(Res);
    }

    ~Comm()
    {
        InstanceMutex.lock();
        {
            if (Comm::InstanceCount == 1)
                WSACleanup();
            Comm::InstanceCount--;
        }
        InstanceMutex.unlock();
    };

};

WSAData Comm::WSA;
std::atomic<std::int32_t> Comm::InstanceCount = 0;
std::mutex Comm::InstanceMutex;

#endif // COMM_HPP_INCLUDED