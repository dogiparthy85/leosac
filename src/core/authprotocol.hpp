/**
 * \file authprotocol.hpp
 * \author Thibault Schueller <ryp.sqrt@gmail.com>
 * \brief AuthProtocol class declaration
 */

#ifndef AUTHPROTOCOL_HPP
#define AUTHPROTOCOL_HPP

#include <map>
#include <string>
#include <list>

#include "modules/imodule.hpp"
#include "iauthprotocol.hpp"

class IAuthModule;
class ILoggerModule;

class AuthProtocol : public IAuthProtocol
{
    typedef void (AuthProtocol::*RegisterFunc)(IModule*);

public:
    explicit AuthProtocol();
    ~AuthProtocol() = default;

    AuthProtocol(const AuthProtocol& other) = delete;
    AuthProtocol& operator=(const AuthProtocol& other) = delete;

public:
    virtual void    logMessage(const std::string& message) override;
    virtual void    createAuthRequest(const std::string& content, const std::string& target) override;
    virtual void    authorize(AuthRequest::Uid id, bool granted) override;

public:
    void    sync();
    void    registerModule(IModule* module);

private:
    void    processAuthRequest(AuthRequest& ar);
    void    registerDoorModule(IModule* module);
    void    registerAccessPointModule(IModule* module);
    void    registerAuthModule(IModule* module);
    void    registerLoggerModule(IModule* module);
    void    registerActivityMonitorModule(IModule* module);

private:
    AuthRequest::Uid                            _authCounter;
    std::map<AuthRequest::Uid, AuthRequest>     _requests;
    std::map<IModule::ModuleType, RegisterFunc> _registrationHandler;
    std::list<ILoggerModule*>                   _loggerModules;
    IAuthModule*                                _authModule;
};

#endif // AUTHPROTOCOL_HPP